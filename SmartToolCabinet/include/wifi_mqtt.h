// wifi_mqtt.h - OneNet物模型兼容版本
#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <WifiClient.h>
#include <ArduinoJson.h>
#include "config.h"

typedef std::function<void(uint8_t tool_index, uint8_t duration)> RemoteUnlockCallback;
typedef std::function<void(uint8_t tool_index, bool status)> SetToolStatusCallback;
typedef std::function<void(uint8_t reset_type)> SystemResetCallback;
typedef std::function<void(uint8_t alarm_type, uint8_t duration)> TestAlarmCallback;
typedef std::function<void(uint8_t photo_type)> TakePhotoCallback;

class WiFiMQTT {
private:
    WiFiClient wifi_client;
    PubSubClient mqtt_client;
    
    unsigned long last_heartbeat;
    unsigned long last_reconnect_attempt;
    bool connected;
    
    // 回调函数
    RemoteUnlockCallback remote_unlock_cb = nullptr;
    SetToolStatusCallback set_tool_status_cb = nullptr;
    SystemResetCallback system_reset_cb = nullptr;
    TestAlarmCallback test_alarm_cb = nullptr;
    TakePhotoCallback take_photo_cb = nullptr;
    
    // OneNet平台参数
    String product_id;
    String device_id;
    
    // 工具状态缓存（与物模型bitMap对应）
    uint8_t tool_status_bitmap;  // 6个工具状态（使用前6位）
    
    // 设备状态缓存
    bool door_status;       // 柜门状态
    bool alarm_status;      // 报警状态
    bool lock_status;       // 电磁锁状态
    int32_t wifi_signal;    // WiFi信号强度
    uint8_t system_status;  // 系统状态
    uint32_t error_code;    // 错误代码
    
public:
    WiFiMQTT();
    void init();
    bool connect();
    void loop();
    
    // WiFi连接
    bool connectWiFi();
    bool connectToOneNet();
    
    // 状态上报（物模型属性）
    void publishAllProperties();  // 上报所有属性
    void publishToolStatusBitmap();  // 工具状态位图
    void publishDoorStatus(bool open);
    void publishAlarmStatus(bool triggered);
    void publishLockStatus(bool locked);
    void publishWiFiSignal();
    void publishSystemStatus(uint8_t status);
    void publishErrorCode(uint32_t code);
    void publishUserAccounts();  // 上报用户账号到云平台
    
    // 设置回调函数
    void setRemoteUnlockCallback(RemoteUnlockCallback cb) { remote_unlock_cb = cb; }
    void setSetToolStatusCallback(SetToolStatusCallback cb) { set_tool_status_cb = cb; }
    void setSystemResetCallback(SystemResetCallback cb) { system_reset_cb = cb; }
    void setTestAlarmCallback(TestAlarmCallback cb) { test_alarm_cb = cb; }
    void setTakePhotoCallback(TakePhotoCallback cb) { take_photo_cb = cb; }
    
    // 事件上报（物模型事件）
    void publishBorrowEvent(uint8_t tool_index, const String& rfid_code, const String& time_str);
    void publishReturnEvent(uint8_t tool_index, const String& rfid_code, const String& time_str);
    void publishIllegalOpenEvent(uint8_t alarm_code, uint32_t duration);
    void publishTimeoutAlarm(uint8_t tool_index, uint32_t overdue_minutes);
    void publishSystemError(uint8_t error_code, const String& error_msg);
    
    // 服务响应 - 简化函数声明
    bool respondToService(const String& service_id, const String& msg_id, bool result, const JsonObject& data = JsonObject());
    
    bool isConnected() const { return connected; }
    String getIP() { return WiFi.localIP().toString(); }
    
    // 工具状态bitmap操作
    void setToolStatusBit(uint8_t tool_index, bool borrowed);
    bool getToolStatusBit(uint8_t tool_index);
    uint8_t getToolStatusBitmap() { return tool_status_bitmap; }
    
    // 设备状态操作
    void setDoorStatus(bool open) { door_status = open; }
    bool getDoorStatus() { return door_status; }
    
    void setAlarmStatus(bool triggered) { alarm_status = triggered; }
    bool getAlarmStatus() { return alarm_status; }
    
    void setLockStatus(bool locked) { lock_status = locked; }
    bool getLockStatus() { return lock_status; }
    
    void updateWiFiSignal() { wifi_signal = WiFi.RSSI(); }
    int32_t getWiFiSignal() { return wifi_signal; }
    
    void setSystemStatus(uint8_t status) { system_status = status; }
    uint8_t getSystemStatus() { return system_status; }
    
    void setErrorCode(uint32_t code) { error_code = code; }
    uint32_t getErrorCode() { return error_code; }
    
private:
    void mqttCallback(char* topic, byte* payload, unsigned int length);
    void parseCommand(const String& topic, const String& payload);
    void handlePropertySet(const JsonObject& params);
    void handleServiceCall(const String& service_id, const JsonObject& params, const String& msg_id);
    void sendHeartbeat();
    
    // 生成OneNet主题
    String getPropertyPostTopic();
    String getEventPostTopic();
    String getPropertySetTopic();
    String getServiceCallTopic();
    String getServiceReplyTopic(const String& service_id);
    
    // 生成OneNet格式的消息
    String generatePropertyMessage(const JsonObject& properties);
    String generateEventMessage(const String& event_id, const JsonObject& params);
    
    // 工具函数
    String getCurrentTime();
    String formatErrorCode(uint32_t code);
};

extern WiFiMQTT network;