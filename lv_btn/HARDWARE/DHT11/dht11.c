/*
 * =============================================================================
 * 文件名称: dht11.c
 * 模块功能: DHT11温湿度传感器驱动
 * =============================================================================
 * 
 * 【模块原理】
 * DHT11是一款数字温湿度传感器，采用单总线协议通信。
 * 传感器内部包含电阻式湿度测量元件和NTC温度测量元件，
 * 并连接一个高性能8位单片机进行数据处理和校验。
 * 
 * 【技术参数】
 * - 工作电压: 3.3V ~ 5.5V
 * - 湿度范围: 20% ~ 90% RH (±5% RH)
 * - 温度范围: 0°C ~ 50°C (±2°C)
 * - 采样周期: ≥1秒
 * - 数据格式: 40位 (8位湿度整数 + 8位湿度小数 + 8位温度整数 + 8位温度小数 + 8位校验和)
 * 
 * 【通信时序】
 * 1. 起始信号:
 *    - 主机拉低数据线 ≥18ms
 *    - 主机拉高数据线 20~40us
 *    - 主机释放总线，切换为输入模式
 * 
 * 2. DHT11响应:
 *    - DHT11拉低数据线 80us (响应信号)
 *    - DHT11拉高数据线 80us (准备发送数据)
 * 
 * 3. 数据传输 (40位，高位先出):
 *    - 每位数据以50us低电平开始
 *    - 数据"0": 高电平持续 26~28us
 *    - 数据"1": 高电平持续 70us
 *    - 判断方法: 延时40us后采样，高电平为1，低电平为0
 * 
 * 【数据格式】
 * 字节0: 湿度整数部分 (例如: 65 表示 65%)
 * 字节1: 湿度小数部分 (DHT11固定为0)
 * 字节2: 温度整数部分 (例如: 25 表示 25°C)
 * 字节3: 温度小数部分 (DHT11固定为0)
 * 字节4: 校验和 = 字节0 + 字节1 + 字节2 + 字节3 (低8位)
 * 
 * 【硬件连接】
 * DHT11模块 (3针):
 *   - VCC  -> 3.3V 或 5V
 *   - DATA -> PA5 (STM32)
 *   - GND  -> GND
 * 
 * 【定时器资源】
 * TIM3: 专用于DHT11微秒级精确延时
 *   - 时钟源: APB1 = 84MHz
 *   - 预分频: 84-1 (84MHz/84 = 1MHz)
 *   - 计数精度: 1us
 * 
 * 【关键实现要点】
 * 1. 时序精确性: 使用TIM3硬件定时器，避免软件延时误差
 * 2. 临界区保护: 整个读取过程禁止中断，防止时序被打断
 * 3. 数据校验: 校验和验证确保数据完整性
 * 4. 采样间隔: ≥2秒，避免传感器过热
 * 
 * 版本: V2.0
 * 日期: 2025-12-11
 * =============================================================================
 */

#include "dht11.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* =============================================================================
 * 全局变量定义
 * =============================================================================
 */
static dht11_data_t dht11_data = {0};  // DHT11数据缓存
static uint8_t dht11_valid = 0;        // 数据有效标志

/* TIM3句柄 - DHT11专用微秒定时器 */
static TIM_HandleTypeDef TIM3_Handler;

/* =============================================================================
 * 内部函数: TIM3定时器初始化
 * =============================================================================
 */

/**
 * @brief  初始化TIM3用于微秒延时
 * @note   TIM3配置:
 *         - 时钟源: APB1 = 84MHz
 *         - 预分频: 84-1 -> 计数频率 = 84MHz/84 = 1MHz
 *         - 计数周期: 1us
 *         - 最大延时: 65535us ≈ 65ms
 */
static void DHT11_TIM3_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();         // 使能TIM3时钟
    
    TIM3_Handler.Instance = TIM3;
    TIM3_Handler.Init.Prescaler = 84 - 1;           // 84MHz / 84 = 1MHz (1us)
    TIM3_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;  // 向上计数
    TIM3_Handler.Init.Period = 0xFFFF;              // 最大计数值 65535
    TIM3_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM3_Handler);
    HAL_TIM_Base_Start(&TIM3_Handler);   // 启动定时器(轮询模式，不使用中断)
}

/**
 * @brief  TIM3微秒延时(精确)
 * @param  us: 延时微秒数 (1~65535)
 * @note   使用硬件定时器计数，精度1us
 *         原理: 清零计数器，等待计数值达到目标值
 */
static void DHT11_Delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&TIM3_Handler, 0);        // 清零计数器
    while(__HAL_TIM_GET_COUNTER(&TIM3_Handler) < us);  // 等待计数完成
}

/* =============================================================================
 * 外部接口函数
 * =============================================================================
 */

/**
 * @brief  DHT11初始化
 * @note   初始化流程:
 *         1. 初始化TIM3定时器
 *         2. 配置GPIO为推挽输出
 *         3. 数据线置高电平(空闲状态)
 *         4. 等待2秒让DHT11上电稳定
 */
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 初始化TIM3专用定时器 */
    DHT11_TIM3_Init();
    
    /* 初始化GPIO */
    DHT11_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;             // 无上下拉(模块自带上拉)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;   // 高速
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
    
    DHT11_DATA_OUT(1);           // 数据线置高(空闲状态)
    os_delay_ms(2000);           // 等待DHT11上电稳定(规格要求≥1秒)
    
    printf("[DHT11] init on PA5, TIM3 ready\r\n");
}

/**
 * @brief  读取DHT11温湿度数据
 * @param  data: 数据结构体指针，用于存储读取结果
 * @retval 1=读取成功, 0=读取失败
 * 
 * @note   【读取流程】
 *         1. 进入临界区(禁止中断)
 *         2. 发送起始信号: 拉低20ms -> 拉高30us
 *         3. 切换为输入模式，等待DHT11响应
 *         4. 等待响应信号: 低电平80us -> 高电平80us
 *         5. 读取40位数据(5字节)
 *         6. 退出临界区
 *         7. 校验数据并返回结果
 * 
 * @note   【数据拼接方式】
 *         - 每位数据以50us低电平开始
 *         - 延时40us后采样: 高电平=1, 低电平=0
 *         - 8位组成1字节，高位先出
 *         - 共读取5字节: 湿度整数、湿度小数、温度整数、温度小数、校验和
 */
uint8_t DHT11_Read_Data(dht11_data_t *data)
{
    uint8_t buf[5] = {0};        // 数据缓冲区: [湿度整数][湿度小数][温度整数][温度小数][校验和]
    uint8_t i, j, byte, timeout;
    uint8_t result = 0;
    
    if(data == NULL) return 0;
    
    /* ========== 进入临界区(禁止中断和任务切换) ========== */
    taskENTER_CRITICAL();
    
    /* ========== 步骤1: 发送起始信号 ========== */
    DHT11_DATA_MODE_OUTPUT();    // 配置为输出模式
    DHT11_DATA_OUT(1);           // 先确保高电平
    DHT11_Delay_us(10);
    DHT11_DATA_OUT(0);           // 拉低数据线
    
    /* 拉低至少18ms (使用TIM3精确延时，分20次每次1ms) */
    for(i = 0; i < 20; i++)
    {
        DHT11_Delay_us(1000);    // 总计20ms
    }
    
    DHT11_DATA_OUT(1);           // 拉高数据线
    DHT11_Delay_us(30);          // 保持20-40us
    
    /* ========== 步骤2: 切换为输入，等待DHT11响应 ========== */
    DHT11_DATA_MODE_INPUT();     // 配置为输入模式(浮空)
    
    /* 等待DHT11拉低(响应信号开始) */
    timeout = 100;
    while(DHT11_DATA_IN() && timeout > 0)
    {
        DHT11_Delay_us(1);
        timeout--;
    }
    if(timeout == 0) goto exit_fail;  // 超时，DHT11未响应
    
    /* 等待低电平结束(DHT11拉低约80us) */
    timeout = 100;
    while(!DHT11_DATA_IN() && timeout > 0)
    {
        DHT11_Delay_us(1);
        timeout--;
    }
    if(timeout == 0) goto exit_fail;
    
    /* 等待高电平结束(DHT11拉高约80us，准备发送数据) */
    timeout = 100;
    while(DHT11_DATA_IN() && timeout > 0)
    {
        DHT11_Delay_us(1);
        timeout--;
    }
    if(timeout == 0) goto exit_fail;
    
    /* ========== 步骤3: 读取40位数据(5字节) ========== */
    for(i = 0; i < 5; i++)       // 5个字节
    {
        byte = 0;
        for(j = 0; j < 8; j++)   // 每字节8位，高位先出
        {
            byte <<= 1;          // 左移，为新位腾出位置
            
            /* 等待低电平结束(每位数据以50us低电平开始) */
            timeout = 100;
            while(!DHT11_DATA_IN() && timeout > 0)
            {
                DHT11_Delay_us(1);
                timeout--;
            }
            if(timeout == 0) goto exit_fail;
            
            /* 延时40us后采样 */
            /* 数据"0": 高电平26-28us，40us后已变低 */
            /* 数据"1": 高电平70us，40us后仍为高 */
            DHT11_Delay_us(40);
            
            if(DHT11_DATA_IN())  // 采样为高电平，数据位为1
            {
                byte |= 1;       // 最低位置1
                
                /* 等待高电平结束 */
                timeout = 100;
                while(DHT11_DATA_IN() && timeout > 0)
                {
                    DHT11_Delay_us(1);
                    timeout--;
                }
            }
            /* 采样为低电平，数据位为0，byte最低位保持0 */
        }
        buf[i] = byte;           // 保存读取的字节
    }
    
    result = 1;  // 读取成功标志
    
exit_fail:
    /* ========== 退出临界区 ========== */
    taskEXIT_CRITICAL();
    
    /* ========== 步骤4: 数据校验(临界区外执行，允许打印) ========== */
    if(result)
    {
        /* 校验和 = 前4字节之和的低8位 */
        uint8_t checksum = buf[0] + buf[1] + buf[2] + buf[3];
        
        if(checksum == buf[4])   // 校验通过
        {
            data->humidity_int = buf[0];     // 湿度整数
            data->humidity_dec = buf[1];     // 湿度小数(DHT11固定为0)
            data->temperature_int = buf[2];  // 温度整数
            data->temperature_dec = buf[3];  // 温度小数(DHT11固定为0)
            data->checksum = buf[4];         // 校验和
            data->valid = 1;                 // 数据有效
            
            printf("[DHT11] T=%d.%dC H=%d.%d%%\r\n",
                   buf[2], buf[3], buf[0], buf[1]);
            return 1;
        }
        else  // 校验失败
        {
            printf("[DHT11] checksum fail: calc=%d got=%d\r\n", checksum, buf[4]);
            data->valid = 0;
            return 0;
        }
    }
    else  // 读取失败(超时)
    {
        printf("[DHT11] no response\r\n");
        return 0;
    }
}

/**
 * @brief  获取DHT11缓存数据指针
 * @retval 指向dht11_data_t结构体的指针
 * @note   返回的是静态缓存数据，需配合DHT11_Is_Valid()检查有效性
 */
dht11_data_t* DHT11_Get_Data(void)
{
    return &dht11_data;
}

/**
 * @brief  检查DHT11数据是否有效
 * @retval 1=数据有效, 0=数据无效
 */
uint8_t DHT11_Is_Valid(void)
{
    return dht11_valid;
}

/**
 * @brief  DHT11数据处理任务(定期采集)
 * @note   应在FreeRTOS任务中定期调用
 *         采集间隔: 2秒(DHT11规格要求≥1秒)
 *         自动更新全局缓存dht11_data和有效标志dht11_valid
 */
void DHT11_Process_Data(void)
{
    static uint32_t last_read = 0;
    uint32_t current_time = HAL_GetTick();
    
    /* 每2秒读取一次 */
    if((current_time - last_read) > 2000)
    {
        if(DHT11_Read_Data(&dht11_data))
        {
            dht11_valid = 1;     // 读取成功，数据有效
        }
        else
        {
            dht11_valid = 0;     // 读取失败，数据无效
        }
        last_read = current_time;
    }
}
