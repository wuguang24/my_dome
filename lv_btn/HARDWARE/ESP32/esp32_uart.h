#ifndef __ESP32_UART_H
#define __ESP32_UART_H
#include "sys.h"
#include "stdio.h"
#include <stdbool.h>

// ESP32串口通信配置
#define ESP32_UART_REC_LEN  1024  // 最大接收字节数（增大以容纳ESP32回复）
#define EN_ESP32_UART_RX    1     // 使能(1)/禁止(0)串口接收

// 接收缓冲区和状态
extern u8  ESP32_UART_RX_BUF[ESP32_UART_REC_LEN]; // 接收缓冲区
extern u16 ESP32_UART_RX_STA;                     // 接收状态标记
extern UART_HandleTypeDef ESP32_UART_Handler;    // UART句柄

#define ESP32_UART_RXBUFFERSIZE   1               // HAL库接收缓冲区大小
extern u8 aRxBufferESP32[ESP32_UART_RXBUFFERSIZE]; // HAL库接收Buffer

// ESP32通信协议函数
void ESP32_UART_Init(u32 bound);                  // 初始化ESP32串口（默认115200）

// 发送传感器数据到ESP32（使用DATA:命令）
// 参数：JSON格式的传感器数据
// 返回：true-成功，false-失败
bool ESP32_Send_SensorData(
    bool gps_ok,              // GPS状态
    bool radar_ok,            // 雷达状态
    float temperature,        // 温度
    float humidity,           // 湿度
    const char* gps_dw1,      // GPS定位1
    const char* gps_dw2,      // GPS定位2
    int xinlv1,               // 心率1
    int xinlv2,               // 心率2
    const char* shijian1,     // 时间1
    const char* shijian2      // 时间2
);

// 查询ESP32在线状态
// 返回：true-在线，false-离线
bool ESP32_Query_Status(void);

// 查询网络时间
// 参数：time_buf - 时间字符串缓冲区（至少32字节）
// 返回：true-成功，false-失败
bool ESP32_Query_Time(char* time_buf, u16 buf_size);

// 处理ESP32下发的蜂鸣器控制命令
// 参数：buzzer_state - 输出蜂鸣器状态（0-关闭，1-开启）
// 返回：true-收到新命令，false-无新命令
bool ESP32_Get_BuzzerCmd(u8* buzzer_state);

// 兼容旧接口：在线检测
// 返回：true-在线，false-离线
bool ESP32_Is_Online(void);

// 解析ESP32主动发送的数据（TIME:等）
void ESP32_Parse_RxData(void);

#endif
