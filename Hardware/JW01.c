#include "JW01.h"
#include "USART1.h"  // 使用现有的USART1驱动
#include <stdio.h>

// JW01 CO2 传感器模块驱动
// 通信方式：USART1（常见为 9600 8N1）
// 数据帧（按当前解析逻辑推断）：
// - 一帧 6 字节
// - 帧头：0x2C
// - CO2 值：Byte1 为高字节，Byte2 为低字节（ppm）
// - 校验：Byte5 = Byte0..Byte4 的累加和（低 8 位）
// 说明：USART1 驱动在中断里收集数据，本文件在主循环中轮询解析，避免在中断里做复杂处理。

// JW01数据缓存
static uint8_t JW01_RxBuffer[6];
static uint8_t JW01_RxIndex = 0;
static uint8_t JW01_DataReady = 0;
static uint16_t CO2_Value = 0;


// 调试变量：用于观察原始帧与校验情况（可在调试器中查看）
uint8_t JW01_Debug_RawData[6];  // 存储原始数据
uint8_t JW01_Debug_Checksum = 0; // 计算的校验和
uint8_t JW01_Debug_Valid = 0;    // 数据有效性标志


/**
  * @brief  JW01串口初始化
  * @param  无
  * @retval 无
  */
void JW01_Init(void)
{
    // 初始化串口1，波特率 9600（按模块常见默认值）
    USART1_Init(9600);
    
    // 初始化接收状态
    JW01_RxIndex = 0;
    JW01_DataReady = 0;
    CO2_Value = 0;
    
    // 清空USART1接收缓冲区（丢弃上电瞬间的杂波/残留字节）
    USART1_Data_Clear();
}

/**
  * @brief  JW01数据处理函数（需要在主循环中调用）
  * @param  无
  * @retval 无
  */
/*void JW01_ProcessData(void)
{
    // 获取USART1接收到的数据长度
    uint16_t data_len = USART1_Data_Len_Get();
    
    if(data_len > 0)
    {
        // 获取接收缓冲区
        uint8_t *rx_data = USART1_Data_Get();
        
        // 处理每个字节
        for(uint16_t i = 0; i < data_len; i++)
        {
            uint8_t byte = rx_data[i];
            
            // 寻找帧头0x2C
            if(JW01_RxIndex == 0)
            {
                if(byte == 0x2C)  // 找到帧头
                {
                    JW01_RxBuffer[JW01_RxIndex++] = byte;
                }
            }
            else
            {
                // 存储后续数据
                JW01_RxBuffer[JW01_RxIndex++] = byte;
                
                // 收到完整一帧（6字节）
                if(JW01_RxIndex >= 6)
                {
                    // 验证校验和
                    uint8_t checksum = JW01_RxBuffer[0] + JW01_RxBuffer[1] + 
                                      JW01_RxBuffer[2] + JW01_RxBuffer[3] + 
                                      JW01_RxBuffer[4];
                    
                    if(checksum == JW01_RxBuffer[5])  // 校验和正确
                    {
                        // 计算CO2值：高字节*256 + 低字节
                        CO2_Value = (JW01_RxBuffer[1] << 8) | JW01_RxBuffer[2];
                        
                        // 标记数据就绪
                        JW01_DataReady = 1;
                    }
                    
                    // 重置接收状态
                    JW01_RxIndex = 0;
                }
            }
        }
        
        // 清空USART1接收缓冲区
        USART1_Data_Clear();
    }
}
*/
void JW01_ProcessData(void)
{
    // 获取 USART1 接收到的数据长度
    uint16_t data_len = USART1_Data_Len_Get();
    
    if(data_len > 0)
    {
        // 获取接收缓冲区（线性缓存，长度为 data_len）
        uint8_t *rx_data = USART1_Data_Get();
        
        // 逐字节解析：通过帧头同步，累计到 6 字节后校验
        for(uint16_t i = 0; i < data_len; i++)
        {
            uint8_t byte = rx_data[i];
            
            // 寻找帧头0x2C
            if(JW01_RxIndex == 0)
            {
                if(byte == 0x2C)  // 找到帧头
                {
                    JW01_RxBuffer[JW01_RxIndex++] = byte;
                }
            }
            else
            {
                // 存储后续数据
                JW01_RxBuffer[JW01_RxIndex++] = byte;
                
                // 收到完整一帧（6字节）
                if(JW01_RxIndex >= 6)
                {
                    // 存储原始数据用于调试
                    for(int j = 0; j < 6; j++) {
                        JW01_Debug_RawData[j] = JW01_RxBuffer[j];
                    }
                    
                    // 验证校验和
                    uint8_t checksum = JW01_RxBuffer[0] + JW01_RxBuffer[1] + 
                                      JW01_RxBuffer[2] + JW01_RxBuffer[3] + 
                                      JW01_RxBuffer[4];
                    
                    JW01_Debug_Checksum = checksum;  // 存储计算的校验和
                    
                    if(checksum == JW01_RxBuffer[5])  // 校验和正确
                    {
                        JW01_Debug_Valid = 1;
                        
                        // 计算CO2值：高字节*256 + 低字节
                        uint16_t raw_co2 = (JW01_RxBuffer[1] << 8) | JW01_RxBuffer[2];
                        
                        // 简单有效性过滤：检查是否为合理范围（350-5000ppm）
                        // 备注：具体有效范围与传感器/环境有关，可按需调整。
                        if(raw_co2 >= 350 && raw_co2 <= 5000) {
                            CO2_Value = raw_co2;
                        } else {
                            CO2_Value = 0;  // 无效数据
                        }
                        
                        // 标记数据就绪
                        JW01_DataReady = 1;
                    }
                    else
                    {
                        JW01_Debug_Valid = 0;
                        CO2_Value = 0;  // 校验失败，清零
                    }
                    
                    // 重置接收状态
                    JW01_RxIndex = 0;
                }
            }
        }
        
        // 清空USART1接收缓冲区：已消费完本轮数据
        USART1_Data_Clear();
    }
}

/**
  * @brief  检查是否有新的CO2数据
  * @param  无
  * @retval 1:有新数据，0:无新数据
  */
uint8_t JW01_NewDataAvailable(void)
{
    // 注意：该标志在 JW01_GetCO2_PPM() 中会被清除
    return JW01_DataReady;
}

/**
  * @brief  获取CO2浓度值（ppm）
  * @param  无
  * @retval CO2浓度（单位：ppm）
  */
uint16_t JW01_GetCO2_PPM(void)
{
    // 读取 CO2 值，同时清除“新数据”标志
    JW01_DataReady = 0;  // 清除数据就绪标志
    return CO2_Value;
}
