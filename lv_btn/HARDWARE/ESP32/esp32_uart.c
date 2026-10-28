/*
 * 模块名称: ESP32串口通信驱动 (esp32_uart.c)
 * 功能说明:
 *   - 通过USART6与ESP32进行串口通信（115200波特率）
 *   - 实现ESP32_STM32串口通信协议
 *   - 发送传感器数据到ESP32（DATA:命令）
 *   - 查询ESP32状态和时间
 *   - 接收ESP32下发的蜂鸣器控制命令
 * 硬件连接:
 *   STM32 USART6 (PC6/PC7) <-> ESP32 UART1 (GPIO17/GPIO18)
 */
#include "esp32_uart.h"   // ESP32 串口通信头文件
#include "delay.h"        // 延时接口（兼容OS与裸机）
#include "FreeRTOS.h"     // FreeRTOS 核心
#include "task.h"         // 任务API
#include "semphr.h"       // 信号量API
#include <string.h>        // C 标准库字符串
#include <stdio.h>         // C 标准库IO

#if EN_ESP32_UART_RX   // 如果使能了接收
// 串口接收缓冲区
u8 ESP32_UART_RX_BUF[ESP32_UART_REC_LEN];     // 接收缓冲区（循环累计）
u16 ESP32_UART_RX_STA = 0;                    // 接收状态标记（当前长度）

u8 aRxBufferESP32[ESP32_UART_RXBUFFERSIZE];   // HAL库使用的串口接收缓冲区（1字节）
UART_HandleTypeDef ESP32_UART_Handler;        // UART句柄（USART6）

/* 由应用层提供的串口RX信号量，用于唤醒解析任务 */
extern SemaphoreHandle_t g_esp32_rx_sem;      // 应用层RX信号量（由ISR释放）

// 延时函数：根据FreeRTOS是否启动选择延时方式
static void esp32_delay_ms(uint32_t ms) // 毫秒延时：若已启动RTOS则用vTaskDelay
{
	// 检查FreeRTOS调度器是否已启动
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		vTaskDelay(pdMS_TO_TICKS(ms));
	}
	else
	{
		delay_ms(ms); // FreeRTOS未启动，使用HAL延时
	}
}

// 初始化ESP32串口（USART6，波特率115200）
void ESP32_UART_Init(u32 bound) // 初始化USART6并开启接收中断
{
    // UART 初始化参数
    ESP32_UART_Handler.Instance = USART6;                      // USART6
    ESP32_UART_Handler.Init.BaudRate = bound;                  // 波特率（默认115200）
    ESP32_UART_Handler.Init.WordLength = UART_WORDLENGTH_8B;   // 字长为8位数据格式
    ESP32_UART_Handler.Init.StopBits = UART_STOPBITS_1;        // 一个停止位
    ESP32_UART_Handler.Init.Parity = UART_PARITY_NONE;         // 无奇偶校验位
    ESP32_UART_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;   // 无硬件流控
    ESP32_UART_Handler.Init.Mode = UART_MODE_TX_RX;            // 收发模式
    HAL_UART_Init(&ESP32_UART_Handler);                        // HAL_UART_Init()会使能USART6

    // 开启接收中断
    HAL_UART_Receive_IT(&ESP32_UART_Handler, (u8 *)aRxBufferESP32, ESP32_UART_RXBUFFERSIZE); // 使能中断接收
    
    printf("[ESP32] UART initialized at %lu baud\r\n", bound);
}

// ESP32串口发送字符串
static void ESP32_Send_String(const char *str) // 同步发送字符串（阻塞）
{
	HAL_UART_Transmit(&ESP32_UART_Handler, (uint8_t *)str, strlen(str), 1000);
}

// 从缓冲区中移除已处理的数据（保留未处理的部分）
static void ESP32_Remove_Processed(const char* end_marker)
{
	if(ESP32_UART_RX_STA == 0) return;
	
	ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = 0;
	char* p = strstr((char*)ESP32_UART_RX_BUF, end_marker);
	if(p != NULL)
	{
		// 找到换行符作为消息结束
		char* newline = strchr(p, '\n');
		if(newline != NULL)
		{
			// 移除已处理的数据，保留剩余部分
			u16 processed_len = (newline + 1) - (char*)ESP32_UART_RX_BUF;
			u16 remaining = ESP32_UART_RX_STA - processed_len;
			if(remaining > 0)
			{
				memmove(ESP32_UART_RX_BUF, newline + 1, remaining);
			}
			ESP32_UART_RX_STA = remaining;
		}
		else
		{
			// 没有换行符，清空整个缓冲区
			ESP32_UART_RX_STA = 0;
		}
	}
}

// 等待ESP32回复（带超时）
// 返回：true-收到期望的回复，false-超时或未收到
static bool ESP32_Wait_Reply(const char* expected, u32 timeout_ms) // 等待期望应答
{
	uint32_t start_time = HAL_GetTick();
	
	while((HAL_GetTick() - start_time) < timeout_ms)
	{
		if(ESP32_UART_RX_STA > 0)
		{
			ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = 0; // 添加结束符
			if(strstr((char *)ESP32_UART_RX_BUF, expected) != NULL)
			{
				// 只移除已处理的数据，不清空整个缓冲区
				ESP32_Remove_Processed(expected);
				return true;
			}
		}
		esp32_delay_ms(10);
	}
	
	// 超时，不清空缓冲区（可能有其他数据）
	return false;
}

// 发送传感器数据到ESP32（使用DATA:命令）
// 协议格式：DATA:{"GPS_zhuangtai":true,"LEIDA_zhuangtai":true,...}\n
bool ESP32_Send_SensorData(
    bool gps_ok,
    bool radar_ok,
    float temperature,
    float humidity,
    const char* gps_dw1,
    const char* gps_dw2,
    int xinlv1,
    int xinlv2,
    const char* shijian1,
    const char* shijian2)
{
	char json_buf[1024];
	
	// 构建JSON字符串（按照阿里云物模型格式）
	// 注意：阿里云bool类型用0/1表示，不是true/false
	snprintf(json_buf, sizeof(json_buf),
		"DATA:{\"GPS_zhuangtai\":%d,\"LEIDA_zhuangtai\":%d,"
		"\"WENDU\":%.1f,\"SHIDU\":%.1f,"
		"\"GPS_dingwei1\":\"%s\",\"GPS_dingwei2\":\"%s\","
		"\"XINLV1\":%d,\"XINLV2\":%d,"
		"\"SHIJIAN1\":\"%s\",\"SHIJIAN2\":\"%s\"}\n",
		gps_ok ? 1 : 0,
		radar_ok ? 1 : 0,
		temperature,
		humidity,
		gps_dw1 ? gps_dw1 : "",
		gps_dw2 ? gps_dw2 : "",
		xinlv1 < 0 ? 0 : xinlv1,
		xinlv2 < 0 ? 0 : xinlv2,
		shijian1 ? shijian1 : "",
		shijian2 ? shijian2 : "");
	
	// 发送前先解析缓冲区中可能存在的TIME数据
	ESP32_Parse_RxData();
	
	// 发送数据到ESP32
	ESP32_Send_String(json_buf);     // 发送一帧DATA: JSON
	printf("[ESP32] TX: DATA len=%d\r\n", (int)strlen(json_buf)); // 简化日志，避免打印大量JSON
	
	// 等待ESP32回复 DATA:OK
	if(ESP32_Wait_Reply("DATA:OK", 2000))
	{
		printf("[ESP32] Data sent OK\r\n");
		return true;
	}
	else
	{
		printf("[ESP32] Data send FAIL (timeout or error)\r\n");
		return false;
	}
}

// 查询ESP32在线状态
// 发送：STATUS?\n
// 期望回复：STATUS:ONLINE\n 或 STATUS:OFFLINE\n
bool ESP32_Query_Status(void) // 询问在线状态（STATUS?）
{
	// 发送前先解析缓冲区中可能存在的TIME数据
	ESP32_Parse_RxData();
	
	// 发送查询命令
	ESP32_Send_String("STATUS?\n");
	printf("[ESP32] Query status\r\n");
	
	// 等待回复（同时检查ONLINE和OFFLINE）
	uint32_t start_time = HAL_GetTick();
	while((HAL_GetTick() - start_time) < 1000)
	{
		if(ESP32_UART_RX_STA > 0)
		{
			ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = 0;
			if(strstr((char *)ESP32_UART_RX_BUF, "STATUS:ONLINE") != NULL)
			{
				ESP32_Remove_Processed("STATUS:ONLINE");
				printf("[ESP32] Status: ONLINE\r\n");
				return true;
			}
			else if(strstr((char *)ESP32_UART_RX_BUF, "STATUS:OFFLINE") != NULL)
			{
				ESP32_Remove_Processed("STATUS:OFFLINE");
				printf("[ESP32] Status: OFFLINE\r\n");
				return false;
			}
		}
		esp32_delay_ms(10);
	}
	
	printf("[ESP32] Status: TIMEOUT\r\n");
	return false;
}

// 缓存ESP32主动发送的时间
static char g_esp32_time_cache[32] = {0};
static uint8_t g_esp32_time_valid = 0;

// 解析ESP32主动发送的数据（在接收中断或轮询中调用）
// 解析TIME:命令，BUZZER:由ESP32_Get_BuzzerCmd处理
void ESP32_Parse_RxData(void)
{
	if(ESP32_UART_RX_STA > 0)
	{
		ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = 0;
		
		// 查找 TIME: 开头的数据
		char* p = strstr((char *)ESP32_UART_RX_BUF, "TIME:");
		if(p != NULL)
		{
			char* time_str = p + 5; // 跳过 "TIME:"
			
			// 提取时间字符串（去除换行符）
			char temp[32];
			strncpy(temp, time_str, sizeof(temp)-1);
			temp[sizeof(temp)-1] = '\0';
			char* newline = strchr(temp, '\n');
			if(newline) *newline = '\0';
			newline = strchr(temp, '\r');
			if(newline) *newline = '\0';
			
			// 验证时间格式：必须是19个字符且格式正确 "YYYY-MM-DD HH:MM:SS"
			if(strlen(temp) >= 19 && 
			   temp[4] == '-' && temp[7] == '-' && 
			   temp[10] == ' ' && temp[13] == ':' && temp[16] == ':')
			{
				temp[19] = '\0'; // 只保留19个字符
				strncpy(g_esp32_time_cache, temp, sizeof(g_esp32_time_cache)-1);
				g_esp32_time_valid = 1;
				printf("[ESP32] Time cached: %s\r\n", g_esp32_time_cache);
			}
			
			// 只移除TIME消息，保留其他数据
			ESP32_Remove_Processed("TIME:");
		}
		
		// 防止缓冲区溢出：如果数据太多且没有有效消息，清理旧数据
		if(ESP32_UART_RX_STA > (ESP32_UART_REC_LEN - 100))
		{
			// 保留最后256字节
			u16 keep = 256;
			if(ESP32_UART_RX_STA > keep)
			{
				memmove(ESP32_UART_RX_BUF, &ESP32_UART_RX_BUF[ESP32_UART_RX_STA - keep], keep);
				ESP32_UART_RX_STA = keep;
			}
		}
	}
}

// 查询网络时间
// 优先使用ESP32主动发送的缓存时间，如果没有则发送查询命令
bool ESP32_Query_Time(char* time_buf, u16 buf_size)
{
	if(!time_buf || buf_size < 32) return false;
	
	// 先解析接收缓冲区中的数据
	ESP32_Parse_RxData();
	
	// 如果有缓存的有效时间，直接返回
	if(g_esp32_time_valid && strlen(g_esp32_time_cache) > 5)
	{
		strncpy(time_buf, g_esp32_time_cache, buf_size - 1);
		time_buf[buf_size - 1] = '\0';
		printf("[ESP32] Time from cache: %s\r\n", time_buf);
		return true;
	}
	
	printf("[ESP32] Query time\r\n");
	
	// 发送查询命令
	ESP32_Send_String("TIME?\n");
	
	// 等待回复
	uint32_t start_time = HAL_GetTick();
	while((HAL_GetTick() - start_time) < 1000)
	{
		if(ESP32_UART_RX_STA > 0)
		{
			ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = 0;
			char* time_str = strstr((char *)ESP32_UART_RX_BUF, "TIME:");
			if(time_str != NULL)
			{
				time_str += 5;
				char* newline = strchr(time_str, '\n');
				if(newline) *newline = '\0';
				newline = strchr(time_str, '\r');
				if(newline) *newline = '\0';
				
				if(strlen(time_str) > 5 && strcmp(time_str, "--") != 0)
				{
					strncpy(time_buf, time_str, buf_size - 1);
					time_buf[buf_size - 1] = '\0';
					// 同时更新缓存
					strncpy(g_esp32_time_cache, time_str, sizeof(g_esp32_time_cache)-1);
					g_esp32_time_valid = 1;
					ESP32_Remove_Processed("TIME:");
					return true;
				}
			}
		}
		esp32_delay_ms(10);
	}
	
	printf("[ESP32] Query time failed\r\n");
	return false;
}

// 处理ESP32下发的蜂鸣器控制命令
// ESP32会发送：BUZZER:ON\n 或 BUZZER:OFF\n
bool ESP32_Get_BuzzerCmd(u8* buzzer_state) // 从RX缓冲解析蜂鸣器命令
{
	if(!buzzer_state) return false;
	
	// 检查接收缓冲区是否有数据
	if(ESP32_UART_RX_STA > 0)
	{
		ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = 0;
		
		// 查找 BUZZER: 命令
		if(strstr((char *)ESP32_UART_RX_BUF, "BUZZER:ON") != NULL)
		{
			*buzzer_state = 1;
			ESP32_Remove_Processed("BUZZER:ON");
			printf("[ESP32] RX: BUZZER:ON\r\n");
			return true;
		}
		else if(strstr((char *)ESP32_UART_RX_BUF, "BUZZER:OFF") != NULL)
		{
			*buzzer_state = 0;
			ESP32_Remove_Processed("BUZZER:OFF");
			printf("[ESP32] RX: BUZZER:OFF\r\n");
			return true;
		}
	}
	
	return false;
}

// 兼容旧接口：在线检测封装
bool ESP32_Is_Online(void) // 兼容封装：调用 Query_Status
{
    return ESP32_Query_Status();
}

// 串口中断服务函数
void USART6_IRQHandler(void) // USART6 中断服务程序
{
    uint32_t timeout = 0;
    const uint32_t MAX_TIMEOUT = 10000;  // 合理的超时值，避免死循环

    HAL_UART_IRQHandler(&ESP32_UART_Handler);    // 调用HAL库中断处理公用函数（会调用接收回调）

    // 等待UART就绪，使用合理的超时值
    timeout = 0;
    while (HAL_UART_GetState(&ESP32_UART_Handler) != HAL_UART_STATE_READY)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT) 
        {
            // 超时，强制重置UART状态
            ESP32_UART_Handler.RxState = HAL_UART_STATE_READY;
            break;
        }
    }

    // 重新开启接收中断，使用合理的超时值
    timeout = 0;
    while (HAL_UART_Receive_IT(&ESP32_UART_Handler, (u8 *)aRxBufferESP32, ESP32_UART_RXBUFFERSIZE) != HAL_OK)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT) 
        {
            // 超时，强制重置并重试一次
            ESP32_UART_Handler.RxState = HAL_UART_STATE_READY;
            HAL_UART_Receive_IT(&ESP32_UART_Handler, (u8 *)aRxBufferESP32, ESP32_UART_RXBUFFERSIZE);
            break;
        }
    }

    /* 通知应用层有新的串口数据可供解析 */
    if(g_esp32_rx_sem) { // 唤醒解析任务
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(g_esp32_rx_sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif
