#include "stm32f10x.h"
#include "DHT11.h"
#include "OLED.h"
#include "Delay.h"
#include "JW01.h"
#include "GY302.h"
#include "MyI2C.h"
#include "timer.h"
#include "Key.h"
#include "Buzz.h"    
#include "Actuator.h"
#include "WiFi_ESP8266.h"
#include "OneNet_MQTT.h"
#include "USART3.h"
#include <stdio.h>
#include <string.h>

// ==================== 模式与参数定义 ====================
// 说明：本工程主界面/设置界面通过 `mode` 切换。
// - MODE_MONITOR：监测显示模式（正常运行）
// - MODE_SET_LOW ：设置“下限”阈值
// - MODE_SET_HIGH：设置“上限”阈值
// 阈值用于：自动控制（风扇/补光/加湿）以及越限蜂鸣器报警。
#define MODE_MONITOR    0
#define MODE_SET_LOW    1
#define MODE_SET_HIGH   2

// 参数索引：用于访问 `low_limit[]/high_limit[]` 以及设置界面选择项
// PARAM_TEMP  : 温度
// PARAM_HUMI  : 湿度
// PARAM_LIGHT : 光照
// PARAM_CO2   : CO2 浓度
#define PARAM_TEMP      0
#define PARAM_HUMI      1
#define PARAM_LIGHT     2
#define PARAM_CO2       3

#define CHN_SHE     7   // 设
#define CHN_ZHI     8   // 置
#define CHN_SHANG   0   // 上
#define CHN_XIA     1   // 下
#define CHN_XIAN    4   // 限
#define CHN_WEN     9   // 温
#define CHN_DU      3   // 度
#define CHN_HUMI    10  // 湿
#define CHN_GUANG   13  // 光
#define CHN_ZHAO    14  // 照
#define CHN_XIAN_   12  // 现

uint8_t mode = MODE_MONITOR;
uint8_t param = PARAM_TEMP;

// 阈值数组：下限/上限（与 PARAM_* 一一对应）
// 注意：设置界面修改的就是这两组阈值。
int16_t low_limit[4]  = {15, 40, 20, 340};
int16_t high_limit[4] = {30, 70, 800, 1000};

// 传感器当前值（在 100ms 周期任务中更新）
uint8_t temperature = 0;
uint8_t humidity = 0;
float light_intensity = 0;
uint16_t co2_value = 0;

// 远程控制接管标志：由网络下发控制时置 1
// remote_override 为 1 时：本地自动控制不再生效，避免相互覆盖。
volatile uint8_t remote_override = 0;
volatile uint8_t remote_fan_on = 0;
volatile uint8_t remote_light_on = 0;
volatile uint8_t remote_hum_on = 0;
// 远程接管超时计数（以 100ms 为单位递减）；到 0 自动退出远程接管
volatile uint8_t remote_override_tmo_100ms = 0;

// 用于光照显示“变化才刷新”，避免 OLED 频繁重绘导致闪烁
float last_light = -1.0;
// 蜂鸣器控制标志：只要任一参数越限，则置 1
uint8_t buzzer_required = 0;

// 滞回参数：用于自动控制的“回差”，避免在阈值附近频繁开关
// 例如：温度达到上限才开风扇，降到(上限-回差)才关风扇。
#define TEMP_HYST  1
#define HUMI_HYST  1
#define LIGHT_HYST 50
#define CO2_HYST   20


// ==================== 主界面显示函数（保持原格式） ====================
void Show_Static_UI(void)
{
    OLED_Clear();
    // 主界面固定内容：标签只画一次，动态数据在后续函数中更新
    OLED_ShowChinese(1, 1, 9);   // 温
    OLED_ShowChinese(1, 3, 10);  // 湿
    OLED_ShowString(1, 5, ":");
    OLED_ShowChinese(2, 1, 13);  // 光
    OLED_ShowChinese(2, 3, 14);  // 照
    OLED_ShowString(2, 5, ":");
    OLED_ShowString(3, 1, "CO2:");
    OLED_ShowString(4, 1, "K1:Set");
}

void Show_Temp_Humidity(uint8_t temp, uint8_t humi, uint8_t ret)
{
    if (ret == 0)
    {
        // ret==0 表示 DHT11 读取成功：显示数值，并在后面标注越限报警字母
        OLED_ShowNum(1, 6, temp, 2);
        OLED_ShowString(1, 8, "C");
				char temp_alarm = ' ';
			 if (temp > high_limit[PARAM_TEMP]) temp_alarm = 'H';
        else if (temp < low_limit[PARAM_TEMP]) temp_alarm = 'L';
        OLED_ShowChar(1, 9, temp_alarm);  // 报警字母（或空格）
        OLED_ShowChar(1, 10, ' ');        // 保留一个空格间隔
			
			
        OLED_ShowNum(1, 11, humi, 2);
        OLED_ShowString(1, 13, "%");
				char humi_alarm = ' ';
        if (humi > high_limit[PARAM_HUMI]) humi_alarm = 'H';
        else if (humi < low_limit[PARAM_HUMI]) humi_alarm = 'L';
        OLED_ShowChar(1, 14, humi_alarm); // 在%后面显示报警

    }
    else
    {
        // ret!=0 表示 DHT11 读取失败：用 -- 占位，同时清掉可能残留的报警字母
        OLED_ShowString(1, 6, "--C  --%");
			// 清除可能残留的报警字母
        OLED_ShowChar(1, 9, ' ');
        OLED_ShowChar(1, 10, ' ');
        OLED_ShowChar(1, 14, ' ');

    }
}

void Show_Light(float light)
{
    // OLED 刷新优化：光照值变化到“整数”层面才重绘字符串，避免闪烁
    if ((int)light != (int)last_light)
    {
        OLED_ShowString(2, 6, "        ");
        if (light <= 0.1f)
            OLED_ShowString(2, 6, "Err    ");
        else if (light < 10000.0f)
        {
            char buf[8];
            sprintf(buf, "%04dlx", (int)light);
            OLED_ShowString(2, 6, buf);
        }
        else
            OLED_ShowString(2, 6, "High   ");
        last_light = light;
    }
		 if (light > 0.1f)  // 数值有效时才报警
    {
        // 光照越限报警：H 表示超上限，L 表示低于下限
        int light_int = (int)light;
        char light_alarm = ' ';
        if (light_int > high_limit[PARAM_LIGHT]) light_alarm = 'H';
        else if (light_int < low_limit[PARAM_LIGHT]) light_alarm = 'L';
        OLED_ShowChar(2, 14, light_alarm);
			
		
    }
    else
    {
        OLED_ShowChar(2, 14, ' ');  // 无效数据时清除报警
    }

}

void Show_CO2(uint16_t co2)
{
    static uint16_t last_co2 = 0;
    if (co2 != last_co2)
    {
        // CO2 数值变化时重绘显示区域
        OLED_ShowString(3, 5, "         ");
        char buf[10];
        sprintf(buf, "%04dppm", co2);
        OLED_ShowString(3, 5, buf);
        last_co2 = co2;
    }
		
		if (co2 > 0)
    {
        // CO2 越限报警显示
        char co2_alarm = ' ';
        if (co2 > high_limit[PARAM_CO2]) co2_alarm = 'H';
        else if (co2 < low_limit[PARAM_CO2]) co2_alarm = 'L';
        OLED_ShowChar(3, 14, co2_alarm);
    }
    else
    {
        OLED_ShowChar(3, 14, ' ');
    }

}

// ==================== 设置界面函数（整行重绘，无闪烁） ====================
void Init_Setting_UI(void)
{
    OLED_Clear();
    // 设置界面首次进入时初始化固定布局；后续更新由 Show_Setting_UI() 完成
    // 第一行：模式
	  OLED_ShowChinese(1, 5, CHN_SHE);   // 设
		OLED_ShowChinese(1, 7, CHN_ZHI);   // 置
    OLED_ShowChinese(1, 9, CHN_XIA);   // 下
    OLED_ShowChinese(1, 11, CHN_XIAN);  // 限    
	   // 第二行：参数名“温度”
    OLED_ShowChinese(2, 1, CHN_WEN);   // 温
    OLED_ShowChinese(2, 3, CHN_DU);    // 度
    // 第三行：数值 + 单位
    OLED_ShowNum(3, 1, low_limit[PARAM_TEMP], 4);
    OLED_ShowString(3, 6, "C");
    // 第四行：完整提示（使用当前温度值，若未读取到则显示0）
    char buf[20];
    sprintf(buf, ":%2dC", temperature);
	    OLED_ShowChinese(4, 1, CHN_XIAN_);   // 现
    OLED_ShowString(4, 3, buf);
}

void Show_Setting_UI(void)
{
    // 设置界面刷新策略：
    // - 用 last_* 记录上次显示状态，仅在变化时更新
    // - 第二/四行采用“整行重绘”避免残影
    static uint8_t last_mode = 0;
    static uint8_t last_param = 0;
    static int16_t last_val = -1;
    static int last_temp = -1, last_humi = -1, last_light_int = -1, last_co2 = -1;

    int16_t val = (mode == MODE_SET_LOW) ? low_limit[param] : high_limit[param];
    const char *units[] = {"C", "%", "lx", "ppm"};

    // 更新第一行（模式）
    if (mode != last_mode)
    {
			OLED_ShowString(1, 1, "                ");  // 清除整行
			OLED_ShowChinese(1, 5, CHN_SHE);
			OLED_ShowChinese(1, 7, CHN_ZHI);
			    if (mode == MODE_SET_LOW)
        {
            OLED_ShowChinese(1, 9, CHN_XIA);
        }
        else
        {
            OLED_ShowChinese(1, 9, CHN_SHANG);
        }
        OLED_ShowChinese(1, 11, CHN_XIAN);
    }

    // 更新第二行（参数名）—— 整行重绘
    if (param != last_param)
    {
        // 清除整行第二行（16个空格）
        OLED_ShowString(2, 1, "                ");
				// 根据当前 param 显示不同参数名称
				switch (param)
					{
							case PARAM_TEMP:
										OLED_ShowChinese(2, 1, CHN_WEN);
										OLED_ShowChinese(2, 3, CHN_DU);
										break;
								case PARAM_HUMI:
										OLED_ShowChinese(2, 1, CHN_HUMI);
										OLED_ShowChinese(2, 3, CHN_DU);
										break;
								case PARAM_LIGHT:
										OLED_ShowChinese(2, 1, CHN_GUANG);
										OLED_ShowChinese(2, 3, CHN_ZHAO);
										break;
								case PARAM_CO2:
										OLED_ShowString(2, 1, "CO2");
										break;
						}
        // 更新单位
        OLED_ShowString(3, 6, "   ");
        OLED_ShowString(3, 6, units[param]);
    }

    // 更新第三行（阈值数值）
    if (val != last_val || param != last_param || mode != last_mode)
    {
        // 第三行显示：当前正在设置的阈值（下限/上限）
        OLED_ShowString(3, 1, "    ");
        OLED_ShowNum(3, 1, val, 4);
			  OLED_ShowString(3, 6, "   ");       // 清除单位区域
        OLED_ShowString(3, 6, units[param]); // 显示新单位
        last_val = val;
    }

    // 获取当前传感器值
    int current = 0;
    switch (param)
    {
        case PARAM_TEMP:  current = temperature; break;
        case PARAM_HUMI:  current = humidity; break;
        case PARAM_LIGHT: current = (int)light_intensity; break;
        case PARAM_CO2:   current = co2_value; break;
    }

    uint8_t need_update = 0;
    if (param == PARAM_TEMP && current != last_temp) { last_temp = current; need_update = 1; }
    else if (param == PARAM_HUMI && current != last_humi) { last_humi = current; need_update = 1; }
    else if (param == PARAM_LIGHT && current != last_light_int) { last_light_int = current; need_update = 1; }
    else if (param == PARAM_CO2 && current != last_co2) { last_co2 = current; need_update = 1; }

    // 更新第四行（当前值 + 按键提示）—— 整行重绘
    if (need_update || param != last_param)
    {
        char buf[20];
        // 清除整行第四行
        OLED_ShowString(4, 1, "                ");
			  OLED_ShowChinese(4, 1, CHN_XIAN_);           // 现
        // 根据参数格式化
        switch (param)
        {
            case PARAM_TEMP:
                sprintf(buf, ":%2dC", current);
                break;
            case PARAM_HUMI:
                sprintf(buf, ":%2d%%", current);
                break;
            case PARAM_LIGHT:
                sprintf(buf, ":%4dlx", current);
                break;
            case PARAM_CO2:
                sprintf(buf, ":%4dppm", current);
                break;
        }
				
				
        OLED_ShowString(4, 3, buf);
    }

    last_mode = mode;
    last_param = param;
}

// ==================== 按键处理函数 ====================
static void handle_key_mode(void)
{
    // K1：模式键
    // MONITOR -> SET_LOW -> SET_HIGH -> MONITOR
    // 返回 MONITOR 时会退出远程接管，并恢复主界面显示。
    if (mode == MODE_MONITOR)
    {
        mode = MODE_SET_LOW;
        OLED_Clear();
        Init_Setting_UI();
    }
    else if (mode == MODE_SET_LOW)
    {
        mode = MODE_SET_HIGH;
    }
    else
    {
        mode = MODE_MONITOR;
        remote_override = 0;
        OLED_Clear();
        Show_Static_UI();           // 返回主界面
    }
}

void handle_key_sel(void)
{
    // K2：在设置模式下切换要设置的参数（温/湿/光/CO2）
    if (mode != MODE_MONITOR)
        param = (param + 1) % 4;
}

void handle_key_up(void)
{
    // K3：在设置模式下增加阈值（不同参数步进不同）
    if (mode != MODE_MONITOR)
    {
        int16_t *p = (mode == MODE_SET_LOW) ? &low_limit[param] : &high_limit[param];
        switch (param)
        {
            case PARAM_TEMP:  (*p)++; if (*p > 50) *p = 50; break;
            case PARAM_HUMI:  (*p)++; if (*p > 100) *p = 100; break;
            case PARAM_LIGHT: (*p) += 100; if (*p > 20000) *p = 20000; break;
            case PARAM_CO2:   (*p) += 10; if (*p > 5000) *p = 5000; break;
        }
        if (mode == MODE_SET_HIGH && *p < low_limit[param])
            *p = low_limit[param];
    }
}

void handle_key_down(void)
{
    // K4：在设置模式下减少阈值（不同参数步进不同）
    if (mode != MODE_MONITOR)
    {
        int16_t *p = (mode == MODE_SET_LOW) ? &low_limit[param] : &high_limit[param];
        switch (param)
        {
            case PARAM_TEMP:  (*p)--; if (*p < 0) *p = 0; break;
            case PARAM_HUMI:  (*p)--; if (*p < 0) *p = 0; break;
            case PARAM_LIGHT: (*p) -= 100; if (*p < 0) *p = 0; break;
            case PARAM_CO2:   (*p) -= 10; if (*p < 300) *p = 300; break;
        }
        if (mode == MODE_SET_LOW && *p > high_limit[param])
            *p = high_limit[param];
    }
}

// ==================== 主程序 ====================
int main(void)
{
    // 外设/模块初始化顺序：
    // - 显示与传感器（OLED/DHT11/I2C 光照/CO2）
    // - 定时器 TIM3（产生 100ms/1s 软定时标志）
    // - 按键与蜂鸣器
    // - 调试串口 USART3
    // - 执行器（风扇/补光/加湿等）
    // - WiFi 与 OneNet(MQTT)
    OLED_Init();
    DHT11_Init();
    MyI2C_Init();
    GY302_Init();
    JW01_Init();
    TIM3_Init();
    Key_Init();
	  Buzz_Init();

    USART3_Init(115200);

    Actuator_Init();

    WiFi_ESP8266_Init(115200);

    OneNet_MQTT_Init();

    Show_Static_UI();

    while (1)
    {
        // 1) 按键扫描：无论是否到 100ms 周期，都允许响应按键
        uint8_t key = Key_Scan();
        if (key == 1) handle_key_mode();
        else if (key == 2) handle_key_sel();
        else if (key == 3) handle_key_up();
        else if (key == 4) handle_key_down();

        if (flag_100ms)
        {
            // 2) 100ms 周期任务：由 TIM3 中断置位 flag_100ms
            // 该周期内完成：
            // - 远程接管超时计时
            // - 采集传感器数据
            // - 越限报警判断与蜂鸣器控制
            // - 自动控制/远程控制输出
            // - UI 刷新
            // - OneNet MQTT 协议栈轮询与周期上报
            flag_100ms = 0;

            if (remote_override_tmo_100ms)
            {
                // 远程接管倒计时：若长时间没有收到远程控制指令，自动回到本地自动控制
                remote_override_tmo_100ms--;
                if (remote_override_tmo_100ms == 0)
                {
                    remote_override = 0;
                }
            }

            // 采集传感器
            // ret==0 表示温湿度有效；光照/CO2 通过各自规则判断有效性
            uint8_t ret = DHT11_Read_Data(&temperature, &humidity);
            light_intensity = GY302_ReadLight();
            JW01_ProcessData();
            if (JW01_NewDataAvailable())
                co2_value = JW01_GetCO2_PPM();
						    buzzer_required = 0;
						
						
						// 越限报警检查：任意一个参数越界则蜂鸣器鸣叫
						// 温湿度有效时检查
            if (ret == 0)
            {
                if (temperature > high_limit[PARAM_TEMP] || temperature < low_limit[PARAM_TEMP])
                    buzzer_required = 1;
                if (humidity > high_limit[PARAM_HUMI] || humidity < low_limit[PARAM_HUMI])
                    buzzer_required = 1;
            }

            // 光照有效时检查（大于0.1视为有效）
            if (light_intensity > 0.1f)
            {
                int light_int = (int)light_intensity;
                if (light_int > high_limit[PARAM_LIGHT] || light_int < low_limit[PARAM_LIGHT])
                    buzzer_required = 1;
            }

            // CO2有效时检查（大于0视为有效）
            if (co2_value > 0)
            {
                if (co2_value > high_limit[PARAM_CO2] || co2_value < low_limit[PARAM_CO2])
                    buzzer_required = 1;
            }

            {
                // 控制输出：
                // - 远程接管时（remote_override=1）：直接按照远程下发状态输出
                // - 否则：根据阈值进行自动控制，并加入滞回避免抖动
                static uint8_t fan_on = 0;
                static uint8_t light_on = 0;
                static uint8_t hum_on = 0;
                static uint8_t fan_by_temp = 0;
                static uint8_t fan_by_humi = 0;
                static uint8_t fan_by_co2 = 0;

                if (remote_override)
                {
                    // 远程优先：直接设置执行器输出
                    fan_on = remote_fan_on;
                    light_on = remote_light_on;
                    hum_on = remote_hum_on;

                    Actuator_SetFan(fan_on);
                    Actuator_SetLight(light_on);
                    Actuator_SetHumidifier(hum_on);

                    {
                        char dbg[64];
                        sprintf(dbg, "[CTRL] remote fan=%u(PB14=%u) light=%u hum=%u\r\n",
                                (unsigned)fan_on,
                                (unsigned)(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_14) ? 1 : 0),
                                (unsigned)light_on, (unsigned)hum_on);
                        USART3_Send_Data((uint8_t*)dbg, (uint8_t)strlen(dbg));
                    }

                    // 远程控制优先：不再执行下面的自动控制段，避免被覆盖
                }
                else
                {
                    // 本地自动控制：根据当前值与阈值进行决策

                if (ret == 0)
                {
                    // 温度/湿度决定风扇与加湿
                    if (!fan_by_temp && (temperature >= (uint8_t)high_limit[PARAM_TEMP]))
                        fan_by_temp = 1;
                    else if (fan_by_temp && (temperature <= (uint8_t)(high_limit[PARAM_TEMP] - TEMP_HYST)))
                        fan_by_temp = 0;

                    if (!fan_by_humi && (humidity >= (uint8_t)high_limit[PARAM_HUMI]))
                        fan_by_humi = 1;
                    else if (fan_by_humi && (humidity <= (uint8_t)(high_limit[PARAM_HUMI] - HUMI_HYST)))
                        fan_by_humi = 0;

                    if (!hum_on && (humidity <= (uint8_t)low_limit[PARAM_HUMI]))
                        hum_on = 1;
                    else if (hum_on && (humidity >= (uint8_t)(low_limit[PARAM_HUMI] + HUMI_HYST)))
                        hum_on = 0;
                }
                else
                {
                    // 温湿度无效时：出于安全考虑关闭加湿（避免误动作）
                    hum_on = 0;
                }

                if (light_intensity > 0.1f)
                {
                    // 光照决定补光灯：低于下限开灯，高于(下限+滞回)关灯
                    int light_int = (int)light_intensity;
                    if (!light_on && (light_int <= low_limit[PARAM_LIGHT]))
                        light_on = 1;
                    else if (light_on && (light_int >= (low_limit[PARAM_LIGHT] + LIGHT_HYST)))
                        light_on = 0;
                }
                else
                {
                    // 光照无效时默认关灯
                    light_on = 0;
                }

                if (co2_value > 0)
                {
                    // CO2 决定风扇：高于上限开风扇，低于(上限-滞回)关风扇
                    if (!fan_by_co2 && (co2_value >= (uint16_t)high_limit[PARAM_CO2]))
                        fan_by_co2 = 1;
                    else if (fan_by_co2 && (co2_value <= (uint16_t)(high_limit[PARAM_CO2] - CO2_HYST)))
                        fan_by_co2 = 0;
                }

                fan_on = (fan_by_temp || fan_by_humi || fan_by_co2) ? 1 : 0;

                Actuator_SetFan(fan_on);
                Actuator_SetLight(light_on);
                Actuator_SetHumidifier(hum_on);

                {
                    char dbg[64];
                    sprintf(dbg, "[CTRL] auto fan=%u(PB14=%u) light=%u hum=%u\r\n",
                            (unsigned)fan_on,
                            (unsigned)(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_14) ? 1 : 0),
                            (unsigned)light_on, (unsigned)hum_on);
                    USART3_Send_Data((uint8_t*)dbg, (uint8_t)strlen(dbg));
                }

                }
            }

            // 控制蜂鸣器（低电平触发）
            if (buzzer_required)
                Buzz_ON();   // 低电平触发
            else
                Buzz_Off();  // 高电平关闭


            if (mode == MODE_MONITOR)
            {
                // 监测界面：显示实时数据
                Show_Temp_Humidity(temperature, humidity, ret);
                Show_Light(light_intensity);
                Show_CO2(co2_value);
                OLED_ShowString(4, 1, "K1:Set");
            }
            else
            {
                // 设置界面：仅做设置 UI 刷新
                Show_Setting_UI();  // 整行重绘，确保显示正确
            }

            // OneNet MQTT 状态机轮询：处理收发、心跳、订阅回调等
            OneNet_MQTT_Tick100ms();

            static uint8_t report_div = 0;
            report_div++;
            if (report_div >= 20)
            {
                // 2 秒周期上报一次（20 * 100ms）
                report_div = 0;
                OneNet_MQTT_ReportOnce(temperature, humidity, (int)light_intensity, co2_value,
                                       Actuator_GetFan(), Actuator_GetLight(), Actuator_GetHumidifier());
            }
        }
        // 主循环最小延时：配合按键扫描与其它轮询，降低 CPU 占用
        Delay_ms(10);
    }
}
