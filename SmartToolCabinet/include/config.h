// config.h - 单工具柜版本
#pragma once
#include <Arduino.h>
#include <Wire.h>

// ========== 设备标识 ==========
#define DEVICE_ID "TOOLBOX_SINGLE"
#define SYSTEM_VERSION "2.0.0"

// ========== OneNet平台配置 ==========
#define ONENET_PRODUCT_ID "40PeGZrCs7"
#define ONENET_DEVICE_ID "TOOLBOX_SINGLE"
#define ONENET_TOKEN "version=2018-10-31&res=products%2F40PeGZrCs7%2Fdevices%2FTOOLBOX_SINGLE&et=1799319600&method=md5&sign=TuVZ5A79pZ8652k7eEMilQ%3D%3D"
#define ONENET_SERVER "mqtts.heclouds.com"
#define ONENET_PORT 1883  

// MQTT主题（按照物模型规范）
#define ONENET_TOPIC_PROPERTY_POST "$sys/" ONENET_PRODUCT_ID "/" ONENET_DEVICE_ID "/thing/property/post"
#define ONENET_TOPIC_PROPERTY_SET "$sys/" ONENET_PRODUCT_ID "/" ONENET_DEVICE_ID "/thing/property/set"
#define ONENET_TOPIC_EVENT_POST "$sys/" ONENET_PRODUCT_ID "/" ONENET_DEVICE_ID "/thing/event/post"
// 服务调用主题需要使用通配符订阅
#define ONENET_TOPIC_SERVICE_CALL "$sys/" ONENET_PRODUCT_ID "/" ONENET_DEVICE_ID "/thing/service/+/invoke"
#define ONENET_TOPIC_SERVICE_REPLY "$sys/" ONENET_PRODUCT_ID "/" ONENET_DEVICE_ID "/thing/service/invoke_reply"

// 心跳间隔
#define HEARTBEAT_INTERVAL 30000

// ========== 网络配置 ==========
#define WIFI_SSID "666"
#define WIFI_PASSWORD "ytr123456"

// ========== 引脚分配 ==========
// RFID模块（SPI）
#define RFID_RST_PIN 38
#define RFID_SS_PIN  21
#define RFID_SCK_PIN 47
#define RFID_MISO_PIN 0
#define RFID_MOSI_PIN 45

//摄像头引脚
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5
#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

// OLED显示
#define OLED_SDA 39
#define OLED_SCL 40
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// 旋转编码器以及功能按键（优化去抖）
#define ENCODER_PSH 41
#define ENCODER_TRA 42
#define ENCODER_TRB 2
#define BUTTON_BACK 1
#define BUTTON_CONFIRM 0xFF

// WS2812B LED
#define WS2812_PIN 48
#define WS2812_NUM_LEDS 8

// 执行器和传感器
#define RELAY_PIN 44        // 电磁锁控制（通电解锁）
#define DOOR_PIN 14        // 门磁传感器
#define BUZZER_PIN 46       // 蜂鸣器（低电平触发）

// ========== 系统参数 ==========
#define TOTAL_TOOLS 6      // 工具数量
#define BUTTON_DEBOUNCE_MS 80
#define LONG_PRESS_MS 1000
#define BACKLIGHT_TIMEOUT 30000
#define DOOR_OPEN_TIMEOUT 60000
#define ALARM_TIMEOUT 5000

// ========== 工具信息结构体 ==========
struct ToolInfo {
    const char* name;      // 工具名称
    uint32_t rfid_uid;     // RFID UID
    bool is_borrowed;      // 借用状态
};

// ========== 系统状态枚举 ==========
enum class AppState {
    INITIALIZING,
    CONNECTING,
    READY,
    MAIN_MENU,
    BORROW_TOOL_SELECT,    // 选择借用工具
    BORROW_CONFIRM,
    RETURN_TOOL_SELECT,    // 选择归还工具
    RETURN_CONFIRM,
    WAIT_RFID_FOR_RETURN,
    WAIT_RFID_FOR_BORROW,
    UNLOCKED,
    WAIT_DOOR_CLOSE,
    ALARM
};

// ========== 操作模式 ==========
enum class OperationMode {
    LOCAL_CONTROL,
    REMOTE_MQTT,
    BORROW_MODE,
    RETURN_MODE,
    MAINTENANCE
};

// ========== 按键动作枚举 ==========
enum class ButtonAction {
    NONE,
    ENCODER_PRESSED,           // 旋钮按下（确认/选择）
    ENCODER_LONG_PRESSED,      // 旋钮长按（返回主菜单）
    BACK_PRESSED,              // BACK按钮按下（取消/返回）
    BACK_LONG_PRESSED,         // BACK长按（查看系统状态）
    ENCODER_ROTATE_CW,         // 顺时针旋转（正常）
    ENCODER_ROTATE_CCW,        // 逆时针旋转（正常）
    ENCODER_ROTATE_CW_PRESSED, // 顺时针旋转（按键按下）
    ENCODER_ROTATE_CCW_PRESSED // 逆时针旋转（按键按下）
};

// ========== 全局变量声明 ==========
extern AppState g_state;
extern OperationMode g_mode;
extern uint8_t g_selected_tool_index;
extern bool g_DoorUnlockAuthorized;
extern bool g_AlarmTriggered;
extern bool system_initialized;
extern bool wifi_connected;
extern bool mqtt_connected;
extern uint32_t alarm_count;
extern ToolInfo g_tools[TOTAL_TOOLS];

// ========== U8g2 OLED显示类声明 ==========
class U8G2Display;  // 前向声明
extern U8G2Display display;  // 全局对象声明

// ========== 全局函数声明 ==========
#ifdef __cplusplus
extern "C" {
#endif
void lock_open();  // 开锁函数（无参数，单工具柜）
void storage_save();
#ifdef __cplusplus
}
#endif