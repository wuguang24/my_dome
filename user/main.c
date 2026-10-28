// 包含 STM32F10x 系列微控制器的头文件
#include "stm32f10x.h"
// 包含延时函数库
#include "Delay.h"
// 包含 DHT11 温湿度传感器库
#include "DHT11.h"
// 包含 OLED 显示库
#include "OLED.h"
// 包含 GY302 光照传感器库
#include "GY302.h"
// 包含 I2C 通信库
#include "MyI2C.h"
// 包含土壤湿度传感器库
#include "Soil.h"
// 包含控制逻辑库
#include "control.h"
// 包含按键库
#include "key.h" 
// 包含风扇控制库
#include "fan.h"      
// 包含水泵控制库
#include "bump.h"     
// 包含LED模块控制库
#include "led_module.h" 
// 包含LED控制库
#include "led.h"      
// 包含蜂鸣器控制库
#include "Buzz.h"     
// 包含标准输入输出库
#include <stdio.h>
// 包含字符串处理库
#include <string.h>
// 包含标准库
#include <stdlib.h>
// 包含 ESP8266 网络模块库
#include "esp8266.h"
// 包含 OneNet 物联网平台库
#include "onenet.h"
// 包含阿里云配置库
#include "aliyun_conf.h"
// 包含串口1库
#include "usart1.h"
// 包含调试库
#include "dbg.h"

// 页面状态枚举
// 定义系统中不同的页面状态
typedef enum {
    PAGE_MAIN = 0,           // 主页面
    PAGE_DEVICE_STATUS,       // 设备状态页面
    PAGE_TEMP_HUMI_SET,       // 温湿度设置页面
    PAGE_LIGHT_SET,           // 光照设置页面
    PAGE_SOIL_SET,            // 土壤湿度设置页面
    PAGE_MODE_SET,            // 模式设置页面
    PAGE_COUNT                // 页面总数
} PageState;

// 设置项枚举
// 定义系统中可设置的参数类型
typedef enum {
    SETTING_TEMP_LOW = 0,       // 温度下限
    SETTING_TEMP_HIGH,          // 温度上限
    SETTING_HUMI,               // 湿度目标值
    SETTING_TEMP_LOW_ALARM,     // 低温报警阈值
    SETTING_TEMP_HIGH_ALARM,    // 高温报警阈值
    SETTING_LIGHT_LOW,          // 光照下限
    SETTING_LIGHT_HIGH,         // 光照上限
    SETTING_LIGHT_ALARM,        // 光照报警阈值
    SETTING_SOIL_LOW,           // 土壤湿度下限
    SETTING_SOIL_HIGH,          // 土壤湿度上限
    SETTING_SOIL_ALARM,         // 土壤报警阈值
    SETTING_COUNT               // 设置项总数
} SettingType;

// 警报类型枚举
// 定义系统中可能的报警类型
typedef enum {
    ALARM_NONE = 0,         // 无报警
    ALARM_TEMP_HIGH,        // 高温报警
    ALARM_TEMP_LOW,         // 低温报警
    ALARM_LIGHT_LOW,        // 光照不足报警
    ALARM_SOIL_DRY,         // 土壤干燥报警
    ALARM_COMBINED          // 组合报警（多种报警同时发生）
} AlarmType;

// 系统状态结构体
// 存储系统的当前状态信息
typedef struct {
    PageState currentPage;          // 当前页面
    PageState previousPage;         // 上一个页面
    SettingType selectedSetting;    // 当前选中的设置项
    uint8_t isSettingMode;          // 是否处于设置模式
    uint32_t settingStartTime;      // 设置模式开始时间
    uint8_t alarmEnabled;           // 报警是否启用
    AlarmType currentAlarm;         // 当前报警类型
    uint8_t alarmActive;            // 报警是否激活
    uint32_t alarmToggleTime;       // 报警状态切换时间
    uint8_t ledRState;              // 红色LED状态
    uint8_t ledBState;              // 蓝色LED状态
    uint8_t buzzState;              // 蜂鸣器状态
    uint8_t isAutoMode;             // 是否处于自动模式
    uint8_t modeSelectedItem;       // 模式页面当前选中项 (0:模式,1:风扇,2:水泵,3:灯)
} SystemState;

// 系统设置结构体
// 存储系统的各种设置参数
typedef struct {
    float temp_low;          // 温度下限
    float temp_high;         // 温度上限
    float targetHumi;        // 湿度目标值
    float alarmTempLow;      // 低温报警阈值
    float alarmTemp;         // 高温报警阈值
    float light_low;         // 光照下限
    float light_high;        // 光照上限
    float alarmLight;        // 光照报警阈值
    float soil_low;          // 土壤湿度下限
    float soil_high;         // 土壤湿度上限
    float alarmSoil;         // 土壤报警阈值
} SystemSettings;

// 设备状态结构体
// 存储各设备的当前状态
typedef struct {
    uint8_t fan_state;   // 风扇状态 (0:关闭, 1:开启)
    uint8_t led_state;   // LED状态 (0:关闭, 1:开启)
    uint8_t pump_state;  // 水泵状态 (0:关闭, 1:开启)
} DeviceState;

// 全局变量
volatile uint32_t system_time_ms = 0;     // 系统时间（毫秒）
DeviceState device_state = {0, 0, 0};      // 设备状态初始化
SystemSettings settings;                   // 系统设置
static uint8_t remote_led_blue = 0;        // 远程控制的蓝色LED状态
static uint8_t remote_led_red = 0;         // 远程控制的红色LED状态
static uint8_t remote_led_valid = 0;       // 远程控制LED状态是否有效

// 函数声明
// 切换到下一个页面
void SwitchToNextPage(SystemState *state);
// 处理按键1的输入
void HandleKey1(SystemState *state);
// 自动控制设备（基于传感器数据）
void AutoControlDevices(uint8_t temp, float light, float soil_moisture, SystemSettings *settings);
// 检查报警条件
void CheckAlarmConditions(uint8_t temp, uint8_t humi, float light, float soil_moisture,
                          SystemState *state, SystemSettings *settings, uint8_t sensor_valid);
// 处理报警
void HandleAlarm(SystemState *state);
// 显示主页面
void ShowMainPage(uint8_t temp, uint8_t humi, float light, float soil_moisture, SystemState *state);
// 显示设备状态页面
void ShowDeviceStatusPage(void);
// 显示设置页面
void ShowSettingPage(PageState page, SettingType setting, SystemSettings *settings);
// 显示模式页面
void ShowModePage(SystemState *state);
// 获取系统时间
uint32_t GetSystemTime(void);

// 定时器3中断处理函数（1ms）
// 每1毫秒触发一次，用于更新系统时间
void TIM3_IRQHandler(void)
{
    // 检查是否是定时器3的更新中断
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        system_time_ms++;            // 系统时间加1毫秒
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  // 清除中断标志位
    }
}

// 定时器3初始化函数
// 配置定时器3为1毫秒中断
void TIM3_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  // 定时器时间基数结构体
    NVIC_InitTypeDef NVIC_InitStructure;            // NVIC中断配置结构体
    
    // 使能定时器3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // 设置定时器周期为10-1（10个时钟周期）
    TIM_TimeBaseStructure.TIM_Period = 10 - 1;
    // 设置定时器预分频为7200-1（7200分频）
    // 72MHz / 7200 = 10kHz，10kHz / 10 = 1kHz（1ms）
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;
    // 时钟分割为1
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    // 向上计数模式
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    // 初始化定时器3
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // 使能定时器3更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 配置定时器3中断通道
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    // 抢占优先级为1
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    // 子优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    // 使能中断通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // 初始化NVIC
    NVIC_Init(&NVIC_InitStructure);
    
    // 使能定时器3
    TIM_Cmd(TIM3, ENABLE);
}

// 获取系统时间函数
// 返回当前系统时间（毫秒）
uint32_t GetSystemTime(void)
{
    return system_time_ms;
}

// 阿里云发布所有数据函数
// 将传感器数据和系统状态发布到阿里云物联网平台
static void Aliyun_PublishAll(uint8_t temp, uint8_t humi, float light, float soil_moisture,
                              SystemState *state, SystemSettings *settings)
{
    char send_data[512];  // 发送数据缓冲区

    // 格式化发送数据为JSON格式
    snprintf(send_data, sizeof(send_data),
             "{\"method\":\"thing/event/property/post\",\"id\":\"203302322\",\"params\":{"
             "\"temp\":%u,\"humi\":%u,\"light\":%d,\"soil\":%.1f,"
             "\"fan\":%u,\"pump\":%u,\"lamp\":%u,"
             "\"auto_mode\":%u,\"alarm_enable\":%u,\"alarm_active\":%u,\"alarm_type\":%u,"
             "\"temp_low\":%.1f,\"temp_high\":%.1f,"
             "\"target_humi\":%.1f,"
             "\"alarm_temp_low\":%.1f,"
             "\"alarm_temp_high\":%.1f,"
             "\"light_low\":%.1f,\"light_high\":%.1f,"
             "\"alarm_light_low\":%.1f,"
             "\"soil_low\":%.1f,\"soil_high\":%.1f,"
             "\"alarm_soil_low\":%.1f"
             "},\"version\":\"1.0.0\"}",
             (unsigned int)temp,              // 温度
             (unsigned int)humi,              // 湿度
             (int)light,                      // 光照
             soil_moisture,                   // 土壤湿度
             (unsigned int)device_state.fan_state,    // 风扇状态
             (unsigned int)device_state.pump_state,   // 水泵状态
             (unsigned int)device_state.led_state,    // LED状态
             (unsigned int)state->isAutoMode,         // 自动模式状态
             (unsigned int)state->alarmEnabled,       // 报警使能状态
             (unsigned int)state->alarmActive,        // 报警激活状态
             (unsigned int)state->currentAlarm,       // 报警类型
             settings->temp_low,            // 温度下限
             settings->temp_high,           // 温度上限
             settings->targetHumi,          // 湿度目标值
             settings->alarmTempLow,        // 低温报警阈值
             settings->alarmTemp,           // 高温报警阈值
             settings->light_low,           // 光照下限
             settings->light_high,          // 光照上限
             settings->alarmLight,          // 光照报警阈值
             settings->soil_low,            // 土壤湿度下限
             settings->soil_high,           // 土壤湿度上限
             settings->alarmSoil);          // 土壤报警阈值

    {
        static char topics_post[128];  // 发布主题缓冲区
        // 如果主题未初始化，则初始化
        if(topics_post[0] == 0)
        {
            snprintf(topics_post, sizeof(topics_post),
                     "/sys/%s/%s/thing/event/property/post",
                     ALIYUN_PRODUCT_KEY, ALIYUN_DEVICE_NAME);
        }
        // 发布数据到阿里云
        OneNet_Publish(topics_post, send_data);
    }
}

// 阿里云下行数据处理函数
// 处理从阿里云物联网平台接收到的控制指令
static void Aliyun_ApplyDownlink(const char *payload, SystemState *state, SystemSettings *settings)
{
    char *p;      // 字符串指针，用于解析JSON数据
    float fval;   // 浮点型变量，用于存储浮点数设置值
    int ival;     // 整型变量，用于存储整型设置值

    // 如果 payload 为空，直接返回
    if(payload == NULL) return;

    // 解析自动模式设置
    p = strstr(payload, "auto_mode");
    if(p) { p = strstr(p, ":"); if(p) { state->isAutoMode = (uint8_t)atoi(p + 1); } }

    // 解析报警使能设置
    p = strstr(payload, "alarm_enable");
    if(p) { p = strstr(p, ":"); if(p) { state->alarmEnabled = (uint8_t)atoi(p + 1); } }

    // 解析温度下限设置
    p = strstr(payload, "temp_low");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->temp_low = fval; } }

    // 解析温度上限设置
    p = strstr(payload, "temp_high");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->temp_high = fval; } }

    // 解析湿度目标值设置
    p = strstr(payload, "target_humi");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->targetHumi = fval; } }

    // 解析低温报警阈值设置
    p = strstr(payload, "alarm_temp_low");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->alarmTempLow = fval; } }

    // 解析高温报警阈值设置
    p = strstr(payload, "alarm_temp_high");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->alarmTemp = fval; } }

    // 解析光照下限设置
    p = strstr(payload, "light_low");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->light_low = fval; } }

    // 解析光照上限设置
    p = strstr(payload, "light_high");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->light_high = fval; } }

    // 解析光照报警阈值设置
    p = strstr(payload, "alarm_light_low");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->alarmLight = fval; } }

    // 解析土壤湿度下限设置
    p = strstr(payload, "soil_low");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->soil_low = fval; } }

    // 解析土壤湿度上限设置
    p = strstr(payload, "soil_high");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->soil_high = fval; } }

    // 解析土壤报警阈值设置
    p = strstr(payload, "alarm_soil_low");
    if(p) { p = strstr(p, ":"); if(p) { fval = (float)atof(p + 1); settings->alarmSoil = fval; } }

    // 解析蓝色LED控制指令
    p = strstr(payload, "led1");
    if(p) { p = strstr(p, ":"); if(p) { ival = atoi(p + 1); remote_led_blue = (uint8_t)(ival ? 1 : 0); remote_led_valid = 1; } }

    // 解析红色LED控制指令
    p = strstr(payload, "led2");
    if(p) { p = strstr(p, ":"); if(p) { ival = atoi(p + 1); remote_led_red = (uint8_t)(ival ? 1 : 0); remote_led_valid = 1; } }

    // 如果远程LED控制有效且没有报警，则执行LED控制
    if(remote_led_valid && state->alarmActive == 0)
    {
        if(remote_led_red) LED_R_ON(); else LED_R_OFF();
        if(remote_led_blue) LED_B_ON(); else LED_B_OFF();
    }

    // 如果不是自动模式，则解析设备控制指令
    if(state->isAutoMode == 0)
    {
        // 解析风扇控制指令
        p = strstr(payload, "fan");
        if(p && strstr(p, "fan_state") == NULL)
        {
            p = strstr(p, ":");
            if(p) { ival = atoi(p + 1); if(ival) { FAN_ON; device_state.fan_state = 1; } else { FAN_OFF; device_state.fan_state = 0; } }
        }

        // 解析水泵控制指令
        p = strstr(payload, "pump");
        if(p)
        {
            p = strstr(p, ":");
            if(p) { ival = atoi(p + 1); if(ival) { BUMP_ON; device_state.pump_state = 1; } else { BUMP_OFF; device_state.pump_state = 0; } }
        }

        // 解析LED模块控制指令
        p = strstr(payload, "lamp");
        if(p)
        {
            p = strstr(p, ":");
            if(p) { ival = atoi(p + 1); if(ival) { LED_MODULE_ON; device_state.led_state = 1; } else { LED_MODULE_OFF; device_state.led_state = 0; } }
        }
    }
}

// 自动控制设备函数（基于上下限的迟滞控制）
// 根据传感器数据自动控制风扇、LED灯和水泵
void AutoControlDevices(uint8_t temp, float light, float soil_moisture, SystemSettings *settings)
{
    // 温度控制风扇（降温）：基于报警阈值
    if(temp > settings->alarmTemp) {
        // 如果温度高于高温报警阈值且风扇未开启，则开启风扇
        if(device_state.fan_state == 0) { FAN_ON; device_state.fan_state = 1; }
    } else if(temp < settings->alarmTempLow) {
        // 如果温度低于低温报警阈值且风扇已开启，则关闭风扇
        if(device_state.fan_state == 1) { FAN_OFF; device_state.fan_state = 0; }
    }
    // 注：在高/低报警阈值之间时，风扇状态保持不变，避免频繁启停
    
    // 光照控制LED灯（补光）
    if(light < settings->light_low) {
        // 如果光照低于下限且LED未开启，则开启LED
        if(device_state.led_state == 0) { LED_MODULE_ON; device_state.led_state = 1; }
    } else if(light > settings->light_high) {
        // 如果光照高于上限且LED已开启，则关闭LED
        if(device_state.led_state == 1) { LED_MODULE_OFF; device_state.led_state = 0; }
    }
    
    // 土壤湿度控制水泵（浇水）
    if(soil_moisture < settings->soil_low) {
        // 如果土壤湿度低于下限且水泵未开启，则开启水泵
        if(device_state.pump_state == 0) { BUMP_ON; device_state.pump_state = 1; }
    } else if(soil_moisture > settings->soil_high) {
        // 如果土壤湿度高于上限且水泵已开启，则关闭水泵
        if(device_state.pump_state == 1) { BUMP_OFF; device_state.pump_state = 0; }
    }
}

// 检查报警条件函数
// 检查各种传感器数据是否触发报警条件
void CheckAlarmConditions(uint8_t temp, uint8_t humi, float light, float soil_moisture,
                          SystemState *state, SystemSettings *settings, uint8_t sensor_valid)
{
    uint8_t alarmCount = 0;         // 报警计数器
    AlarmType detectedAlarm = ALARM_NONE;  // 检测到的报警类型

    // 如果传感器数据无效，则不触发报警
    if (!sensor_valid) {
        state->alarmActive = 0;
        state->currentAlarm = ALARM_NONE;
        return;
    }

    // 检查温度是否高于高温报警阈值
    if(temp > settings->alarmTemp) {
        alarmCount++;
        detectedAlarm = ALARM_TEMP_HIGH;
    }
    // 检查温度是否低于低温报警阈值
    if(temp < settings->alarmTempLow) {
        alarmCount++;
        detectedAlarm = ALARM_TEMP_LOW;
    }
    // 检查光照是否低于光照报警阈值
    if(light < settings->alarmLight) {
        alarmCount++;
        if(detectedAlarm == ALARM_NONE)
            detectedAlarm = ALARM_LIGHT_LOW;
        else if(detectedAlarm != ALARM_TEMP_LOW)
            detectedAlarm = ALARM_COMBINED;
    }
    // 检查土壤湿度是否低于土壤报警阈值
    if(soil_moisture < settings->alarmSoil) {
        alarmCount++;
        if(detectedAlarm == ALARM_NONE)
            detectedAlarm = ALARM_SOIL_DRY;
        else if(detectedAlarm != ALARM_TEMP_LOW)
            detectedAlarm = ALARM_COMBINED;
    }

    // 如果有报警且报警已启用，则激活报警
    if(alarmCount > 0 && state->alarmEnabled) {
        state->alarmActive = 1;
        state->currentAlarm = detectedAlarm;
    } else {
        // 否则，关闭报警
        state->alarmActive = 0;
        state->currentAlarm = ALARM_NONE;
    }
}

// 处理报警函数
// 根据报警类型控制LED和蜂鸣器的状态
void HandleAlarm(SystemState *state)
{
    uint32_t currentTime = GetSystemTime();  // 获取当前系统时间
    uint32_t period;                         // 报警闪烁周期
    uint8_t led_r_on = 0, led_b_on = 0, buzz_on = 0;  // LED和蜂鸣器状态

    // 如果没有报警，则根据远程控制设置LED状态
    if(!state->alarmActive)
    {
        if(remote_led_valid)
        {
            // 如果远程控制有效，则按照远程指令设置LED
            if(remote_led_red) LED_R_ON(); else LED_R_OFF();
            if(remote_led_blue) LED_B_ON(); else LED_B_OFF();
            state->ledRState = remote_led_red;
            state->ledBState = remote_led_blue;
        }
        else
        {
            // 否则，关闭所有LED
            LED_R_OFF();
            LED_B_OFF();
            state->ledRState = 0;
            state->ledBState = 0;
        }
        // 关闭蜂鸣器
        Buzz_Off();
        state->buzzState = 0;
        return;
    }

    // 根据报警类型设置不同的闪烁模式
    switch(state->currentAlarm)
    {
        case ALARM_TEMP_LOW:  // 低温报警
            period = 200;  // 200ms周期
            {
                uint32_t t = currentTime % period;
                // 红蓝光交替闪烁
                if(t < period/2) { led_r_on = 1; led_b_on = 0; }
                else             { led_r_on = 0; led_b_on = 1; }
                buzz_on = 1;  // 蜂鸣器一直响
            }
            break;

        case ALARM_TEMP_HIGH:  // 高温报警
        case ALARM_LIGHT_LOW:  // 光照不足报警
        case ALARM_SOIL_DRY:   // 土壤干燥报警
            period = 500;  // 500ms周期
            {
                uint32_t t = currentTime % period;
                // 红灯闪烁
                led_r_on = (t < period/2) ? 1 : 0;
                led_b_on = 0;
                buzz_on = led_r_on;  // 蜂鸣器随红灯闪烁
            }
            break;

        case ALARM_COMBINED:  // 组合报警
            period = 500;  // 500ms周期
            {
                uint32_t t = currentTime % period;
                // 红灯闪烁
                led_r_on = (t < period/2) ? 1 : 0;
                led_b_on = 0;
                buzz_on = led_r_on;  // 蜂鸣器随红灯闪烁
            }
            break;

        default:
            break;
    }

    // 根据计算结果控制LED和蜂鸣器
    if(led_r_on) LED_R_ON(); else LED_R_OFF();
    if(led_b_on) LED_B_ON(); else LED_B_OFF();   
    if(buzz_on) Buzz_ON(); else Buzz_Off();

    // 更新系统状态
    state->ledRState = led_r_on;
    state->ledBState = led_b_on;                 
    state->buzzState = buzz_on;
}

// 主页面显示函数
// 在OLED屏幕上显示主页面，包括温度、湿度、光照、土壤湿度和报警状态
void ShowMainPage(uint8_t temp, uint8_t humi, float light, float soil_moisture, SystemState *state)
{
    char light_str[10];  // 光照值字符串缓冲区
    char soil_str[10];   // 土壤湿度字符串缓冲区
    
    OLED_Clear();  // 清屏
    
    // 第1行：温度、湿度
    OLED_ShowChinese(1, 1, 9);      // 显示"温"
    OLED_ShowString(1, 3, ":");     // 显示冒号
    OLED_ShowChar(1, 6, 'C');        // 显示温度单位
    OLED_ShowChinese(1, 8, 10);      // 显示"湿"
    OLED_ShowString(1, 10, ":");    // 显示冒号
    OLED_ShowChar(1, 14, '%');       // 显示湿度单位
    OLED_ShowNum(1, 4, temp, 2);     // 显示温度值
    OLED_ShowNum(1, 11, humi, 2);    // 显示湿度值
    
    // 报警图标
    if(state->alarmActive)
    {
        OLED_ShowChar(1, 15, '!');   // 显示感叹号
        // 根据报警类型显示不同的标识符
        switch(state->currentAlarm)
        {
            case ALARM_TEMP_HIGH: OLED_ShowChar(1, 16, 'H'); break;  // 高温
            case ALARM_TEMP_LOW:  OLED_ShowChar(1, 16, 'L'); break;  // 低温
            case ALARM_LIGHT_LOW: OLED_ShowChar(1, 16, 'D'); break;  // 光照不足
            case ALARM_SOIL_DRY:  OLED_ShowChar(1, 16, 'S'); break;  // 土壤干燥
            case ALARM_COMBINED:  OLED_ShowChar(1, 16, 'M'); break;  // 组合报警
            default: break;
        }
    }
    
    // 第2行：光照
    OLED_ShowChinese(2, 1, 13);      // 显示"光"
    OLED_ShowChinese(2, 3, 14);      // 显示"照"
    OLED_ShowString(2, 5, ":");     // 显示冒号
    OLED_ShowString(2, 12, "lx");    // 显示光照单位
    if(light < 1000)
    {
        // 光照小于1000，直接显示
        sprintf(light_str, "%4.0f", light);
        OLED_ShowString(2, 6, light_str);
    }
    else
    {
        // 光照大于等于1000，以千为单位显示
        sprintf(light_str, "%4.1f", light/1000.0);
        OLED_ShowString(2, 6, light_str);
        OLED_ShowChar(2, 11, 'k');    // 显示"k"
    }
    
    // 第3行：土壤湿度
    OLED_ShowChinese(3, 1, 0);       // 显示"土"
    OLED_ShowChinese(3, 3, 1);       // 显示"壤"
    OLED_ShowChinese(3, 5, 10);      // 显示"湿"
    OLED_ShowChinese(3, 7, 3);       // 显示"度"
    OLED_ShowString(3, 9, ":");     // 显示冒号
    OLED_ShowChar(3, 14, '%');       // 显示百分比单位
    if(soil_moisture >= 100.0f) sprintf(soil_str, "100");  // 土壤湿度≥100%
    else if(soil_moisture < 0.0f) sprintf(soil_str, "  0");  // 土壤湿度<0%
    else sprintf(soil_str, "%3.0f", soil_moisture);         // 正常范围
    OLED_ShowString(3, 10, soil_str);  // 显示土壤湿度值

    // 第4行：报警状态
    OLED_ShowString(4, 1, "A:");     // 显示"A:"
    if(state->alarmActive)
    {
        OLED_ShowString(4, 3, "ON ");  // 显示"ON"
        // 根据报警类型显示报警信息
        switch(state->currentAlarm)
        {
            case ALARM_TEMP_HIGH: OLED_ShowString(4, 7, "Temp High"); break;  // 高温报警
            case ALARM_TEMP_LOW:  OLED_ShowString(4, 7, "Temp Low"); break;  // 低温报警
            case ALARM_LIGHT_LOW: OLED_ShowString(4, 7, "Light Low"); break;  // 光照不足报警
            case ALARM_SOIL_DRY:  OLED_ShowString(4, 7, "Soil Dry"); break;  // 土壤干燥报警
            case ALARM_COMBINED:  OLED_ShowString(4, 7, "Multi"); break;  // 组合报警
            default: break;
        }
    }
    else
    {
        OLED_ShowString(4, 3, "OFF");  // 显示"OFF"
    }
}

// 设备状态页面显示函数
// 在OLED屏幕上显示设备状态页面，包括风扇、LED和水泵的状态
void ShowDeviceStatusPage(void)
{
    OLED_Clear();  // 清屏
    
    // 第1行：风扇状态
    OLED_ShowChinese(1, 1, 4);      // 显示"风"
    OLED_ShowChinese(1, 3, 5);      // 显示"扇"
    OLED_ShowString(1, 5, ":");     // 显示冒号
    if(device_state.fan_state == 1) OLED_ShowChinese(1, 7, 19);  // 显示"开"
    else OLED_ShowChinese(1, 7, 20);  // 显示"关"
    
    // 第2行：LED状态
    OLED_ShowChinese(2, 1, 6);      // 显示"灯"
    OLED_ShowString(2, 5, ":");     // 显示冒号
    if(device_state.led_state == 1) OLED_ShowChinese(2, 7, 19);  // 显示"开"
    else OLED_ShowChinese(2, 7, 20);  // 显示"关"
    
    // 第3行：水泵状态
    OLED_ShowChinese(3, 1, 17);     // 显示"水"
    OLED_ShowChinese(3, 3, 18);     // 显示"泵"
    OLED_ShowString(3, 5, ":");     // 显示冒号
    if(device_state.pump_state == 1) OLED_ShowChinese(3, 7, 19);  // 显示"开"
    else OLED_ShowChinese(3, 7, 20);  // 显示"关"
    
    // 第4行：报警状态
    OLED_ShowString(4, 1, "ALARM");  // 显示"ALARM"
    OLED_ShowString(4, 6, ":");     // 显示冒号
    OLED_ShowString(4, 7, "ENABLED");  // 显示"ENABLED"
}

// 设置页面显示函数
// 在OLED屏幕上显示不同的设置页面，包括温湿度设置、光照设置和土壤湿度设置
void ShowSettingPage(PageState page, SettingType setting, SystemSettings *settings)
{
    OLED_Clear();  // 清屏
    
    switch(page)
    {
        case PAGE_TEMP_HUMI_SET:  // 温湿度设置页面
            // 第1行：温湿度设置
            OLED_ShowChinese(1, 1, 11);  // 显示"环"
            OLED_ShowChinese(1, 3, 12);  // 显示"境"
            OLED_ShowChinese(1, 5, 9);   // 显示"温"
            OLED_ShowChinese(1, 7, 10);  // 显示"湿"
            OLED_ShowChinese(1, 9, 3);   // 显示"度"
            OLED_ShowChinese(1, 11, 7);  // 显示"设"
            OLED_ShowChinese(1, 13, 8);  // 显示"置"

            // 第2行：湿度目标值
            OLED_ShowChinese(2, 1, 10);  // 显示"湿"
            OLED_ShowString(2, 3, ":");     // 显示冒号
            OLED_ShowNum(2, 4, (uint16_t)settings->targetHumi, 2);  // 显示湿度目标值
            OLED_ShowChar(2, 6, '%');       // 显示百分比单位
            if(setting == SETTING_HUMI)     // 如果当前选中的是湿度设置
                OLED_ShowString(2, 8, "<-");  // 显示选中标记

            // 第3行：温度下限和上限
            OLED_ShowChinese(3, 1, 9);   // 显示"温"
            OLED_ShowChinese(3, 3, 27);  // 显示"下"
            OLED_ShowString(3, 5, ":");     // 显示冒号
            OLED_ShowNum(3, 6, (uint16_t)settings->temp_low, 2);  // 显示温度下限
            OLED_ShowChar(3, 8, 'C');        // 显示温度单位
            if(setting == SETTING_TEMP_LOW)  // 如果当前选中的是温度下限设置
                OLED_ShowString(3, 9, "<");   // 显示选中标记

            OLED_ShowChinese(3, 10, 26); // 显示"上"
            OLED_ShowString(3, 12, ":");    // 显示冒号
            OLED_ShowNum(3, 13, (uint16_t)settings->temp_high, 2);  // 显示温度上限
            OLED_ShowChar(3, 15, 'C');       // 显示温度单位
            if(setting == SETTING_TEMP_HIGH)  // 如果当前选中的是温度上限设置
                OLED_ShowString(3, 16, "<");   // 显示选中标记

            // 第4行：低温报警和高温报警
            OLED_ShowString(4, 1, "L:");     // 显示"L:"
            OLED_ShowNum(4, 3, (uint16_t)settings->alarmTempLow, 2);  // 显示低温报警阈值
            OLED_ShowChar(4, 5, 'C');        // 显示温度单位
            if(setting == SETTING_TEMP_LOW_ALARM)  // 如果当前选中的是低温报警设置
                OLED_ShowString(4, 6, "<-");  // 显示选中标记

            OLED_ShowString(4, 8, "H:");     // 显示"H:"
            OLED_ShowNum(4, 10, (uint16_t)settings->alarmTemp, 2);  // 显示高温报警阈值
            OLED_ShowChar(4, 12, 'C');        // 显示温度单位
            if(setting == SETTING_TEMP_HIGH_ALARM)  // 如果当前选中的是高温报警设置
                OLED_ShowString(4, 13, "<-");  // 显示选中标记
            break;

        case PAGE_LIGHT_SET:  // 光照设置页面
            // 第1行：光照设置
            OLED_ShowChinese(1, 1, 13);  // 显示"光"
            OLED_ShowChinese(1, 3, 14);  // 显示"照"
            OLED_ShowChinese(1, 5, 7);   // 显示"设"
            OLED_ShowChinese(1, 7, 8);   // 显示"置"

            // 第2行：光照下限
            OLED_ShowChinese(2, 1, 27);  // 显示"下"
            OLED_ShowChinese(2, 3, 28);  // 显示"限"
            OLED_ShowString(2, 5, ":");     // 显示冒号
            OLED_ShowNum(2, 6, (uint16_t)settings->light_low, 4);  // 显示光照下限
            OLED_ShowString(2, 10, "lx");    // 显示光照单位
            if(setting == SETTING_LIGHT_LOW)  // 如果当前选中的是光照下限设置
                OLED_ShowString(2, 13, "<-");  // 显示选中标记

            // 第3行：光照上限
            OLED_ShowChinese(3, 1, 26);  // 显示"上"
            OLED_ShowChinese(3, 3, 28);  // 显示"限"
            OLED_ShowString(3, 5, ":");     // 显示冒号
            OLED_ShowNum(3, 6, (uint16_t)settings->light_high, 4);  // 显示光照上限
            OLED_ShowString(3, 10, "lx");    // 显示光照单位
            if(setting == SETTING_LIGHT_HIGH)  // 如果当前选中的是光照上限设置
                OLED_ShowString(3, 13, "<-");  // 显示选中标记

            // 第4行：光照报警
            OLED_ShowString(4, 1, "Alarm:");  // 显示"Alarm:"
            OLED_ShowNum(4, 7, (uint16_t)settings->alarmLight, 4);  // 显示光照报警阈值
            OLED_ShowString(4, 11, "lx");    // 显示光照单位
            if(setting == SETTING_LIGHT_ALARM)  // 如果当前选中的是光照报警设置
                OLED_ShowString(4, 14, "<-");  // 显示选中标记
            break;

        case PAGE_SOIL_SET:  // 土壤湿度设置页面
            // 第1行：土壤湿度设置
            OLED_ShowChinese(1, 1, 0);   // 显示"土"
            OLED_ShowChinese(1, 3, 1);   // 显示"壤"
            OLED_ShowChinese(1, 5, 10);  // 显示"湿"
            OLED_ShowChinese(1, 7, 3);   // 显示"度"
            OLED_ShowChinese(1, 9, 7);   // 显示"设"
            OLED_ShowChinese(1, 11, 8);  // 显示"置"

            // 第2行：土壤下限
            OLED_ShowChinese(2, 1, 27);  // 显示"下"
            OLED_ShowChinese(2, 3, 28);  // 显示"限"
            OLED_ShowString(2, 5, ":");     // 显示冒号
            OLED_ShowNum(2, 6, (uint16_t)settings->soil_low, 2);  // 显示土壤湿度下限
            OLED_ShowChar(2, 8, '%');       // 显示百分比单位
            if(setting == SETTING_SOIL_LOW)  // 如果当前选中的是土壤湿度下限设置
                OLED_ShowString(2, 10, "<-");  // 显示选中标记

            // 第3行：土壤上限
            OLED_ShowChinese(3, 1, 26);  // 显示"上"
            OLED_ShowChinese(3, 3, 28);  // 显示"限"
            OLED_ShowString(3, 5, ":");     // 显示冒号
            OLED_ShowNum(3, 6, (uint16_t)settings->soil_high, 2);  // 显示土壤湿度上限
            OLED_ShowChar(3, 8, '%');       // 显示百分比单位
            if(setting == SETTING_SOIL_HIGH)  // 如果当前选中的是土壤湿度上限设置
                OLED_ShowString(3, 10, "<-");  // 显示选中标记

            // 第4行：土壤报警
            OLED_ShowString(4, 1, "Alarm:");  // 显示"Alarm:"
            OLED_ShowNum(4, 7, (uint16_t)settings->alarmSoil, 2);  // 显示土壤报警阈值
            OLED_ShowChar(4, 9, '%');       // 显示百分比单位
            if(setting == SETTING_SOIL_ALARM)  // 如果当前选中的是土壤报警设置
                OLED_ShowString(4, 11, "<-");  // 显示选中标记
            break;

        default:
            break;
    }
}

// 模式页面显示函数
// 在OLED屏幕上显示模式设置页面，包括自动/手动模式切换和各设备的状态
void ShowModePage(SystemState *state)
{
    OLED_Clear();

    // 行1：标题“模式设置”
    OLED_ShowChinese(1, 1, 21);   // 模
    OLED_ShowChinese(1, 3, 22);   // 式
    OLED_ShowChinese(1, 5, 7);    // 设
    OLED_ShowChinese(1, 7, 8);    // 置

    // 行2：模式 和 风扇
    if (state->modeSelectedItem == 0)
        OLED_ShowChar(2, 1, '>');
    else
        OLED_ShowChar(2, 1, ' ');
    OLED_ShowChinese(2, 2, 21);   // 模
    OLED_ShowChinese(2, 4, 22);   // 式
    OLED_ShowChar(2, 6, ':');
    if (state->isAutoMode) {
        OLED_ShowChinese(2, 7, 23);   // 自
        
    } else {
        OLED_ShowChinese(2, 7, 24);   // 手
        
    }

    if (state->modeSelectedItem == 1)
        OLED_ShowChar(2, 9, '>');
    else
        OLED_ShowChar(2, 9, ' ');
    OLED_ShowChinese(2, 10, 4);    // 风
    OLED_ShowChinese(2, 12, 5);    // 扇
    OLED_ShowChar(2, 14, ':');
    if (device_state.fan_state)
        OLED_ShowChinese(2, 15, 19); // 开
    else
        OLED_ShowChinese(2, 15, 20); // 关

    // 行3：水泵 和 灯
    if (state->modeSelectedItem == 2)
        OLED_ShowChar(3, 1, '>');
    else
        OLED_ShowChar(3, 1, ' ');
    OLED_ShowChinese(3, 2, 17);   // 水
    OLED_ShowChinese(3, 4, 18);   // 泵
    OLED_ShowChar(3, 6, ':');
    if (device_state.pump_state)
        OLED_ShowChinese(3, 7, 19); // 开
    else
        OLED_ShowChinese(3, 7, 20); // 关

    if (state->modeSelectedItem == 3)
        OLED_ShowChar(3, 9, '>');
    else
        OLED_ShowChar(3, 9, ' ');
    OLED_ShowChinese(3, 10, 6);    // 灯
    OLED_ShowChar(3, 12, ':');
    if (device_state.led_state)
        OLED_ShowChinese(3, 13, 19); // 开
    else
        OLED_ShowChinese(3, 13, 20); // 关

    // 行4：操作提示
    OLED_ShowString(4, 1, "K4:NextK2:Switch");
}

// 按键状态记录
static uint32_t key1_last_press_time = 0;
static uint8_t key1_press_count = 0;

// 页面切换逻辑函数（仅用于非设置模式）
// 在非设置模式下，切换到下一个页面
void SwitchToNextPage(SystemState *state)
{
    if(!state->isSettingMode)  // 如果不是设置模式
    {
        uint8_t nextPage = (uint8_t)state->currentPage + 1;  // 计算下一个页面
        if(nextPage > PAGE_MODE_SET)  // 如果超过最后一个页面
            nextPage = PAGE_MAIN;     // 回到主页面
        state->currentPage = (PageState)nextPage;  // 更新当前页面
    }
}

// 按键1处理函数（在设置模式下切换选项，双击切换页面）
// 处理按键1的输入，根据当前模式执行不同的操作
void HandleKey1(SystemState *state)
{
    uint32_t current_time = GetSystemTime();  // 获取当前系统时间
    
    if(state->isSettingMode)  // 如果是设置模式
    {
        if((current_time - key1_last_press_time) < 500) // 双击检测（500ms内连续按两次）
        {
            key1_press_count++;
            if(key1_press_count >= 2)  // 如果连续按了两次
            {
                // 进入下一个设置页面
                switch(state->currentPage)
                {
                    case PAGE_TEMP_HUMI_SET:
                        state->currentPage = PAGE_LIGHT_SET;
                        state->selectedSetting = SETTING_LIGHT_LOW;
                        break;
                    case PAGE_LIGHT_SET:
                        state->currentPage = PAGE_SOIL_SET;
                        state->selectedSetting = SETTING_SOIL_LOW;
                        break;
                    case PAGE_SOIL_SET:
                        state->currentPage = PAGE_TEMP_HUMI_SET;
                        state->selectedSetting = SETTING_TEMP_LOW;
                        break;
                    default:
                        break;
                }
                key1_press_count = 0;  // 重置按键计数
            }
        }
        else
        {
            // 短按：在当前页面切换选项
            switch(state->currentPage)
            {
                case PAGE_TEMP_HUMI_SET:
                    // 顺序：湿度目标值 -> 温度下限 -> 温度上限 -> 低温报警 -> 高温报警 -> 循环
                    if(state->selectedSetting == SETTING_HUMI)
                        state->selectedSetting = SETTING_TEMP_LOW;
                    else if(state->selectedSetting == SETTING_TEMP_LOW)
                        state->selectedSetting = SETTING_TEMP_HIGH;
                    else if(state->selectedSetting == SETTING_TEMP_HIGH)
                        state->selectedSetting = SETTING_TEMP_LOW_ALARM;
                    else if(state->selectedSetting == SETTING_TEMP_LOW_ALARM)
                        state->selectedSetting = SETTING_TEMP_HIGH_ALARM;
                    else // SETTING_TEMP_HIGH_ALARM
                        state->selectedSetting = SETTING_HUMI;
                    break;
                case PAGE_LIGHT_SET:
                    // 顺序：LIGHT_LOW -> LIGHT_HIGH -> LIGHT_ALARM -> 循环
                    if(state->selectedSetting == SETTING_LIGHT_LOW)
                        state->selectedSetting = SETTING_LIGHT_HIGH;
                    else if(state->selectedSetting == SETTING_LIGHT_HIGH)
                        state->selectedSetting = SETTING_LIGHT_ALARM;
                    else
                        state->selectedSetting = SETTING_LIGHT_LOW;
                    break;
                case PAGE_SOIL_SET:
                    // 顺序：SOIL_LOW -> SOIL_HIGH -> SOIL_ALARM -> 循环
                    if(state->selectedSetting == SETTING_SOIL_LOW)
                        state->selectedSetting = SETTING_SOIL_HIGH;
                    else if(state->selectedSetting == SETTING_SOIL_HIGH)
                        state->selectedSetting = SETTING_SOIL_ALARM;
                    else
                        state->selectedSetting = SETTING_SOIL_LOW;
                    break;
                default:
                    break;
            }
            key1_press_count = 1;  // 重置按键计数为1
        }
        key1_last_press_time = current_time;  // 更新上次按键时间
        state->settingStartTime = current_time;  // 更新设置模式开始时间
    }
    else
    {
        SwitchToNextPage(state);  // 非设置模式下，切换到下一个页面
    }
}

// 主函数
// 程序的入口点，包含系统初始化、硬件初始化和主循环
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemInit();
    
    uint8_t temp = 0, humi = 0, ret;
    float light = 0.0, soil_moisture = 0.0;
    
    // 初始化系统状态
    SystemState sysState;  // 系统状态结构体
    sysState.currentPage = PAGE_MAIN;  // 当前页面设置为主页面
    sysState.previousPage = PAGE_MAIN;  // 上一个页面设置为主页面
    sysState.selectedSetting = SETTING_TEMP_LOW;  // 当前选中的设置项为温度下限
    sysState.isSettingMode = 0;  // 初始为非设置模式
    sysState.settingStartTime = 0;  // 设置模式开始时间初始化为0
    sysState.alarmEnabled = 1;  // 报警功能默认启用
    sysState.alarmActive = 0;  // 初始无报警
    sysState.currentAlarm = ALARM_NONE;  // 当前报警类型为无
    sysState.alarmToggleTime = 0;  // 报警状态切换时间初始化为0
    sysState.ledRState = 0;  // 红色LED初始状态为关闭
    sysState.ledBState = 0;  // 蓝色LED初始状态为关闭
    sysState.buzzState = 0;  // 蜂鸣器初始状态为关闭
    sysState.isAutoMode = 1;  // 初始为自动模式
    sysState.modeSelectedItem = 0;  // 模式页面默认选中第0项（模式选择）
    
    // 初始化系统设置（上下限 + 湿度单值 + 报警）
    settings.temp_low = 25.0f;
    settings.temp_high = 30.0f;
    settings.targetHumi = 60.0f;
    settings.alarmTempLow = 10.0f;
    settings.alarmTemp = 35.0f;   // 高温报警阈值
    settings.light_low = 300.0f;
    settings.light_high = 800.0f;
    settings.alarmLight = 100.0f;
    settings.soil_low = 20.0f;
    settings.soil_high = 40.0f;
    settings.alarmSoil = 15.0f;
    
    // 硬件初始化
    uart1_Init(115200);  // 初始化串口1，波特率115200，用于调试和阿里云通信
    {
        // 读取并打印复位原因，用于调试系统启动状态
        uint32_t csr = RCC->CSR;  // 读取复位状态寄存器
        DBG_PRINTF("[RST] CSR=0x%08lX\r\n", (unsigned long)csr);  // 打印复位状态寄存器值
        if(csr & RCC_CSR_PORRSTF)  DBG_PRINTF("[RST] POR/PDR\r\n");  // 上电/掉电复位
        if(csr & RCC_CSR_PINRSTF)  DBG_PRINTF("[RST] NRST\r\n");     // 外部复位（NRST引脚）
        if(csr & RCC_CSR_SFTRSTF)  DBG_PRINTF("[RST] SOFT\r\n");     // 软件复位
        if(csr & RCC_CSR_IWDGRSTF) DBG_PRINTF("[RST] IWDG\r\n");    // 独立看门狗复位
        if(csr & RCC_CSR_WWDGRSTF) DBG_PRINTF("[RST] WWDG\r\n");    // 窗口看门狗复位
        if(csr & RCC_CSR_LPWRRSTF) DBG_PRINTF("[RST] LPWR\r\n");    // 低功耗复位
        RCC_ClearFlag();  // 清除复位标志
    }
    DBG_PRINTF("[BOOT] start\r\n");  // 打印启动信息
    DBG_PRINTF("[WIFI] ssid=%s\r\n", ALIYUN_WIFI_SSID);  // 打印WiFi SSID
    DBG_PRINTF("[ALIYUN] pk=%s dn=%s host=%s:%d\r\n", ALIYUN_PRODUCT_KEY, ALIYUN_DEVICE_NAME, ALIYUN_MQTT_HOST, ALIYUN_MQTT_PORT);  // 打印阿里云配置
    
    // 初始化各模块
    TIM3_Init();        // 初始化定时器3（用于系统时间计数，1ms中断）
    OLED_Init();        // 初始化OLED显示屏，用于显示系统状态和传感器数据
    DHT11_Init();       // 初始化DHT11温湿度传感器，用于读取环境温湿度
    MyI2C_Init();       // 初始化I2C通信，用于与GY302和OLED通信
    GY302_Init();       // 初始化GY302光照传感器，用于读取环境光照强度
    Soil_Init();        // 初始化土壤湿度传感器，用于读取土壤湿度
    LED_Init();         // 初始化LED指示灯，用于状态指示和报警
    Buzz_Init();        // 初始化蜂鸣器，用于报警发声
    Key_Init();         // 初始化按键，用于用户交互
    FAN_Init();         // 初始化风扇，用于温度控制
    BUMP_Init();        // 初始化水泵，用于土壤湿度控制
    LED_MODULE_Init();  // 初始化LED模块，用于补光
    
    // 初始状态：关闭所有设备，确保系统启动时处于安全状态
    FAN_OFF;            // 关闭风扇
    BUMP_OFF;           // 关闭水泵
    LED_MODULE_OFF;     // 关闭LED模块
    LED_R_OFF();        // 关闭红色LED
    LED_B_OFF();        // 关闭蓝色LED
    Buzz_Off();         // 关闭蜂鸣器
    
    // 显示启动信息
    OLED_ShowString(1, 1, "System Starting...");  // 显示系统启动中
    OLED_ShowString(3, 1, "Alarm System ON");     // 显示报警系统已开启
    Delay_ms(1000);     // 延时1秒，让用户看到启动信息
    OLED_Clear();       // 清屏，准备显示正常运行界面

    // 配置阿里云MQTT订阅主题
    {
        static char topics_set[128];
        snprintf(topics_set, sizeof(topics_set),
                 "/sys/%s/%s/thing/service/property/set",
                 ALIYUN_PRODUCT_KEY, ALIYUN_DEVICE_NAME);
        OneNet_ConfigSubscribeTopic(topics_set);  // 配置订阅主题
    }

    // 时间和状态变量初始化
    uint32_t last_sensor_update = 0;     // 上次传感器更新时间
    uint8_t display_needs_refresh = 1;   // 显示是否需要刷新
    uint32_t last_display_refresh = 0;   // 上次显示刷新时间
    uint32_t last_aliyun_post = 0;       // 上次阿里云发布时间
    uint8_t aliyun_force_post = 1;       // 强制发布阿里云数据
    uint8_t dht_valid = 0;               // DHT11传感器数据是否有效

    while(1)  // 主循环
    {
        ESP8266_Service();  // 处理ESP8266网络服务
        OneNet_Service();   // 处理OneNet服务

        // 检查ESP8266 TCP连接状态
        if(ESP8266_IsTcpConnected())
        {
            unsigned char *ipd = ESP8266_GetIPD(0);  // 获取网络数据
            if(ipd != NULL)
            {
                OneNet_RevPro(ipd);  // 处理接收的数据
            }
        }

        // 每2秒更新一次传感器数据，确保数据采集的实时性和稳定性
        if((GetSystemTime() - last_sensor_update) >= 2000)
        {
            last_sensor_update = GetSystemTime();  // 更新传感器更新时间

            // 读取DHT11温湿度数据
            ret = DHT11_Read_Data(&temp, &humi);  // 读取温度和湿度值
            if(ret == SUCCESS)
                dht_valid = 1;  // 数据有效
            else
                dht_valid = 0;  // 数据无效

            // 读取光照传感器数据
            light = GY302_ReadLight();  // 读取光照强度值
            if(light <= 0) light = 0;  // 确保光照值非负

            // 读取土壤湿度传感器数据
            soil_moisture = Soil_Value_Conversion();  // 读取并转换土壤湿度值
            if(soil_moisture < 0.0f) soil_moisture = 0.0f;  // 确保土壤湿度非负
            else if(soil_moisture > 100.0f) soil_moisture = 100.0f;  // 确保土壤湿度不超过100%

            // 如果是自动模式，根据传感器数据自动控制设备
            if(sysState.isAutoMode)
            {
                AutoControlDevices(temp, light, soil_moisture, &settings);  // 调用自动控制函数
            }

            // 检查是否满足报警条件
            CheckAlarmConditions(temp, humi, light, soil_moisture, &sysState, &settings, dht_valid);  // 检查各传感器数据是否触发报警

            // 标记显示需要刷新，确保显示屏及时更新最新数据
            display_needs_refresh = 1;
        }

        // 处理阿里云下行数据
        if(OneNet_IsMqttConnected() && OneNet_IsSubscribed())
        {
            const char *rx = OneNet_GetRxPayload();  // 获取接收的载荷
            if(rx != NULL)
            {
                Aliyun_ApplyDownlink(rx, &sysState, &settings);  // 处理下行数据
                OneNet_ClearRxPayload();  // 清除接收的载荷
                aliyun_force_post = 1;  // 强制发布数据
                display_needs_refresh = 1;  // 标记显示需要刷新
            }
        }

        // 每5秒或强制发布时，向阿里云发布数据
        if(OneNet_IsMqttConnected() && OneNet_IsSubscribed())
        {
            if((GetSystemTime() - last_aliyun_post) >= 5000 || aliyun_force_post)
            {
                last_aliyun_post = GetSystemTime();
                aliyun_force_post = 0;
                Aliyun_PublishAll(temp, humi, light, soil_moisture, &sysState, &settings);  // 发布所有数据
            }
        }

        HandleAlarm(&sysState);

        // 按键1处理（短按）
        if(Key_Scan_NonBlock(KEY_PORT, KEY1_PIN))  // 非阻塞式按键扫描
        {
            if(sysState.currentPage != PAGE_MODE_SET)  // 不在模式页面时
            {
                HandleKey1(&sysState);  // 处理按键1逻辑（切换设置项或页面）
                display_needs_refresh = 1;  // 标记显示需要刷新
                if(sysState.isSettingMode)  // 如果进入设置模式
                    sysState.settingStartTime = GetSystemTime();  // 更新设置模式开始时间
            }
        }

        // 按键1长按处理（进入设置模式）
        if(CheckLongPressKey1())  // 检查按键1是否长按
        {
            if(!sysState.isSettingMode)  // 如果未处于设置模式
            {
                sysState.isSettingMode = 1;  // 进入设置模式
                sysState.currentPage = PAGE_TEMP_HUMI_SET;  // 切换到温湿度设置页面
                sysState.selectedSetting = SETTING_HUMI;  // 默认选中湿度设置项
                sysState.settingStartTime = GetSystemTime();  // 更新设置模式开始时间
                display_needs_refresh = 1;  // 标记显示需要刷新
            }
        }

        // 按键4长按处理（进入/退出模式页面）
        if(CheckLongPressKey4())  // 检查按键4是否长按
        {
            if(sysState.currentPage != PAGE_MODE_SET)  // 不在模式页面时
            {
                sysState.previousPage = sysState.currentPage;  // 保存当前页面
                sysState.currentPage = PAGE_MODE_SET;  // 切换到模式页面
                sysState.modeSelectedItem = 0;  // 默认选中模式选项
            }
            else  // 已在模式页面时
            {
                sysState.currentPage = sysState.previousPage;  // 返回到之前的页面
            }
            display_needs_refresh = 1;  // 标记显示需要刷新
        }

        // 按键4短按处理（在模式页面中切换选项）
        if(Key_Scan_NonBlock(KEY4_PORT, KEY4_PIN))  // 非阻塞式按键扫描
        {
            if(sysState.currentPage == PAGE_MODE_SET)  // 在模式页面时
            {
                sysState.modeSelectedItem++;  // 切换到下一个选项
                if(sysState.modeSelectedItem >= 4) sysState.modeSelectedItem = 0;  // 循环切换
                display_needs_refresh = 1;  // 标记显示需要刷新
            }
        }

        // KEY2 增加（用于增加设置值或切换状态）
        if(Key_Scan_NonBlock(KEY_PORT, KEY2_PIN))  // 非阻塞式按键扫描
        {
            if(sysState.isSettingMode)  // 在设置模式时
            {
                switch(sysState.selectedSetting)  // 根据当前选中的设置项
                {
                    case SETTING_TEMP_LOW:  // 温度下限
                        settings.temp_low = (settings.temp_low < settings.temp_high) ? settings.temp_low + 1.0f : settings.temp_high;
                        break;
                    case SETTING_TEMP_HIGH:  // 温度上限
                        settings.temp_high = (settings.temp_high < 50.0f) ? settings.temp_high + 1.0f : 50.0f;
                        break;
                    case SETTING_HUMI:  // 湿度目标值
                        settings.targetHumi = (settings.targetHumi < 100.0f) ? settings.targetHumi + 1.0f : 100.0f;
                        break;
                    case SETTING_TEMP_LOW_ALARM:  // 低温报警阈值
                        settings.alarmTempLow = (settings.alarmTempLow < 50.0f) ? settings.alarmTempLow + 1.0f : 50.0f;
                        break;
                    case SETTING_TEMP_HIGH_ALARM:  // 高温报警阈值
                        settings.alarmTemp = (settings.alarmTemp < 50.0f) ? settings.alarmTemp + 1.0f : 50.0f;
                        break;
                    case SETTING_LIGHT_LOW:  // 光照下限
                        settings.light_low = (settings.light_low < settings.light_high) ? settings.light_low + 100.0f : settings.light_high;
                        break;
                    case SETTING_LIGHT_HIGH:  // 光照上限
                        settings.light_high = (settings.light_high < 10000.0f) ? settings.light_high + 100.0f : 10000.0f;
                        break;
                    case SETTING_LIGHT_ALARM:  // 光照报警阈值
                        settings.alarmLight = (settings.alarmLight < 10000.0f) ? settings.alarmLight + 100.0f : 10000.0f;
                        break;
                    case SETTING_SOIL_LOW:  // 土壤湿度下限
                        settings.soil_low = (settings.soil_low < settings.soil_high) ? settings.soil_low + 1.0f : settings.soil_high;
                        break;
                    case SETTING_SOIL_HIGH:  // 土壤湿度上限
                        settings.soil_high = (settings.soil_high < 100.0f) ? settings.soil_high + 1.0f : 100.0f;
                        break;
                    case SETTING_SOIL_ALARM:  // 土壤报警阈值
                        settings.alarmSoil = (settings.alarmSoil < 100.0f) ? settings.alarmSoil + 1.0f : 100.0f;
                        break;
                    default: break;
                }
                display_needs_refresh = 1;  // 标记显示需要刷新
                sysState.settingStartTime = GetSystemTime();  // 更新设置模式开始时间
                aliyun_force_post = 1;  // 强制发布数据到阿里云
            }
            else if(sysState.currentPage == PAGE_MODE_SET)  // 在模式页面时
            {
                switch(sysState.modeSelectedItem)  // 根据当前选中的模式选项
                {
                    case 0: sysState.isAutoMode = !sysState.isAutoMode; break;  // 切换自动/手动模式
                    case 1: if(device_state.fan_state) { FAN_OFF; device_state.fan_state = 0; } else { FAN_ON; device_state.fan_state = 1; } break;  // 切换风扇状态
                    case 2: if(device_state.pump_state) { BUMP_OFF; device_state.pump_state = 0; } else { BUMP_ON; device_state.pump_state = 1; } break;  // 切换水泵状态
                    case 3: if(device_state.led_state) { LED_MODULE_OFF; device_state.led_state = 0; } else { LED_MODULE_ON; device_state.led_state = 1; } break;  // 切换LED状态
                    default: break;
                }
                display_needs_refresh = 1;  // 标记显示需要刷新
                aliyun_force_post = 1;  // 强制发布数据到阿里云
            }
        }

        // KEY3 减少（用于减少设置值或切换状态）
        if(Key_Scan_NonBlock(KEY_PORT, KEY3_PIN))  // 非阻塞式按键扫描
        {
            if(sysState.isSettingMode)  // 在设置模式时
            {
                switch(sysState.selectedSetting)  // 根据当前选中的设置项
                {
                    case SETTING_TEMP_LOW:  // 温度下限
                        settings.temp_low = (settings.temp_low > 0.0f) ? settings.temp_low - 1.0f : 0.0f;
                        break;
                    case SETTING_TEMP_HIGH:  // 温度上限
                        settings.temp_high = (settings.temp_high > settings.temp_low) ? settings.temp_high - 1.0f : settings.temp_low;
                        break;
                    case SETTING_HUMI:  // 湿度目标值
                        settings.targetHumi = (settings.targetHumi > 0.0f) ? settings.targetHumi - 1.0f : 0.0f;
                        break;
                    case SETTING_TEMP_LOW_ALARM:  // 低温报警阈值
                        settings.alarmTempLow = (settings.alarmTempLow > 0.0f) ? settings.alarmTempLow - 1.0f : 0.0f;
                        break;
                    case SETTING_TEMP_HIGH_ALARM:  // 高温报警阈值
                        settings.alarmTemp = (settings.alarmTemp > 0.0f) ? settings.alarmTemp - 1.0f : 0.0f;
                        break;
                    case SETTING_LIGHT_LOW:  // 光照下限
                        settings.light_low = (settings.light_low > 0.0f) ? settings.light_low - 100.0f : 0.0f;
                        break;
                    case SETTING_LIGHT_HIGH:  // 光照上限
                        settings.light_high = (settings.light_high > settings.light_low) ? settings.light_high - 100.0f : settings.light_low;
                        break;
                    case SETTING_LIGHT_ALARM:  // 光照报警阈值
                        settings.alarmLight = (settings.alarmLight > 0.0f) ? settings.alarmLight - 100.0f : 0.0f;
                        break;
                    case SETTING_SOIL_LOW:  // 土壤湿度下限
                        settings.soil_low = (settings.soil_low > 0.0f) ? settings.soil_low - 1.0f : 0.0f;
                        break;
                    case SETTING_SOIL_HIGH:  // 土壤湿度上限
                        settings.soil_high = (settings.soil_high > settings.soil_low) ? settings.soil_high - 1.0f : settings.soil_low;
                        break;
                    case SETTING_SOIL_ALARM:  // 土壤报警阈值
                        settings.alarmSoil = (settings.alarmSoil > 0.0f) ? settings.alarmSoil - 1.0f : 0.0f;
                        break;
                    default: break;
                }
                display_needs_refresh = 1;  // 标记显示需要刷新
                sysState.settingStartTime = GetSystemTime();  // 更新设置模式开始时间
                aliyun_force_post = 1;  // 强制发布数据到阿里云
            }
            else if(sysState.currentPage == PAGE_MODE_SET)  // 在模式页面时
            {
                switch(sysState.modeSelectedItem)  // 根据当前选中的模式选项
                {
                    case 0: sysState.isAutoMode = !sysState.isAutoMode; break;  // 切换自动/手动模式
                    case 1: if(device_state.fan_state) { FAN_OFF; device_state.fan_state = 0; } else { FAN_ON; device_state.fan_state = 1; } break;  // 切换风扇状态
                    case 2: if(device_state.pump_state) { BUMP_OFF; device_state.pump_state = 0; } else { BUMP_ON; device_state.pump_state = 1; } break;  // 切换水泵状态
                    case 3: if(device_state.led_state) { LED_MODULE_OFF; device_state.led_state = 0; } else { LED_MODULE_ON; device_state.led_state = 1; } break;  // 切换LED状态
                    default: break;
                }
                display_needs_refresh = 1;  // 标记显示需要刷新
                aliyun_force_post = 1;  // 强制发布数据到阿里云
            }
        }

        // 设置模式超时处理
        if(sysState.isSettingMode)
        {
            if((GetSystemTime() - sysState.settingStartTime) >= 10000)  // 10秒超时
            {
                sysState.isSettingMode = 0;  // 退出设置模式
                sysState.currentPage = PAGE_MAIN;  // 切换回主页面
                display_needs_refresh = 1;  // 标记显示需要刷新
            }
        }

        // 显示刷新时间控制（每100ms刷新一次）
        if((GetSystemTime() - last_display_refresh) >= 100)
        {
            last_display_refresh = GetSystemTime();  // 更新上次显示刷新时间
            display_needs_refresh = 1;  // 标记显示需要刷新
        }

        // 实际的显示刷新逻辑
        if(display_needs_refresh)
        {
            display_needs_refresh = 0;  // 清除刷新标记

            if(sysState.isSettingMode)  // 在设置模式时
                ShowSettingPage(sysState.currentPage, sysState.selectedSetting, &settings);  // 显示设置页面
            else  // 非设置模式时
            {
                switch(sysState.currentPage)  // 根据当前页面状态
                {
                    case PAGE_MAIN:  // 主页面
                        ShowMainPage(temp, humi, light, soil_moisture, &sysState);  // 显示主页面，包含传感器数据
                        break;
                    case PAGE_DEVICE_STATUS:  // 设备状态页面
                        ShowDeviceStatusPage();  // 显示设备状态页面
                        break;
                    case PAGE_MODE_SET:  // 模式设置页面
                        ShowModePage(&sysState);  // 显示模式设置页面
                        break;
                    default:  // 默认情况
                        sysState.currentPage = PAGE_MAIN;  // 切换回主页面
                        ShowMainPage(temp, humi, light, soil_moisture, &sysState);  // 显示主页面
                        break;
                }
            }
        }

        Delay_ms(10);
    }
}
