/**
 ******************************************************************************
 * @file    app_tasks.c
 * @brief   应用任务管理实现文件
 *          包含所有FreeRTOS任务的实现和调度
 * @note    将RTOS任务调度与UI界面代码分离，便于维护
 ******************************************************************************
 */

#include "app_tasks.h"                 // 应用任务头文件（任务原型与对外接口）
#include "led.h"                       // LED 控制接口
#include "beep.h"                      // 蜂鸣器控制接口
#include "key.h"                       // 按键扫描接口
#include "FreeRTOS.h"                  // FreeRTOS 核心头文件
#include "task.h"                      // FreeRTOS 任务管理
#include "queue.h"                     // FreeRTOS 消息队列
#include "event_groups.h"              // FreeRTOS 事件组
#include "semphr.h"                    // FreeRTOS 信号量（含互斥量）
#include "delay.h"                     // 延时工具（裸机/RTOS兼容）
#include "lvgl.h"                      // LVGL 主头文件
#include "lv_port_disp_template.h"     // LVGL 显示端口移植
#include "lv_port_indev_template.h"    // LVGL 输入设备移植
#include "gui_guider.h"                // UI 生成器自动生成的界面对象定义
#include "events_init.h"               // UI 事件绑定初始化
#include "custom.h"                    // 自定义 UI 初始化
#include "radar.h"                     // 雷达模块接口
#include "gps.h"                       // GPS 模块接口
#include "esp32_uart.h"                // ESP32 串口通信接口
#include "dht11.h"                     // DHT11 温湿度模块接口
#include "edge_alarm.h"                // 边缘告警算法
#include "offline_buf.h"               // 离线事件缓冲
#include "iwdg.h"                      // 独立看门狗接口
#include "usart3.h"                    // USART3(雷达)串口接口
#include "uart4.h"                     // UART4(GPS)串口接口
#include <stdio.h>                      // 标准IO
#include <string.h>                     // 字符串处理

/* 全局：SAR 模式标志，供 UI 读取（0：普通模式，1：搜救模式） */
volatile uint8_t g_sar_mode = 0;

/* 全局：当前UI页面索引（0=Main，1=SAR，2=History） */
volatile uint8_t g_ui_page_index = 0;

/* 调试开关：串口原始数据打印（0=关闭，1=开启） */
volatile uint8_t g_uart_debug = 0;  // 关闭UART RAW DEBUG（每2秒打印一次的那个）

/**
 * @brief  打印各串口接收到的原始数据（调试用）
 * @note   仅在g_uart_debug=1时打印，用于排查串口接收问题
 */
static void debug_print_uart_raw(void)
{
    static uint32_t last_print = 0;
    uint32_t now = HAL_GetTick();
    
    /* 每2秒打印一次 */
    if((now - last_print) < 2000) return;
    last_print = now;
    
    printf("\r\n========== UART RAW DEBUG ==========\r\n");
    
    /* USART3 - 雷达 */
    printf("[USART3/Radar] RX_STA=%u, Data(hex): ", USART3_RX_STA);
    if(USART3_RX_STA > 0) {
        uint16_t len = (USART3_RX_STA > 32) ? 32 : USART3_RX_STA;
        for(uint16_t i = 0; i < len; i++) {
            printf("%02X ", USART3_RX_BUF[i]);
        }
        if(USART3_RX_STA > 32) printf("...");
    } else {
        printf("(empty)");
    }
    printf("\r\n");
    
    /* UART4 - GPS */
    printf("[UART4/GPS] RX_STA=%u, Data: ", UART4_RX_STA);
    if(UART4_RX_STA > 0) {
        uint16_t len = (UART4_RX_STA > 64) ? 64 : UART4_RX_STA;
        for(uint16_t i = 0; i < len; i++) {
            char c = UART4_RX_BUF[i];
            if(c >= 0x20 && c < 0x7F) printf("%c", c);
            else printf(".");
        }
        if(UART4_RX_STA > 64) printf("...");
    } else {
        printf("(empty)");
    }
    printf("\r\n");
    
    /* USART6 - ESP32 */
    printf("[USART6/ESP32] RX_STA=%u, Data: ", ESP32_UART_RX_STA);
    if(ESP32_UART_RX_STA > 0) {
        uint16_t len = (ESP32_UART_RX_STA > 64) ? 64 : ESP32_UART_RX_STA;
        for(uint16_t i = 0; i < len; i++) {
            char c = ESP32_UART_RX_BUF[i];
            if(c >= 0x20 && c < 0x7F) printf("%c", c);
            else printf(".");
        }
        if(ESP32_UART_RX_STA > 64) printf("...");
    } else {
        printf("(empty)");
    }
    printf("\r\n");
    printf("=====================================\r\n");
}

/* 新UI：全局界面对象与UI覆盖控制标志 */
lv_ui guider_ui;                        // 界面对象容器（由 UI 生成器定义）
volatile int g_ui_led1_override = -1;   // LED1 覆盖：-1无覆盖/0关/1开
volatile int g_ui_buzzer_override = -1; // 蜂鸣器覆盖：-1无覆盖/0关/1开

/* UI消息队列与事件组（仅用于UI数据通道与模块状态） */
typedef enum {
    UI_MSG_NET_TIME = 1,   // 网络时间
    UI_MSG_NET_STATUS = 2, // 网络在线状态
    UI_MSG_RADAR = 3,      // 雷达快照
    UI_MSG_GPS = 4,        // GPS 快照
    UI_MSG_DHT = 5,        // 温湿度快照
} ui_msg_type_t;

typedef struct {
    ui_msg_type_t type; // 消息类型
    union {
        struct { char time[32]; } net_time; // 网络时间字符串
        struct { uint8_t online; } net_status; // 在线=1/离线=0
        struct { int8_t exist; int16_t distance_cm; int16_t heart_rate; int16_t respiration; int16_t body_motion; } radar; // 雷达参数
        struct { int32_t lat_e6; int32_t lon_e6; uint8_t fixmode; uint8_t svnum; int32_t alt_dm; } gps; // GPS 参数（定点）
        struct { uint8_t valid; float t_c; float rh; } dht; // 温湿度参数
    } u;
} ui_msg_t;

static QueueHandle_t g_ui_queue = NULL;     // UI消息队列，用于跨任务传递UI数据
static EventGroupHandle_t g_sys_evt = NULL; // 系统事件组，统一管理模块健康位
#define EVT_NET_OK    (1u<<0) // 网络健康位
#define EVT_RADAR_OK  (1u<<1) // 雷达健康位
#define EVT_GPS_OK    (1u<<2) // GPS健康位
#define EVT_DHT_OK    (1u<<3) // 温湿度健康位

/* UI缓存：由UI任务消费消息后更新，供渲染使用 */
static char g_ui_time[32] = {0};          // 网络时间缓存

static struct { int8_t exist; int16_t distance_cm; int16_t heart_rate; int16_t respiration; int16_t body_motion; } g_ui_radar = { -1, -1, -1, -1, -1 }; // 雷达缓存
static struct { double lat; double lon; uint8_t fixmode; uint8_t svnum; double alt_m; uint8_t valid; } g_ui_gps = {0}; // GPS缓存
static struct { uint8_t valid; float t_c; float rh; } g_ui_dht = {0, 0, 0}; // 温湿度缓存

/* 任务心跳位（供看门狗任务检查） */
static volatile uint32_t g_task_heartbeat = 0; // 任务心跳汇总
#define HB_RADAR   (1u<<0) // 雷达任务心跳
#define HB_GPS     (1u<<1) // GPS任务心跳
#define HB_DHT     (1u<<2) // DHT任务心跳
#define HB_CTRL    (1u<<3) // 控制任务心跳
#define HB_UI      (1u<<4) // UI任务心跳
#define HB_LED     (1u<<5) // LED任务心跳

/* 模块在线状态（有数据时置1，用于看门狗动态调整检测） */
static volatile uint8_t g_radar_online = 0;  // 雷达模块在线
static volatile uint8_t g_gps_online = 0;    // GPS模块在线
static volatile uint8_t g_dht_online = 0;    // DHT模块在线

/******************************************************************************************************/
/*FreeRTOS任务配置*/

/* START_TASK 任务配置 */
#define START_TASK_PRIO     1     // start_task 任务优先级（数值越大优先级越高）
#define START_STK_SIZE      256   // start_task 任务栈大小
static TaskHandle_t StartTask_Handler; // start_task 任务句柄

/* LV_DEMO_TASK 任务（UI） */
#define LV_DEMO_TASK_PRIO   4     // UI任务优先级（保证及时渲染）
#define LV_DEMO_STK_SIZE    2048  // UI任务栈大小（增大以支持复杂界面）
static TaskHandle_t LV_DEMOTask_Handler; // UI任务句柄

/* RADAR 任务 */
#define RADAR_TASK_PRIO     3     // 雷达任务优先级
#define RADAR_STK_SIZE      768   // 雷达任务栈大小
static TaskHandle_t RadarTask_Handler; // 雷达任务句柄

/* GPS 任务 */
#define GPS_TASK_PRIO       2     // GPS任务优先级
#define GPS_STK_SIZE        768   // GPS任务栈大小
static TaskHandle_t GPSTask_Handler; // GPS任务句柄

/* DHT11 任务 */
#define DHT_TASK_PRIO       2     // DHT任务优先级
#define DHT_STK_SIZE        384   // DHT任务栈大小
static TaskHandle_t DHTTask_Handler; // DHT任务句柄

/* 控制任务：LED/BEEP/按键/ESP32上报 */
#define CTRL_TASK_PRIO      3     // 控制任务优先级
#define CTRL_STK_SIZE       1536  // 控制任务栈大小（ESP32_Send_SensorData需要大缓冲区）
static TaskHandle_t CtrlTask_Handler; // 控制任务句柄

/* 看门狗任务：检查任务心跳并喂狗 */
#define WD_TASK_PRIO        5     // 看门狗任务优先级（最高，确保定期喂狗）
#define WD_STK_SIZE         256   // 看门狗任务栈大小
static TaskHandle_t WDTask_Handler; // 看门狗任务句柄

/* LED_TASK 任务配置 */
#define LED_TASK_PRIO       4     // LED任务优先级
#define LED_STK_SIZE        128   // LED任务栈大小
static TaskHandle_t LEDTask_Handler; // LED任务句柄

/* NET 任务：定时查询网络时间与在线状态并通过UI队列下发 */
#define NET_TASK_PRIO       2     // 网络任务优先级
#define NET_STK_SIZE        768   // 网络任务栈大小
static TaskHandle_t NetTask_Handler;     // 网络任务句柄
/* 前向声明：Net任务 */
static void net_task(void *pvParameters); // 网络任务原型

/* ESP32 RX 信号量与解析任务 */
SemaphoreHandle_t g_esp32_rx_sem = NULL;  /* 由USART6中断释放 */
static TaskHandle_t ESP32RxTask_Handler;
static void esp32_rx_task(void *pvParameters);

/* 从ESP32解析到的蜂鸣器指令（由esp32_rx_task写入，control_task读取并清零标志） */
static volatile int g_rx_buzzer_cmd_flag = 0;
static volatile uint8_t g_rx_buzzer_state = 0;

/******************************************************************************************************/

/* 强度映射辅助函数：距离(cm)到[0,1]接近度 */
static float map_proximity_intensity(int16_t distance_cm) // 距离映射为接近度[0,1]
{
    if(distance_cm < 0) return 0.0f;       // 无效距离返回0
    const float maxd = 300.0f;             // 300cm 以上按最远处理
    float d = (float)distance_cm;          // 转浮点
    if(d > maxd) d = maxd;                 // 裁剪至最大距离
    float v = 1.0f - (d / maxd);           // 距离越近，值越大
    if(v < 0.0f) v = 0.0f;                 // 下限保护
    if(v > 1.0f) v = 1.0f;                 // 上限保护
    return v;                               // 返回接近度
}

/**
 * @brief  初始化并启动所有应用任务
 */
void App_Tasks_Init(void) // 应用任务统一初始化入口
{
    xTaskCreate((TaskFunction_t )start_task,     // 任务函数
                (const char*    )"start_task",   // 任务名（调试可见）
                (uint16_t       )START_STK_SIZE, // 任务栈大小
                (void*          )NULL,           // 参数
                (UBaseType_t    )START_TASK_PRIO,// 优先级
                (TaskHandle_t*  )&StartTask_Handler); // 句柄输出
    
    vTaskStartScheduler(); /* 开启任务调度（不返回） */
}

/**
 * @brief  START任务 - 创建其他所有任务
 */
void start_task(void *pvParameters)
{
    pvParameters = pvParameters;
    printf("[START] start_task begin\r\n");
    taskENTER_CRITICAL();

    /* 初始化LVGL */
    printf("[START] LVGL init...\r\n");
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    printf("[START] LVGL init OK\r\n");

    /* 创建UI消息队列与事件组（UI通道） */
    g_ui_queue = xQueueCreate(32, sizeof(ui_msg_t));
    g_sys_evt  = xEventGroupCreate();
    g_esp32_rx_sem = xSemaphoreCreateBinary();
    printf("[START] Queue/Event/Sem created: Q=%p E=%p S=%p\r\n", 
           (void*)g_ui_queue, (void*)g_sys_evt, (void*)g_esp32_rx_sem);

    /* 初始化边缘算法和离线缓冲 */
    edge_alarm_init();
    offline_init();
    printf("[START] Edge/Offline init OK\r\n");
    
    /* 硬件传感器初始化 */
    printf("[START] Sensor init...\r\n");
    radar_init();            /* 初始化雷达模块(USART3) */
    GPS_Init();              /* 初始化GPS模块(UART4) */
    DHT11_Init();            /* 初始化温湿度传感器(PG14) */
    ESP32_UART_Init(115200); /* 初始化ESP32串口通信(USART6) */
    printf("[START] All sensors init OK\r\n");

    /* 创建UI任务 */
    xTaskCreate((TaskFunction_t )lv_demo_task,
                (const char*    )"lv_demo_task",
                (uint16_t       )LV_DEMO_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LV_DEMO_TASK_PRIO,
                (TaskHandle_t*  )&LV_DEMOTask_Handler);

    /* LED闪烁任务 */
    xTaskCreate((TaskFunction_t )led_task,
                (const char*    )"led_task",
                (uint16_t       )LED_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )LED_TASK_PRIO,
                (TaskHandle_t*  )&LEDTask_Handler);

    /* Radar 任务 */
    xTaskCreate((TaskFunction_t )radar_task,
                (const char*    )"radar_task",
                (uint16_t       )RADAR_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )RADAR_TASK_PRIO,
                (TaskHandle_t*  )&RadarTask_Handler);

    /* GPS 任务 */
    xTaskCreate((TaskFunction_t )gps_task,
                (const char*    )"gps_task",
                (uint16_t       )GPS_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )GPS_TASK_PRIO,
                (TaskHandle_t*  )&GPSTask_Handler);

    /* DHT11 任务 */
    xTaskCreate((TaskFunction_t )dht_task,
                (const char*    )"dht_task",
                (uint16_t       )DHT_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )DHT_TASK_PRIO,
                (TaskHandle_t*  )&DHTTask_Handler);

    /* 控制任务：LED/BEEP/按键/ESP32数据上报 */
    xTaskCreate((TaskFunction_t )control_task,
                (const char*    )"control_task",
                (uint16_t       )CTRL_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )CTRL_TASK_PRIO,
                (TaskHandle_t*  )&CtrlTask_Handler);

    /* Net任务：查询网络时间与在线状态，下发到UI队列 */
    xTaskCreate((TaskFunction_t )net_task,
                (const char*    )"net_task",
                (uint16_t       )NET_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )NET_TASK_PRIO,
                (TaskHandle_t*  )&NetTask_Handler);

    /* ESP32 解析任务：等待串口中断信号，解析命令 */
    xTaskCreate((TaskFunction_t )esp32_rx_task,
                (const char*    )"esp32_rx_task",
                (uint16_t       )512,  // 解析任务栈
                (void*          )NULL,
                (UBaseType_t    )3,
                (TaskHandle_t*  )&ESP32RxTask_Handler);

    /* 暂时禁用看门狗以排查复位原因 */
    /* 启用看门狗：预分频128，重载值3125 -> 超时约10秒 */
    IWDG_Init(IWDG_PRESCALER_128, 3125);
    xTaskCreate((TaskFunction_t )watchdog_task,
                (const char*    )"watchdog_task",
                (uint16_t       )WD_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )WD_TASK_PRIO,
                (TaskHandle_t*  )&WDTask_Handler);
    printf("[WD] Watchdog ENABLED (timeout ~10s)\r\n");

    taskEXIT_CRITICAL();
    vTaskDelete(StartTask_Handler);
}

/**
 * @brief  LVGL演示任务 - UI刷新
 */
static void ui_update_once(void) // UI单次刷新：仅使用UI缓存进行渲染
{
    /* 使用UI缓存（由各任务通过UI队列维护） */

    /* MAIN 页面 */
    if(guider_ui.MAIN) { // 若主页面有效
        /* 时间：从缓存显示（由Net任务维护） */
        if(g_ui_time[0] != '\0') { // 有有效时间字符串
            lv_label_set_text(guider_ui.MAIN_shijian, g_ui_time); // 更新主页面时间标签
        }

        /* 存在 */
        lv_label_set_text(guider_ui.MAIN_cunzi, (g_ui_radar.exist==1)?"YES":"NO"); // 显示是否有人

        /* 呼吸/心率/体动/距离(cm) */
        if(g_ui_radar.respiration >= 0) { char buf[16]; snprintf(buf,sizeof(buf),"%d rpm", g_ui_radar.respiration); lv_label_set_text(guider_ui.MAIN_huxi, buf);} else lv_label_set_text(guider_ui.MAIN_huxi, "--"); // 呼吸
        if(g_ui_radar.heart_rate >= 0)  { char buf[16]; snprintf(buf,sizeof(buf),"%d bpm", g_ui_radar.heart_rate);  lv_label_set_text(guider_ui.MAIN_xinlv, buf);} else lv_label_set_text(guider_ui.MAIN_xinlv, "--"); // 心率
        if(g_ui_radar.body_motion >= 0) { char buf[16]; snprintf(buf,sizeof(buf),"%d", g_ui_radar.body_motion);      lv_label_set_text(guider_ui.MAIN_tidong, buf);} else lv_label_set_text(guider_ui.MAIN_tidong, "--"); // 体动
        if(g_ui_radar.distance_cm >= 0) { char buf[16]; snprintf(buf,sizeof(buf),"%d", g_ui_radar.distance_cm);      lv_label_set_text(guider_ui.MAIN_juli, buf);} else lv_label_set_text(guider_ui.MAIN_juli, "--"); // 距离

        /* 经度/纬度（十进制度） */
        if(g_ui_gps.valid) { // GPS有效
            double lat = g_ui_gps.lat; double lon = g_ui_gps.lon; // 读取缓存
            char bufw[32], bufj[32]; // 文本缓冲
            snprintf(bufw,sizeof(bufw),"%.6f%c", (lat>=0)?lat:-lat, (lat>=0)?'N':'S'); // 纬度字符串
            snprintf(bufj,sizeof(bufj),"%.6f%c", (lon>=0)?lon:-lon, (lon>=0)?'E':'W'); // 经度字符串
            lv_label_set_text(guider_ui.MAIN_weidu, bufw);  // 更新纬度标签
            lv_label_set_text(guider_ui.MAIN_jingdu, bufj); // 更新经度标签
        } else { // GPS无效
            lv_label_set_text(guider_ui.MAIN_weidu, "No Fix");  // 纬度未知
            lv_label_set_text(guider_ui.MAIN_jingdu, "No Fix"); // 经度未知
        }

        /* 温湿度 */
        if(g_ui_dht.valid) { // 温湿度有效
            char tbuf[16], hbuf[16]; // 文本缓冲
            snprintf(tbuf,sizeof(tbuf),"%.1f °C", g_ui_dht.t_c);  // 温度字符串
            snprintf(hbuf,sizeof(hbuf),"%.1f %%RH", g_ui_dht.rh); // 湿度字符串
            lv_label_set_text(guider_ui.MAIN_wendu, tbuf); // 更新温度
            lv_label_set_text(guider_ui.MAIN_shidu, hbuf); // 更新湿度
        } else { // 无效
            lv_label_set_text(guider_ui.MAIN_wendu, "--"); // 温度未知
            lv_label_set_text(guider_ui.MAIN_shidu, "--"); // 湿度未知
        }
    }

    /* SAR 页面 */
    if(guider_ui.SAR) { // 若SAR页面有效
        if(g_ui_radar.respiration >= 0) { char buf[16]; snprintf(buf,sizeof(buf),"%d rpm", g_ui_radar.respiration); lv_label_set_text(guider_ui.SAR_huxi, buf);} else lv_label_set_text(guider_ui.SAR_huxi, "--"); // 呼吸
        if(g_ui_radar.heart_rate >= 0)  { char buf[16]; snprintf(buf,sizeof(buf),"%d bpm", g_ui_radar.heart_rate);  lv_label_set_text(guider_ui.SAR_xinlv, buf);} else lv_label_set_text(guider_ui.SAR_xinlv, "--"); // 心率
        if(g_ui_radar.distance_cm >= 0) { char buf[16]; snprintf(buf,sizeof(buf),"%d", g_ui_radar.distance_cm);      lv_label_set_text(guider_ui.SAR_juli, buf);} else lv_label_set_text(guider_ui.SAR_juli, "--"); // 距离

        if(g_ui_dht.valid) { // 温湿度有效
            char tbuf[16], hbuf[16];
            snprintf(tbuf,sizeof(tbuf),"%.1f °C", g_ui_dht.t_c);   // 温度
            snprintf(hbuf,sizeof(hbuf),"%.1f %%RH", g_ui_dht.rh);  // 湿度
            lv_label_set_text(guider_ui.SAR_wendu, tbuf); // 更新温度
            lv_label_set_text(guider_ui.SAR_shidu, hbuf); // 更新湿度
        } else { // 无效
            lv_label_set_text(guider_ui.SAR_wendu, "--"); // 温度未知
            lv_label_set_text(guider_ui.SAR_shidu, "--"); // 湿度未知
        }
    }

    /* 故障诊断 页面 */
    if(guider_ui.Fault_Diagnosis) { // 若故障页有效
        /* 使用事件组统一显示模块健康状态 */
        EventBits_t bits = xEventGroupGetBits(g_sys_evt); // 读取事件位

        /* 雷达健康 */
        const char* radar_ok = (bits & EVT_RADAR_OK) ? "YES" : "NO"; // 事件位转文本
        lv_label_set_text(guider_ui.Fault_Diagnosis_leidazhuangtai, radar_ok); // 更新

        /* GPS状态：若健康则显示fix等级，否则No */
        if(bits & EVT_GPS_OK) { // GPS健康
            const char* fix = (g_ui_gps.fixmode==3)?"3D":((g_ui_gps.fixmode==2)?"2D":"2D/3D?"); // 模式文本
            lv_label_set_text(guider_ui.Fault_Diagnosis_GPSzhuangtai, fix); // 更新
        } else {
            lv_label_set_text(guider_ui.Fault_Diagnosis_GPSzhuangtai, "No"); // 不健康
        }

        /* 温湿度健康 */
        const char* dht_ok = (bits & EVT_DHT_OK) ? "YES" : "NO"; // 温湿度状态
        lv_label_set_text(guider_ui.Fault_Diagnosis_wenshiduzhuangtai, dht_ok); // 更新

        /* 网络在线健康 */
        const char* net_ok = (bits & EVT_NET_OK) ? "YES" : "NO"; // 网络状态
        lv_label_set_text(guider_ui.Fault_Diagnosis_wangluozhuangtai, net_ok); // 更新
    }
}

void lv_demo_task(void *pvParameters) // UI任务：消费队列+定时渲染+LVGL定时器
{
    pvParameters = pvParameters; // 消除未使用告警
    /* 初始化新UI */
    setup_ui(&guider_ui);    // 创建界面对象
    events_init(&guider_ui); // 绑定事件回调
    custom_init(&guider_ui); // 自定义初始化

    uint32_t last_ui_update = 0;    // 上次UI更新时间戳
    uint8_t last_page_index = 0xFF; /* 跟踪页面变化（若后续使用按键切页，可在此扩展） */

    while(1) // 永久循环
    {
        /* 按键切换页面：检测 g_ui_page_index 变化并执行界面切换 */
        if(last_page_index != g_ui_page_index)
        {
            lv_disp_t *d = lv_obj_get_disp(lv_scr_act());
            if(d->prev_scr == NULL && d->scr_to_load == NULL) // 确保没有正在进行的切换
            {
                {
                    lv_mem_monitor_t mon;
                    lv_mem_monitor(&mon);
                    printf("[UI] Switching to page %d, LVGL free mem: %d bytes\r\n", 
                           g_ui_page_index, mon.free_size);
                }
                /* 先删除旧页面释放内存 */
                if(g_ui_page_index != 0 && guider_ui.MAIN && guider_ui.MAIN_del == false) {
                    lv_obj_del(guider_ui.MAIN);
                    guider_ui.MAIN = NULL;
                    guider_ui.MAIN_del = true;
                }
                if(g_ui_page_index != 1 && guider_ui.SAR && guider_ui.SAR_del == false) {
                    lv_obj_del(guider_ui.SAR);
                    guider_ui.SAR = NULL;
                    guider_ui.SAR_del = true;
                }
                if(g_ui_page_index != 2 && guider_ui.Fault_Diagnosis && guider_ui.Fault_Diagnosis_del == false) {
                    lv_obj_del(guider_ui.Fault_Diagnosis);
                    guider_ui.Fault_Diagnosis = NULL;
                    guider_ui.Fault_Diagnosis_del = true;
                }
                
                switch(g_ui_page_index)
                {
                    case 0: // MAIN页面
                        if(guider_ui.MAIN_del == true)
                            setup_scr_MAIN(&guider_ui);
                        lv_scr_load(guider_ui.MAIN);
                        guider_ui.MAIN_del = false;
                        break;
                    case 1: // SAR页面
                        if(guider_ui.SAR_del == true)
                            setup_scr_SAR(&guider_ui);
                        lv_scr_load(guider_ui.SAR);
                        guider_ui.SAR_del = false;
                        break;
                    case 2: // Fault_Diagnosis页面
                        if(guider_ui.Fault_Diagnosis_del == true)
                            setup_scr_Fault_Diagnosis(&guider_ui);
                        lv_scr_load(guider_ui.Fault_Diagnosis);
                        guider_ui.Fault_Diagnosis_del = false;
                        break;
                    default:
                        break;
                }
                {
                    lv_mem_monitor_t mon2;
                    lv_mem_monitor(&mon2);
                    printf("[UI] Page switch done, LVGL free mem: %d bytes\r\n", mon2.free_size);
                }
            }
            last_page_index = g_ui_page_index; // 更新跟踪变量
        }

        /* 消费UI队列：合并最新的网络时间与在线状态 */
        {
            ui_msg_t msg;      // 消息临时变量
            int drain = 0;     // 本轮已处理条数
            while(drain < 16 && xQueueReceive(g_ui_queue, &msg, 0) == pdTRUE) // 最多处理16条，避免长阻塞
            {
                switch(msg.type) // 按类型更新缓存
                {
                    case UI_MSG_NET_TIME:
                        strncpy(g_ui_time, msg.u.net_time.time, sizeof(g_ui_time)-1); // 复制时间
                        g_ui_time[sizeof(g_ui_time)-1] = '\0'; // 结尾保护
                        break;
                    case UI_MSG_NET_STATUS:
                        break; // 网络健康已通过事件组体现
                    case UI_MSG_RADAR:
                        g_ui_radar.exist = msg.u.radar.exist; // 更新雷达缓存
                        g_ui_radar.distance_cm = msg.u.radar.distance_cm;
                        g_ui_radar.heart_rate = msg.u.radar.heart_rate;
                        g_ui_radar.respiration = msg.u.radar.respiration;
                        g_ui_radar.body_motion = msg.u.radar.body_motion;
                        break;
                    case UI_MSG_GPS:
                        g_ui_gps.lat = ((double)msg.u.gps.lat_e6) / 1e6; // 转成浮点
                        g_ui_gps.lon = ((double)msg.u.gps.lon_e6) / 1e6;
                        g_ui_gps.fixmode = msg.u.gps.fixmode;
                        g_ui_gps.svnum = msg.u.gps.svnum;
                        g_ui_gps.alt_m = ((double)msg.u.gps.alt_dm) / 10.0;
                        g_ui_gps.valid = (g_ui_gps.fixmode > 0) ? 1 : 0; // 有效性
                        break;
                    case UI_MSG_DHT:
                        g_ui_dht.valid = msg.u.dht.valid; // 温湿度有效
                        g_ui_dht.t_c = msg.u.dht.t_c;     // 温度
                        g_ui_dht.rh = msg.u.dht.rh;       // 湿度
                        break;
                    default:
                        break;
                }
                drain++; // 计数
            }
        }

        if(lv_tick_elaps(last_ui_update) >= 200) // 每200ms更新一次UI
        {
            ui_update_once();          // 执行UI更新
            last_ui_update = lv_tick_get(); // 刷新基准时间
        }
        /* 处理LVGL定时器与动画 */
        lv_timer_handler();  // LVGL定时器
        g_task_heartbeat |= HB_UI; // 心跳置位
        vTaskDelay(5);       // 主循环节拍
    }
}

/* Net任务：查询网络时间与在线状态，并将结果通过UI队列下发 */
static void net_task(void *pvParameters) // 网络任务：查询网络时间/在线状态，投递UI队列并置事件位
{
    pvParameters = pvParameters; // 消除未使用告警
    TickType_t last_time_q = 0;  // 上次时间查询时刻
    TickType_t last_stat_q = 0;  // 上次状态查询时刻
    int last_online = -1;        // 上次在线状态

    while(1) // 永久循环
    {
        TickType_t now = xTaskGetTickCount(); // 当前节拍

        /* 每1000ms查询一次网络时间，有效才发送 */
        if((now - last_time_q) >= pdMS_TO_TICKS(1000)) // 1秒周期
        {
            char timebuf[32] = {0}; // 时间缓冲
            if(ESP32_Query_Time(timebuf, sizeof(timebuf))) // 查询成功
            {
                ui_msg_t m; m.type = UI_MSG_NET_TIME; // 组装消息
                strncpy(m.u.net_time.time, timebuf, sizeof(m.u.net_time.time)-1); // 拷贝时间
                (void)xQueueSendToBack(g_ui_queue, &m, 0); // 投递到UI队列
            }
            last_time_q = now; // 更新时间戳
        }

        /* 每2000ms查询一次在线状态，变化才发送 */
        if((now - last_stat_q) >= pdMS_TO_TICKS(2000)) // 2秒周期
        {
            int online = ESP32_Is_Online() ? 1 : 0; // 当前在线
            if(online != last_online) // 变化才上报
            {
                ui_msg_t m; m.type = UI_MSG_NET_STATUS; // 组装消息
                m.u.net_status.online = (uint8_t)online; // 在线位
                (void)xQueueSendToBack(g_ui_queue, &m, 0); // 投递

                if(online) xEventGroupSetBits(g_sys_evt, EVT_NET_OK); // 置位网络健康
                else       xEventGroupClearBits(g_sys_evt, EVT_NET_OK); // 清位网络健康

                last_online = online; // 更新缓存
            }
            last_stat_q = now; // 更新时间戳
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms 任务节拍
    }
}

/* ESP32 串口解析任务：等待中断信号量或超时轮询，解析蜂鸣器命令和时间数据 */
static void esp32_rx_task(void *pvParameters)
{
    pvParameters = pvParameters; // 消除未使用告警
    while(1) // 永久循环
    {
        if(g_esp32_rx_sem) { // 有信号量
            (void)xSemaphoreTake(g_esp32_rx_sem, pdMS_TO_TICKS(100)); // 等待串口IRQ通知
        } else {
            vTaskDelay(pdMS_TO_TICKS(100)); // 无信号量则定时轮询
        }

        // 解析ESP32主动发送的数据（TIME:等）
        ESP32_Parse_RxData();

        u8 buzzer_cmd = 0; // 接收变量
        if(ESP32_Get_BuzzerCmd(&buzzer_cmd)) // 解析到蜂鸣器命令
        {
            g_rx_buzzer_state = (buzzer_cmd ? 1 : 0); // 状态缓存
            g_rx_buzzer_cmd_flag = 1;                 // 标志通知控制任务
        }
    }
}

/**
 * @brief  LED任务 - 简单闪烁
 */
void led_task(void *pvParameters)
{
    pvParameters = pvParameters;
    while(1)
    {
        vTaskDelay(1000);
        g_task_heartbeat |= HB_LED;
    }
}

/**
 * @brief  雷达任务 - 读取雷达数据
 */
void radar_task(void *pvParameters) // 雷达任务：采集+变化判定+投递UI // 雷达数据采集与去抖/变更判定
{
    pvParameters = pvParameters; // 消除未使用告警 // 避免未使用参数告警
    /* 上次快照用于去抖与变化检测 */ // 保存上一轮数据用于比较
    int8_t  prev_exist = -2; /* -2确保首次上报 */ // 用极端初值保证第一帧一定上报
    int16_t prev_dist = -32768, prev_hr = -32768, prev_resp = -32768, prev_motion = -32768; // 上次值 // 使用最小值表示“未初始化”
    while(1) // 永久循环 // 任务主循环
    {
        radar_process_data();              // 处理串口/算法，更新状态 // 驱动层/算法层刷新内部状态
        radar_state_t r = radar_get_state(); // 读取快照 // 以结构体形式获取当前快照

        /* 变化阈值：距离>5cm、hr/resp变化>=1、motion变化>=5 或 exist变化 */ // 去抖策略与阈值依据
        int change = 0; // 是否判定为“有变化”标志
        if(r.exist != prev_exist) change = 1; // 存在/不存在变化直接触发
        if(!change && r.distance_cm >= 0 && (prev_dist == -32768 || (r.distance_cm - prev_dist > 5) || (prev_dist - r.distance_cm > 5))) change = 1; // 距离阈值±5cm
        if(!change && r.heart_rate >= 0 && (prev_hr == -32768 || r.heart_rate != prev_hr)) change = 1; // 心率任意变化即触发（步进为1bpm）
        if(!change && r.respiration >= 0 && (prev_resp == -32768 || r.respiration != prev_resp)) change = 1; // 呼吸任意变化即触发（步进为1rpm）
        if(!change && r.body_motion >= 0 && (prev_motion == -32768 || (r.body_motion - prev_motion > 5) || (prev_motion - r.body_motion > 5))) change = 1; // 体动变化阈值±5

        if(change) // 有变化才投递 // 仅在变化时向UI通道发送，降低负载
        {
            g_radar_online = 1; // 标记雷达模块在线（有有效数据）
            xEventGroupSetBits(g_sys_evt, EVT_RADAR_OK); // 设置雷达健康事件位
            
            ui_msg_t m; m.type = UI_MSG_RADAR; // 组装雷达消息 // 通过UI消息队列传递快照
            m.u.radar.exist = (int8_t)r.exist; // 是否有人：-1未知/0无/1有
            m.u.radar.distance_cm = (int16_t)r.distance_cm; // 距离：单位cm
            m.u.radar.heart_rate = (int16_t)r.heart_rate; // 心率：bpm
            m.u.radar.respiration = (int16_t)r.respiration; // 呼吸：rpm
            m.u.radar.body_motion = (int16_t)r.body_motion; // 体动：百分比或相对量化值
            (void)xQueueSendToBack(g_ui_queue, &m, 0); // 投递 // 非阻塞发送至UI队列

            prev_exist = (int8_t)r.exist; // 更新上一轮缓存
            prev_dist = (int16_t)r.distance_cm; // 更新距离
            prev_hr = (int16_t)r.heart_rate; // 更新心率
            prev_resp = (int16_t)r.respiration; // 更新呼吸
            prev_motion = (int16_t)r.body_motion; // 更新体动
        }

        g_task_heartbeat |= HB_RADAR; // 心跳 // 设置雷达任务心跳位供看门狗检测
        vTaskDelay(20);               // 20ms 任务节拍 // 控制采样/处理频率
    }
}

/**
 * @brief  GPS任务 - 读取GPS数据
 */
void gps_task(void *pvParameters)
{
    pvParameters = pvParameters;
    int32_t prev_lat_e6 = -1, prev_lon_e6 = -1; // 用-1确保首次一定投递
    uint8_t prev_fix = 0xFF; uint8_t prev_sv = 0xFF; int32_t prev_alt_dm = -1;
    uint8_t first_post = 1; // 首次投递标志
    
    while(1)
    {
        GPS_Process_Data();
        nmea_msg* gps = GPS_Get_Data();
        if(gps)
        {
            double lat = gps->latitude / 100000.0; 
            if(gps->nshemi=='S'||gps->nshemi=='s') lat = -lat;
            double lon = gps->longitude / 100000.0; 
            if(gps->ewhemi=='W'||gps->ewhemi=='w') lon = -lon;
            int32_t lat_e6 = (int32_t)(lat * 1e6);
            int32_t lon_e6 = (int32_t)(lon * 1e6);
            int32_t alt_dm = (int32_t)(gps->altitude);
            uint8_t fix = (uint8_t)gps->gpssta;
            uint8_t sv = (uint8_t)gps->posslnum;

            int change = first_post; // 首次一定投递
            if(lat_e6 != prev_lat_e6 || lon_e6 != prev_lon_e6) change = 1;
            if(!change && (fix != prev_fix || sv != prev_sv)) change = 1;
            if(!change && alt_dm != prev_alt_dm) change = 1;

            if(change)
            {
                first_post = 0;
                g_gps_online = 1;
                // GPS有效定位时设置事件位
                if(fix > 0) {
                    xEventGroupSetBits(g_sys_evt, EVT_GPS_OK);
                } else {
                    xEventGroupClearBits(g_sys_evt, EVT_GPS_OK);
                }
                
                ui_msg_t m; 
                m.type = UI_MSG_GPS;
                m.u.gps.lat_e6 = lat_e6; 
                m.u.gps.lon_e6 = lon_e6; 
                m.u.gps.fixmode = fix; 
                m.u.gps.svnum = sv; 
                m.u.gps.alt_dm = alt_dm;
                (void)xQueueSendToBack(g_ui_queue, &m, 0);
                
                prev_lat_e6 = lat_e6; prev_lon_e6 = lon_e6; 
                prev_fix = fix; prev_sv = sv; prev_alt_dm = alt_dm;
            }
        }

        g_task_heartbeat |= HB_GPS;
        vTaskDelay(200);
    }
}

/**
 * @brief  DHT11任务 - 读取温湿度
 */
void dht_task(void *pvParameters) // DHT任务：采集+变化判定+投递UI // 温湿度采集、去抖与变化投递
{
    pvParameters = pvParameters; // 消除未使用告警 // 避免未使用参数告警
    uint8_t prev_valid = 0; float prev_t = 0.0f; float prev_rh = 0.0f; // 上次值 // 上一轮有效性与温湿度
    while(1) // 永久循环 // 任务主循环
    {
        DHT11_Process_Data();          // 采集/解析 // 驱动层采样并解析有效数据
        dht11_data_t* d = DHT11_Get_Data(); // 快照 // 获取当前温湿度缓存
        if(d)
        {
            uint8_t valid = d->valid ? 1 : 0; // 有效标志：1有效/0无效
            float t = 0.0f, rh = 0.0f; // 默认值（无效时为0）
            if(valid) // 仅在有效数据时组装
            {
                t = (float)d->temperature_int + (float)d->temperature_dec/10.0f; // 温度=整数位+小数一位/10
                rh = (float)d->humidity_int + (float)d->humidity_dec/10.0f; // 湿度=整数位+小数一位/10
            }

            int change = 0; // 变化标志
            if(valid != prev_valid) change = 1; // 有效性变化直接触发
            if(valid && !change)
            {
                if((t - prev_t > 0.2f) || (prev_t - t > 0.2f) || (rh - prev_rh > 0.5f) || (prev_rh - rh > 0.5f)) change = 1; // 温度>0.2°C或湿度>0.5%变化触发
            }

            if(change) // 有变化才投递 // 仅在变化时向UI队列下发，避免频繁刷新
            {
                if(valid) {
                    g_dht_online = 1; // 标记DHT模块在线（有有效数据）
                    xEventGroupSetBits(g_sys_evt, EVT_DHT_OK); // 设置DHT健康事件位
                } else {
                    xEventGroupClearBits(g_sys_evt, EVT_DHT_OK); // 清除DHT健康事件位
                }
                
                ui_msg_t m; m.type = UI_MSG_DHT; m.u.dht.valid = valid; m.u.dht.t_c = t; m.u.dht.rh = rh; // 组装DHT消息 // t单位°C，rh单位%RH
                (void)xQueueSendToBack(g_ui_queue, &m, 0); // 投递 // 非阻塞发送
                prev_valid = valid; prev_t = t; prev_rh = rh; // 更新上一轮缓存
            }
        }

        g_task_heartbeat |= HB_DHT; // 心跳 // 设置DHT任务心跳位
        vTaskDelay(50);             // 50ms 任务节拍 // 控制采样与投递频率
    }
}

/**
 * @brief  控制任务 - 按键/LED/蜂鸣器/ESP32上报
 */
void control_task(void *pvParameters) // 控制任务：按键/LED/蜂鸣器/数据上报 // 控制任务主入口
{
    pvParameters = pvParameters; // 消除未使用告警 // 避免未使用参数告警
    printf("[CTRL] control task started\r\n"); // 启动日志 // 打印任务启动信息
    
    // 启动时测试蜂鸣器和LED（确认硬件正常）
    printf("[CTRL] Testing BEEP and LED...\r\n");
    // 测试蜂鸣器（响2声）
    BEEP_ON(); vTaskDelay(pdMS_TO_TICKS(200)); BEEP_OFF(); vTaskDelay(pdMS_TO_TICKS(100));
    BEEP_ON(); vTaskDelay(pdMS_TO_TICKS(200)); BEEP_OFF();
    // 测试LED1绿灯（亮1秒）
    LED1_Set(1); 
    printf("[CTRL] LED1 ON\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    LED1_Set(0);
    printf("[CTRL] LED1 OFF\r\n");
    // 测试LED0红灯（亮1秒）
    LED0_SetPWM(500);
    printf("[CTRL] LED0 ON\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    LED0_SetPWM(0);
    printf("[CTRL] LED0 OFF\r\n");
    printf("[CTRL] Hardware test done\r\n");

    /* 蜂鸣器控制 - 新逻辑：
     * buzzer_local_enabled: 按键本地控制（有节奏响）
     * buzzer_cloud_enabled: 阿里云远程控制（一直响，优先级最高）
     * 优先级：阿里云 > 按键
     */
    static uint8_t buzzer_local_enabled = 0;  // 按键本地控制（默认关闭）
    static uint8_t buzzer_cloud_enabled = 0;  // 阿里云远程控制（默认关闭）
    static uint8_t beep_on = 0; // 当前蜂鸣器是否处于鸣叫态（1鸣叫/0静音）
    static TickType_t last_toggle = 0; // 上次蜂鸣器状态切换的节拍计数
    static float g_beep_intensity = 0.5f; // 蜂鸣器节奏强度（用于本地控制）

    /* 首/次检测双槽位缓存（1/2轮换）：用于云端双槽展示 */
    static int xinlv1 = -1, xinlv2 = -1; // 槽1/槽2心率缓存（默认无效-1）
    static char shijian1[32] = {0}, shijian2[32] = {0}; // 槽1/槽2时间字符串
    static char gps1[128] = {0}, gps2[128] = {0}; // 槽1/槽2 GPS字符串
    static uint8_t next_slot = 1; // 下次写入的槽位号（1或2）
    static uint8_t prev_hrresp_valid = 0; // 上一周期HR/Resp有效性标志
    
    /* 槽位更新模式：0=定期轮换模式（每次上报轮换），1=边沿触发模式（仅在人离开后再回来时切换） */
    static uint8_t slot_update_mode = 0; // 默认定期轮换模式

    /* 边缘算法状态 */
    edge_alarm_out_t alarm_out; // 边缘告警算法输出结构体
    uint8_t prev_apnea = 0, prev_hr_alarm = 0, prev_struggle = 0, prev_away = 0; // 事件边沿检测缓存

    /* 上报周期控制 */
    TickType_t last_prop_post = 0; // 上次属性上报的节拍时间戳

    while(1) // 永久循环 // 任务主循环
    {
        /* UI覆盖：控制云端蜂鸣器 */
        if(g_ui_buzzer_override == 0) { buzzer_cloud_enabled = 0; } // UI强制关闭云端控制
        else if(g_ui_buzzer_override == 1) { buzzer_cloud_enabled = 1; } // UI强制开启云端控制

        /* ESP32蜂鸣器远程控制（阿里云下发，优先级最高） */
        if(g_rx_buzzer_cmd_flag) {
            buzzer_cloud_enabled = g_rx_buzzer_state;  // 阿里云控制状态
            g_rx_buzzer_cmd_flag = 0;
            printf("[CTRL] Cloud buzzer cmd: %s\r\n", buzzer_cloud_enabled?"ON(常响)":"OFF");
            // 云端开启时立即响
            if(buzzer_cloud_enabled) {
                BEEP_ON();
                beep_on = 1;
                printf("[CTRL] BEEP_ON called!\r\n");
            }
            // 云端关闭时，如果本地也关闭，则立即停止蜂鸣器
            if(!buzzer_cloud_enabled && !buzzer_local_enabled) {
                BEEP_OFF();
                beep_on = 0;
            }
        }

        /* 按键处理：KEY0单击切换本地蜂鸣器（有节奏响）；KEY_UP单击切换UI页面 */
        u8 key = KEY_Scan(0);
        
        /* 调试：每2秒打印一次按键原始状态 */
        static TickType_t last_key_debug = 0;
        if(xTaskGetTickCount() - last_key_debug >= pdMS_TO_TICKS(2000)) {
            last_key_debug = xTaskGetTickCount();
            printf("[KEY_DBG] KEY0=%d, WK_UP=%d, scan=%d\r\n", 
                   (int)HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0),
                   (int)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0),
                   (int)key);
        }
        
        if(key == KEY0_PRES)
        {
            buzzer_local_enabled = !buzzer_local_enabled;  // 切换本地控制
            printf("[CTRL] KEY0 toggled local buzzer: %s\r\n", buzzer_local_enabled?"ON(节奏)":"OFF");
            // 开启时短暂蜂鸣反馈
            if(buzzer_local_enabled) {
                BEEP_ON(); 
                printf("[CTRL] BEEP feedback ON\r\n");
                vTaskDelay(pdMS_TO_TICKS(200)); 
                BEEP_OFF();
            } else {
                // 关闭时立即停止（除非云端控制开启）
                if(!buzzer_cloud_enabled) {
                    BEEP_OFF();
                    beep_on = 0;
                }
            }
        }
        else if(key == WKUP_PRES)
        {
            static TickType_t last_page_switch = 0;
            TickType_t now = xTaskGetTickCount();
            // 防抖：页面切换间隔至少500ms
            if((now - last_page_switch) >= pdMS_TO_TICKS(500))
            {
                g_ui_page_index = (g_ui_page_index + 1) % 3; // 循环切页 // 0:MAIN 1:SAR 2:故障
                printf("[CTRL] KEY_UP switched to page: %d\r\n", g_ui_page_index); // 日志 // 打印当前页
                last_page_switch = now;
            }
        }

        /* 读取雷达与GPS状态（只读快照） */
        radar_state_t r = radar_get_state(); // 雷达快照 // 获取当前雷达检测状态
        nmea_msg* gps = GPS_Get_Data();      // GPS快照 // 获取当前GPS解析数据

        /* 存在指示与LED控制（普通模式） */
        if(r.exist == 1) // 存在目标 // 检测到有人时进入
        {
            /* 有人：LED1(绿灯)亮，LED0(红灯)根据距离调光 */
            /* 直接点亮LED1，不受UI覆盖影响 */
            LED1_Set(1); // 有人时绿灯常亮

            float prox = map_proximity_intensity(r.distance_cm); // 计算距离对应的接近强度[0..1]
            
            /* LED0红灯：距离越近越亮 */
            uint16_t led0_duty = (uint16_t)(prox * 500.0f); // 将接近强度映射到占空比
            LED0_SetPWM(led0_duty); // 设置LED0红灯占空比
            float motion = (r.body_motion >= 0) ? ((float)r.body_motion / 100.0f) : 0.0f; // 体动归一化[0..1]
            
            /* HR/Resp 归一化 */
            float hr_norm = 0.0f; // 心率归一化值
            if(r.heart_rate >= 0) {
                hr_norm = ((float)r.heart_rate - 50.0f) / 80.0f; // 以50~130bpm映射到[0..1]
                if(hr_norm < 0.0f) hr_norm = 0.0f; // 下限保护
                if(hr_norm > 1.0f) hr_norm = 1.0f; // 上限保护
            }
            float resp_norm = 0.0f; // 呼吸归一化值
            if(r.respiration >= 0) {
                resp_norm = ((float)r.respiration - 8.0f) / 22.0f; // 以8~30rpm映射到[0..1]
                if(resp_norm < 0.0f) resp_norm = 0.0f; // 下限保护
                if(resp_norm > 1.0f) resp_norm = 1.0f; // 上限保护
            }
            
            float intensity = prox; // 总强度初值取接近度
            if(motion > intensity) intensity = motion; // 与体动取较大者
            if(hr_norm > intensity) intensity = hr_norm; // 与心率取较大者
            if(resp_norm > intensity) intensity = resp_norm; // 与呼吸取较大者
            if(intensity < 0.0f) intensity = 0.0f; // 下限保护
            if(intensity > 1.0f) intensity = 1.0f; // 上限保护

            /* 有人时：记录强度用于本地蜂鸣器节奏控制 */
            g_beep_intensity = intensity;
        }
        else
        {
            /* 无人：LED0(红灯)呈吸效果，LED1(绿灯)灭 */
            LED1_Set(0); // 无人时绿灯熄灭
            
            // LED0红灯呈吸效果（慢速闪烁）
            static uint32_t breath_time = 0; // 上次“呼吸值”更新时间戳
            uint32_t now = HAL_GetTick(); // 当前毫秒计数
            if(now - breath_time >= 10) {  // 每10ms更新一次 // 控制呼吸步进
                static uint16_t breath_val = 0; // 当前占空比值
                static int8_t breath_dir = 1; // 变化方向（1增/−1减）
                
                breath_val += breath_dir * 5;  // 每次增加/减少5 // 形成平滑变化
                if(breath_val >= 500) { breath_val = 500; breath_dir = -1; } // 达上限则反向
                if(breath_val == 0) { breath_dir = 1; } // 回到0则再次增亮
                
                LED0_SetPWM(breath_val); // 应用当前占空比
                breath_time = now; // 记录更新时间
            }
            
            /* 无人时：本地蜂鸣器不响（在下面统一处理） */
        }

        /* SAR 模式：以 life_score & 距离做更敏感引导 */ // SAR模式：基于生命评分与距离增强指示
        if(g_sar_mode) // SAR搜救模式效果 // 当处于搜救模式时进入该分支
        {
            edge_alarm_update(&r, HAL_GetTick(), &alarm_out); // 更新边缘告警算法输出（输入雷达状态和当前毫秒时刻）

            float intensity = (float)alarm_out.life_score / 100.0f; // 将LifeScore(0-100)归一化到[0,1]
            if(intensity < 0.0f) intensity = 0.0f; // 下限保护，避免小于0
            if(intensity > 1.0f) intensity = 1.0f; // 上限保护，避免大于1

            /* SAR模式：LED0红灯PWM显示LifeScore强度，LED1绿灯闪烁 */ // 视觉引导：红灯强度+绿灯闪烁频率
            // LED0: 亮度随生命信号强度变化 // 红灯占空比=强度*500
            uint16_t led0_duty = (uint16_t)(intensity * 500.0f); // 计算LED0占空比(0-500)
            LED0_SetPWM(led0_duty); // 设置LED0红灯PWM占空比
            
            // LED1: 闪烁频率随强度变化 // 强度越大闪烁越快
            static uint32_t led_toggle_time = 0; // 记录上次LED1翻转时间戳
            uint32_t now = HAL_GetTick(); // 读取当前毫秒计数
            uint32_t blink_period = (uint32_t)(1000.0f - intensity * 800.0f); // 200ms~1000ms // 根据强度计算闪烁周期
            if(blink_period < 200) blink_period = 200; // 最小周期限制200ms，避免过快
            
            if(now - led_toggle_time >= blink_period) { // 到达一个周期则翻转LED1
                static uint8_t led_state = 0; // 记录LED1当前状态
                led_state = !led_state; // 取反实现闪烁
                LED1_Set(led_state); // 设置LED1（绿灯）开关
                led_toggle_time = now; // 更新时间戳
            }

            /* SAR模式：记录强度用于蜂鸣器节奏控制 */
            g_beep_intensity = intensity;
        }

        /* ========== 统一蜂鸣器控制（在所有模式判断之后） ========== 
         * 优先级：阿里云(常响) > 按键(节奏响)
         * 阿里云控制无视雷达状态，始终有效
         */
        if(buzzer_cloud_enabled)
        {
            // 阿里云控制：一直响（优先级最高，无视雷达状态）
            BEEP_ON();
            beep_on = 1;
        }
        else if(buzzer_local_enabled && r.exist == 1)
        {
            // 按键本地控制：有人时有节奏响
            float period_ms_f = 1000.0f - g_beep_intensity * 850.0f;
            if(period_ms_f < 150.0f) period_ms_f = 150.0f;
            uint32_t period_ms = (uint32_t)period_ms_f;
            uint32_t on_ms = period_ms / 3;

            TickType_t now_tick = xTaskGetTickCount();
            TickType_t dt = now_tick - last_toggle;
            TickType_t on_ticks = pdMS_TO_TICKS(on_ms);
            TickType_t off_ticks = pdMS_TO_TICKS(period_ms - on_ms);

            if(beep_on) {
                if(dt >= on_ticks) { BEEP_OFF(); beep_on = 0; last_toggle = now_tick; }
            } else {
                if(dt >= off_ticks) { BEEP_ON(); beep_on = 1; last_toggle = now_tick; }
            }
        }
        else
        {
            // 都关闭或无人，蜂鸣器静音
            BEEP_OFF();
            beep_on = 0;
        }

        /* 事件触发：写入离线缓冲 */ // 依据算法事件生成离线事件记录
        {
            edge_alarm_update(&r, HAL_GetTick(), &alarm_out); // 刷新算法输出，确保下方判断使用最新值
            offline_event_t ev; // 定义离线事件结构体
            memset(&ev, 0, sizeof(ev)); // 清零初始化结构体
            ev.ts = HAL_GetTick(); // 记录事件时间戳（毫秒）
            ev.exist = (uint8_t)((r.exist==1)?1:0); // 是否有人：1是/0否
            ev.hr = (uint8_t)((r.heart_rate>=0)?r.heart_rate:0); // 心率：无效则0
            ev.resp = (uint8_t)((r.respiration>=0)?r.respiration:0); // 呼吸：无效则0
            ev.motion = (uint8_t)((r.body_motion>=0)?r.body_motion:0); // 体动：无效则0
            ev.distance = (uint16_t)((r.distance_cm>=0)?r.distance_cm:0); // 距离：无效则0

            if(alarm_out.apnea_level>0 && prev_apnea==0) { // 呼吸暂停从无到有触发
                ev.code = (uint8_t)(10+alarm_out.apnea_level); // 编码：10+等级
                offline_push(&ev); // 推入离线缓冲
            }
            prev_apnea = alarm_out.apnea_level>0 ? 1:0; // 记录本轮状态
            
            if(alarm_out.hr_alarm_level>0 && prev_hr_alarm==0) { // 心率告警从无到有触发
                ev.code = (uint8_t)(20+alarm_out.hr_alarm_level); // 编码：20+等级
                offline_push(&ev); // 推入离线缓冲
            }
            prev_hr_alarm = alarm_out.hr_alarm_level>0 ? 1:0; // 记录本轮状态
            
            if(alarm_out.struggle && !prev_struggle) { // 发生挣扎事件的上升沿
                ev.code = 31; // 编码31：挣扎
                offline_push(&ev); // 推入离线缓冲
            }
            prev_struggle = alarm_out.struggle; // 更新挣扎状态缓存
            
            if(alarm_out.away_too_long && !prev_away) { // 长时间离开事件上升沿
                ev.code = 41; // 编码41：离开过久
                offline_push(&ev); // 推入离线缓冲
            }
            prev_away = alarm_out.away_too_long; // 更新离开状态缓存
        }

        /* 首/次检测触发（边沿触发模式） */ 
        uint8_t now_hrresp_valid = (r.heart_rate >= 0 && r.respiration >= 0) ? 1 : 0; // 判定本轮HR/Resp是否有效
        if(slot_update_mode == 1 && now_hrresp_valid && !prev_hrresp_valid) // 边沿触发模式：从无效->有效的上升沿
        {
            uint8_t slot_used = next_slot; // 记录当前使用的槽位号
            
            /* 组装时间字符串 */
            char timebuf[32] = {0};
            if(!ESP32_Query_Time(timebuf, sizeof(timebuf))) {
                timebuf[0] = '\0';
            }

            /* 组装 GPS 定位字符串 */
            char gpsbuf[128] = {0};
            if(gps)
            {
                double lat = gps->latitude / 100000.0;
                if(gps->nshemi == 'S' || gps->nshemi == 's') lat = -lat;
                double lon = gps->longitude / 100000.0;
                if(gps->ewhemi == 'W' || gps->ewhemi == 'w') lon = -lon;
                double alt = gps->altitude / 10.0;
                snprintf(gpsbuf, sizeof(gpsbuf),
                         "lat=%.6f,lon=%.6f,alt=%.1fm,fix=%u,sv=%u",
                         lat, lon, alt, (unsigned)gps->fixmode, (unsigned)gps->svnum);
            }

            if(next_slot == 1) {
                xinlv1 = (r.heart_rate >= 0) ? r.heart_rate : xinlv1;
                strncpy(shijian1, timebuf, sizeof(shijian1)-1);
                strncpy(gps1, gpsbuf, sizeof(gps1)-1);
                next_slot = 2;
            } else {
                xinlv2 = (r.heart_rate >= 0) ? r.heart_rate : xinlv2;
                strncpy(shijian2, timebuf, sizeof(shijian2)-1);
                strncpy(gps2, gpsbuf, sizeof(gps2)-1);
                next_slot = 1;
            }
            printf("[CTRL] edge-trigger slot %d: HR=%d time=%s GPS=%s\r\n",
                   slot_used, r.heart_rate, timebuf, gpsbuf);
        }
        prev_hrresp_valid = now_hrresp_valid; // 更新上次有效性标志

        /* 周期性属性上报（每5s） */
        if(xTaskGetTickCount() - last_prop_post >= pdMS_TO_TICKS(5000))
        {
            bool gps_ok = GPS_Is_Valid() ? true : false;
            bool radar_ok = (r.exist != -1) ? true : false;

            float temperature_c = 0.0f, humidity_p = 0.0f;
            dht11_data_t* d = DHT11_Get_Data();
            if(d && d->valid)
            {
                temperature_c = (float)d->temperature_int + (float)d->temperature_dec / 10.0f;
                humidity_p = (float)d->humidity_int + (float)d->humidity_dec / 10.0f;
            }

            /* 定期轮换模式：每次上报时更新当前槽位数据 */
            if(slot_update_mode == 0 && now_hrresp_valid)
            {
                /* 组装时间字符串 */
                char timebuf[32] = {0};
                if(!ESP32_Query_Time(timebuf, sizeof(timebuf))) {
                    timebuf[0] = '\0';
                }

                /* 组装 GPS 定位字符串 */
                char gpsbuf[128] = {0};
                if(gps)
                {
                    double lat = gps->latitude / 100000.0;
                    if(gps->nshemi == 'S' || gps->nshemi == 's') lat = -lat;
                    double lon = gps->longitude / 100000.0;
                    if(gps->ewhemi == 'W' || gps->ewhemi == 'w') lon = -lon;
                    double alt = gps->altitude / 10.0;
                    snprintf(gpsbuf, sizeof(gpsbuf),
                             "lat=%.6f,lon=%.6f,alt=%.1fm,fix=%u,sv=%u",
                             lat, lon, alt, (unsigned)gps->fixmode, (unsigned)gps->svnum);
                }

                if(next_slot == 1) {
                    xinlv1 = (r.heart_rate >= 0) ? r.heart_rate : xinlv1;
                    strncpy(shijian1, timebuf, sizeof(shijian1)-1);
                    strncpy(gps1, gpsbuf, sizeof(gps1)-1);
                    next_slot = 2; // 下次切到槽2
                } else {
                    xinlv2 = (r.heart_rate >= 0) ? r.heart_rate : xinlv2;
                    strncpy(shijian2, timebuf, sizeof(shijian2)-1);
                    strncpy(gps2, gpsbuf, sizeof(gps2)-1);
                    next_slot = 1; // 下次切回槽1
                }
                printf("[CTRL] periodic slot %d: HR=%d time=%s GPS=%s\r\n",
                       (next_slot == 2) ? 1 : 2, r.heart_rate, timebuf, gpsbuf);
            }

            /* 通过ESP32上报传感器数据到阿里云 */
            ESP32_Send_SensorData(
                gps_ok,
                radar_ok,
                temperature_c,
                humidity_p,
                gps1,
                gps2,
                xinlv1,
                xinlv2,
                shijian1,
                shijian2);
            
            last_prop_post = xTaskGetTickCount();
        }

        /* 串口原始数据调试打印 */
        if(g_uart_debug) {
            debug_print_uart_raw();
        }

        g_task_heartbeat |= HB_CTRL; // 心跳 // 标记控制任务活动
        vTaskDelay(20);              // 20ms 任务节拍 // 控制任务循环节拍
    }
}

/**
 * @brief  看门狗任务 - 定期喂狗
 * @note   简化版：无条件喂狗，确保系统不会因任务阻塞而完全卡死
 *         如果FreeRTOS调度器本身卡死，看门狗会触发复位
 */
void watchdog_task(void *pvParameters)
{
    pvParameters = pvParameters;
    
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000)); // 2秒周期
        IWDG_Feed();
        // printf("[WD] Feed OK\r\n"); // 调试时可打开
    }
}

/**
 * @brief  FreeRTOS栈溢出钩子函数
 * @note   当检测到任务栈溢出时调用，打印出错任务名称
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf("[FATAL] Stack overflow in task: %s\r\n", pcTaskName);
    while(1); // 停止运行，便于调试
}

/**
 * @brief  FreeRTOS内存分配失败钩子函数
 * @note   当动态内存分配失败时调用
 */
void vApplicationMallocFailedHook(void)
{
    printf("[FATAL] FreeRTOS malloc failed! Free heap: %d bytes\r\n", 
           xPortGetFreeHeapSize());
    while(1); // 停止运行，便于调试
}
