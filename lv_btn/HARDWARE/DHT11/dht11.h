/*
 * 模块名称: DHT11温湿度传感器驱动 (dht11)
 * 功能概述:
 *   - 读取DHT11传感器的温度和湿度数据
 *   - 使用单总线协议通信
 *   - 提供数据有效性校验
 * 
 * 传感器参数:
 *   - 测量范围: 湿度20-90%RH, 温度0-50℃
 *   - 测量精度: 湿度±5%RH, 温度±2℃
 *   - 分辨率: 湿度1%RH, 温度1℃
 *   - 采样周期: ≥1秒
 * 
 * 硬件连接:
 *   - DATA: PA5 (单总线数据线)
 *   - VCC: 3.3V~5V
 *   - GND: 地
 * 
 * 通信协议:
 *   1. 主机发送起始信号(拉低18ms，然后拉高20-40us)
 *   2. DHT11响应(拉低80us，然后拉高80us)
 *   3. DHT11发送40位数据(8位湿度整数+8位湿度小数+8位温度整数+8位温度小数+8位校验和)
 *   4. 数据位: 低电平50us + 高电平26-28us表示"0"，高电平70us表示"1"
 * 
 * 使用方法:
 *   1. 调用 DHT11_Init() 初始化
 *   2. 调用 DHT11_Process_Data() 读取数据(建议间隔≥1秒)
 *   3. 调用 DHT11_Get_Data() 获取数据指针
 *   4. 调用 DHT11_Is_Valid() 检查数据有效性
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef _DHT11_H
#define _DHT11_H
#include "sys.h"

/*
 * DHT11引脚定义 (PA5)
 * 注意: PG14被LCD FSMC占用，PB0被触摸屏占用，故使用PA5
 */
#define DHT11_GPIO_PORT    GPIOA
#define DHT11_GPIO_PIN     GPIO_PIN_5
#define DHT11_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()

/*
 * DHT11操作宏定义
 */
#define DHT11_DATA_OUT(x)  HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define DHT11_DATA_IN()    HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN)

/* 设置数据线为输入模式 */
#define DHT11_DATA_MODE_INPUT()  do{ \
    GPIO_InitTypeDef GPIO_InitStruct = {0}; \
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN; \
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; \
    GPIO_InitStruct.Pull = GPIO_NOPULL; \
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct); \
} while(0)

/* 设置数据线为输出模式 */
#define DHT11_DATA_MODE_OUTPUT()  do{ \
    GPIO_InitTypeDef GPIO_InitStruct = {0}; \
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN; \
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; \
    GPIO_InitStruct.Pull = GPIO_NOPULL; \
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; \
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct); \
} while(0)

/*
 * DHT11数据结构
 * 存储温湿度数据和有效性标志
 */
typedef struct
{
    uint8_t humidity_int;      // 湿度整数部分 (0-100)
    uint8_t humidity_dec;      // 湿度小数部分 (DHT11始终为0)
    uint8_t temperature_int;   // 温度整数部分 (0-50)
    uint8_t temperature_dec;   // 温度小数部分 (DHT11始终为0)
    uint8_t checksum;          // 校验和 (前4字节之和的低8位)
    uint8_t valid;             // 数据有效标志 (1=有效, 0=无效)
} dht11_data_t;

/*
 * 函数声明
 */
void DHT11_Init(void);                      // 初始化DHT11(GPIO和定时器)
uint8_t DHT11_Read_Data(dht11_data_t *data); // 读取DHT11数据(返回0成功)
dht11_data_t* DHT11_Get_Data(void);         // 获取数据指针
uint8_t DHT11_Is_Valid(void);               // 检查数据是否有效
void DHT11_Process_Data(void);              // 处理DHT11数据(主循环调用)

#endif
