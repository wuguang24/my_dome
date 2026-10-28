// wifi_mqtt.cpp - OneNet物模型兼容版本
#include "wifi_mqtt.h"
#include "storage.h"
#include <ArduinoJson.h>

extern Storage storage;

WiFiMQTT network;

WiFiMQTT::WiFiMQTT() 
    : mqtt_client(wifi_client), 
      last_heartbeat(0), 
      last_reconnect_attempt(0),
      connected(false),
      tool_status_bitmap(0),
      door_status(false),
      alarm_status(false),
      lock_status(true),  // 默认锁关闭
      wifi_signal(0),
      system_status(0),   // 初始化中
      error_code(0) {
    
    product_id = ONENET_PRODUCT_ID;
    device_id = ONENET_DEVICE_ID;
}

void WiFiMQTT::init() {
    Serial.println("初始化网络...");
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    
    mqtt_client.setServer(ONENET_SERVER, ONENET_PORT);
    mqtt_client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->mqttCallback(topic, payload, length);
    });
    mqtt_client.setBufferSize(2048);
    
    Serial.println("网络初始化完成");
}

bool WiFiMQTT::connect() {
    if (connectWiFi() && connectToOneNet()) {
        connected = true;
        system_status = 1; // 运行正常
        publishAllProperties();
        return true;
    }
    connected = false;
    system_status = 3; // 离线模式
    return false;
}

bool WiFiMQTT::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return true;
    
    Serial.printf("连接Wi-Fi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        updateWiFiSignal();
        Serial.printf("\nWi-Fi连接成功! IP: %s, RSSI: %d dBm\n", 
                      WiFi.localIP().toString().c_str(), wifi_signal);
        return true;
    }
    
    Serial.println("\nWi-Fi连接失败!");
    return false;
}

bool WiFiMQTT::connectToOneNet() {
    if (!WiFi.isConnected()) {
        Serial.println("WiFi未连接，无法连接OneNet");
        return false;
    }

    const char* client_id = ONENET_DEVICE_ID;
    const char* username = ONENET_PRODUCT_ID;
    const char* password = ONENET_TOKEN;

    Serial.println("\n=== 连接 OneNet ===");
    Serial.printf("Client ID: %s\n", client_id);
    Serial.printf("Username: %s\n", username);
    Serial.printf("Password: %s\n", password);

    mqtt_client.setServer(ONENET_SERVER, ONENET_PORT);

    if (mqtt_client.connect(client_id, username, password)) {
        Serial.println("成功连接到 OneNet!");
        
        // 订阅属性设置主题
        String property_set_topic = getPropertySetTopic();
        if (mqtt_client.subscribe(property_set_topic.c_str())) {
            Serial.printf("已订阅属性设置主题: %s\n", property_set_topic.c_str());
        }
        
        // 订阅服务调用主题
        String service_call_topic = getServiceCallTopic();
        if (mqtt_client.subscribe(service_call_topic.c_str())) {
            Serial.printf("已订阅服务调用主题: %s\n", service_call_topic.c_str());
        }
        
        return true;
    } else {
        Serial.printf("连接失败，错误码: %d\n", mqtt_client.state());
        return false;
    }
}

String WiFiMQTT::getPropertyPostTopic() {
    return ONENET_TOPIC_PROPERTY_POST;
}

String WiFiMQTT::getEventPostTopic() {
    return ONENET_TOPIC_EVENT_POST;
}

String WiFiMQTT::getPropertySetTopic() {
    return ONENET_TOPIC_PROPERTY_SET;
}

String WiFiMQTT::getServiceCallTopic() {
    return ONENET_TOPIC_SERVICE_CALL;
}

String WiFiMQTT::getServiceReplyTopic(const String& service_id) {
    return "$sys/" + product_id + "/" + device_id + "/thing/service/" + service_id + "/reply";
}

void WiFiMQTT::mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topic_str = String(topic);
    String payload_str;
    
    for (unsigned int i = 0; i < length; i++) {
        payload_str += (char)payload[i];
    }
    
    Serial.printf("收到OneNet消息: [%s] %s\n", topic_str.c_str(), payload_str.c_str());
    
    parseCommand(topic_str, payload_str);
}

void WiFiMQTT::parseCommand(const String& topic, const String& payload) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.printf("JSON解析错误: %s\n", error.c_str());
        return;
    }
    
    if (topic == getPropertySetTopic()) {
        String msg_id = doc["id"] | String(millis());
        JsonObject params = doc["params"];
        if (!params.isNull()) {
            handlePropertySet(params);
            
            // 发送属性设置响应
            String reply_topic = "$sys/" + product_id + "/" + device_id + "/thing/property/set_reply";
            String reply = "{\"id\":\"" + msg_id + "\",\"code\":200,\"msg\":\"success\"}";
            mqtt_client.publish(reply_topic.c_str(), reply.c_str());
            Serial.println("已发送属性设置响应");
        }
    } 
    // 服务调用主题格式: $sys/{product_id}/{device_name}/thing/service/{identifier}/invoke
    else if (topic.indexOf("/thing/service/") >= 0 && topic.endsWith("/invoke")) {
        // 从 topic 中提取 service identifier
        int start = topic.lastIndexOf("/thing/service/") + 15;
        int end = topic.lastIndexOf("/invoke");
        String service_identifier = topic.substring(start, end);
        
        String msg_id = doc["id"] | String(millis());
        
        Serial.printf("收到服务调用: %s\n", service_identifier.c_str());
        
        if (doc.containsKey("params")) {
            JsonObject params = doc["params"];
            handleServiceCall(service_identifier, params, msg_id);
        } else {
            // 没有参数的服务调用
            StaticJsonDocument<64> empty_doc;
            JsonObject empty_params = empty_doc.to<JsonObject>();
            handleServiceCall(service_identifier, empty_params, msg_id);
        }
    }
}

void WiFiMQTT::handlePropertySet(const JsonObject& params) {
    Serial.println("处理属性设置命令:");
    serializeJson(params, Serial);
    Serial.println();
    
    if (params.containsKey("tool_status")) {
        uint8_t new_tool_status = params["tool_status"];
        
        // 更新工具状态位图
        for (int i = 0; i < 6; i++) {
            bool should_borrow = (new_tool_status >> i) & 0x01;
            bool is_borrowed = (tool_status_bitmap >> i) & 0x01;
            
            if (should_borrow != is_borrowed) {
                Serial.printf("远程设置工具%d状态: %s\n", 
                    i+1, should_borrow ? "借用" : "归还");
                
                setToolStatusBit(i, should_borrow);
            }
        }
        
        publishToolStatusBitmap();
    }
    
    // 处理账号密码同步（从云平台下发）
    if (params.containsKey("account") && params.containsKey("password")) {
        String accountStr = params["account"].as<String>();
        String passwordStr = params["password"].as<String>();
        
        Serial.println("收到云平台账号同步:");
        Serial.printf("账号: %s\n", accountStr.c_str());
        
        // 同步到本地存储（不再上报，避免覆盖）
        storage.syncUsersFromCloud(accountStr, passwordStr);
    }
}

void WiFiMQTT::handleServiceCall(const String& service_id, const JsonObject& params, const String& msg_id) {
    Serial.printf("处理服务调用: %s, 消息ID: %s\n", service_id.c_str(), msg_id.c_str());
    serializeJson(params, Serial);
    Serial.println();
    
    bool success = false;
    String message = "";
    StaticJsonDocument<256> response_doc;
    JsonObject response_data = response_doc.to<JsonObject>();
    
    if (service_id == "RemoteUnlock") {
        uint8_t tool_index = params["ToolIndex"] | 1;
        uint8_t duration = params["Duration"] | 30;
        uint8_t mode = params["Mode"] | 1;  // 0=归还模式, 1=借用模式
        
        Serial.println("=== RemoteUnlock 服务参数 ===");
        Serial.printf("ToolIndex: %d\n", tool_index);
        Serial.printf("Duration: %d\n", duration);
        Serial.printf("Mode: %d (%s)\n", mode, mode ? "借用" : "归还");
        Serial.println("=============================");
        
        if (tool_index >= 1 && tool_index <= 6) {
            Serial.printf("远程开锁: 工具%d, 时长%d秒, 模式%s\n", 
                         tool_index, duration, mode ? "借用" : "归还");
            
            if (remote_unlock_cb) {
                // 传递模式信息（通过全局变量，因为回调签名不变）
                extern uint8_t g_RemoteUnlockMode;
                g_RemoteUnlockMode = mode;
                Serial.printf("设置 g_RemoteUnlockMode = %d\n", g_RemoteUnlockMode);
                remote_unlock_cb(tool_index - 1, duration);
            }
            
            success = true;
            message = "开锁请求已接收";
            response_data["result"] = true;
        } else {
            success = false;
            message = "工具序号无效";
            response_data["result"] = false;
        }
        
    } 
    else if (service_id == "SetToolStatus") {
        uint8_t tool_index = params["ToolIndex"] | 1;
        uint8_t status = params["Status"] | 0;
        
        if (tool_index >= 1 && tool_index <= 6) {
            uint8_t index = tool_index - 1;
            bool should_borrow = (status == 1);
            
            Serial.printf("设置工具%d状态: %s\n", tool_index, should_borrow ? "借出" : "在库");
            
            setToolStatusBit(index, should_borrow);
            
            if (set_tool_status_cb) {
                set_tool_status_cb(index, should_borrow);
            }
            
            // 立即上报工具状态到云平台
            publishToolStatusBitmap();
            
            success = true;
            message = "状态设置成功";
            response_data["result"] = true;
        } else {
            success = false;
            message = "工具序号无效";
            response_data["result"] = false;
        }
        
    }
    else if (service_id == "SystemReset") {
        uint8_t reset_type = params["ResetType"] | 1;
        
        Serial.printf("系统复位: 类型%d\n", reset_type);
        
        if (system_reset_cb) {
            system_reset_cb(reset_type);
        }
        
        success = true;
        message = "复位请求已接收";
        response_data["result"] = true;
        
    }
    else if (service_id == "TestAlarm") {
        uint8_t alarm_type = params["AlarmType"] | 1;
        uint8_t duration = params["Duration"] | 5;
        
        Serial.printf("测试报警: 类型%d, 时长%d秒\n", alarm_type, duration);
        
        if (test_alarm_cb) {
            test_alarm_cb(alarm_type, duration);
        }
        
        success = true;
        message = "报警测试请求已接收";
        response_data["result"] = true;
        
    }
    else if (service_id == "TakePhoto") {
        uint8_t photo_type = params["PhotoType"] | 1;
        
        Serial.printf("拍照请求: 类型%d\n", photo_type);
        
        if (take_photo_cb) {
            take_photo_cb(photo_type);
        }
        
        success = true;
        message = "拍照请求已接收";
        response_data["result"] = true;
        response_data["ImageUrl"] = "http://example.com/photo.jpg"; // 示例URL
        
    }
    else if (service_id == "GetSystemStatus") {
        success = true;
        message = "系统状态查询成功";
        
        response_data["DeviceID"] = ONENET_DEVICE_ID;
        response_data["IPAddress"] = WiFi.localIP().toString();
        response_data["Uptime"] = millis() / 60000; // 分钟
        response_data["MemoryFree"] = ESP.getFreeHeap() / 1024; // KB
        
    }
    // 添加用户账号
    else if (service_id == "AddUser") {
        String username = params["Username"] | "";
        String password = params["Password"] | "";
        
        Serial.printf("添加用户: %s\n", username.c_str());
        
        if (username.length() == 8 && password.length() == 8) {
            if (storage.userExists(username)) {
                success = false;
                message = "账号已存在";
            } else if (storage.addUser(username, password)) {
                success = true;
                message = "注册成功";
                // 上报最新账号到云平台
                publishUserAccounts();
            } else {
                success = false;
                message = "用户数量已满";
            }
        } else {
            success = false;
            message = "账号密码格式错误";
        }
        response_data["result"] = success;
        
    }
    // 修改密码
    else if (service_id == "ChangePassword") {
        String username = params["Username"] | "";
        String oldPwd = params["OldPassword"] | "";
        String newPwd = params["NewPassword"] | "";
        
        Serial.printf("修改密码: %s\n", username.c_str());
        
        if (storage.verifyUser(username, oldPwd)) {
            if (newPwd.length() == 8) {
                storage.updatePassword(username, newPwd);
                success = true;
                message = "密码修改成功";
                publishUserAccounts();
            } else {
                success = false;
                message = "新密码格式错误";
            }
        } else {
            success = false;
            message = "原密码错误";
        }
        response_data["result"] = success;
        
    } else {
        success = false;
        message = "未知的服务ID";
        response_data["result"] = false;
    }
    
    // 添加消息到响应数据
    response_data["msg"] = message;
    
    // 调用 respondToService，使用消息ID
    respondToService(service_id, msg_id, success, response_data);
}

bool WiFiMQTT::respondToService(const String& service_id, const String& msg_id, bool result, const JsonObject& data) {
    StaticJsonDocument<512> resp;
    resp["id"] = msg_id;
    resp["code"] = result ? 200 : 400;
    
    JsonObject resp_data = resp.createNestedObject("data");
    
    // 复制数据到响应
    if (!data.isNull()) {
        for (JsonPair kv : data) {
            resp_data[kv.key()] = kv.value();
        }
    }
    
    String resp_str;
    serializeJson(resp, resp_str);
    
    // 响应主题格式: $sys/{product_id}/{device_name}/thing/service/{identifier}/invoke_reply
    String topic = "$sys/" + product_id + "/" + device_id + "/thing/service/" + service_id + "/invoke_reply";
    bool publish_success = mqtt_client.publish(topic.c_str(), resp_str.c_str());
    
    String message = data["msg"] | (result ? "成功" : "失败");
    Serial.printf("服务响应: %s, 结果: %s, 消息: %s\n", 
                  service_id.c_str(), 
                  result ? "成功" : "失败",
                  message.c_str());
    
    if (publish_success) {
        Serial.printf("响应已发送到主题: %s\n", topic.c_str());
    }
    
    return publish_success;
}

void WiFiMQTT::loop() {
    if (!mqtt_client.connected()) {
        connected = false;
        system_status = 3; // 离线模式
        
        unsigned long now = millis();
        if (now - last_reconnect_attempt > 5000) {
            last_reconnect_attempt = now;
            Serial.println("尝试重新连接OneNet...");
            if (connect()) {
                last_reconnect_attempt = 0;
            }
        }
    } else {
        connected = true;
        system_status = 1; // 运行正常
        mqtt_client.loop();
        
        unsigned long now = millis();
        if (now - last_heartbeat > HEARTBEAT_INTERVAL) {
            sendHeartbeat();
            last_heartbeat = now;
        }
    }
}

void WiFiMQTT::publishAllProperties() {
    Serial.println("\n=== 上报全量属性 ===");
    
    StaticJsonDocument<2048> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    // 工具状态位图
    JsonObject tool_status_obj = params.createNestedObject("tool_status");
    tool_status_obj["value"] = tool_status_bitmap;
    
    // 柜门状态
    JsonObject door_status_obj = params.createNestedObject("door_status");
    door_status_obj["value"] = door_status;
    
    // 报警状态
    JsonObject alarm_status_obj = params.createNestedObject("alarm_status");
    alarm_status_obj["value"] = alarm_status;
    
    // 电磁锁状态
    JsonObject lock_status_obj = params.createNestedObject("lock_status");
    lock_status_obj["value"] = !lock_status; // 注意：物模型中true=开启，false=关闭
    
    // WiFi信号强度
    updateWiFiSignal();
    JsonObject wifi_signal_obj = params.createNestedObject("wifi_signal");
    wifi_signal_obj["value"] = wifi_signal;
    
    // 系统状态
    JsonObject system_status_obj = params.createNestedObject("system_status");
    system_status_obj["value"] = system_status;
    
    // 错误代码
    JsonObject error_code_obj = params.createNestedObject("error_code");
    error_code_obj["value"] = error_code;
    
    // 用户数量
    JsonObject user_count_obj = params.createNestedObject("user_count");
    user_count_obj["value"] = storage.getUserCount();
    
    // 工具数量（固定6个）
    JsonObject tool_count_obj = params.createNestedObject("tool_count");
    tool_count_obj["value"] = 6;
    
    // 借用中数量（统计tool_status_bitmap中的1的个数）
    uint8_t borrowed = 0;
    for (int i = 0; i < 6; i++) {
        if ((tool_status_bitmap >> i) & 0x01) {
            borrowed++;
        }
    }
    JsonObject borrowed_count_obj = params.createNestedObject("borrowed_count");
    borrowed_count_obj["value"] = borrowed;
    
    String output;
    serializeJson(doc, output);
    
    Serial.println("上报属性JSON:");
    Serial.println(output);
    
    bool success = mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
    Serial.printf("属性上报结果: %s\n", success ? "成功" : "失败");
    
    if (success) {
        Serial.printf("已发送到主题: %s\n", getPropertyPostTopic().c_str());
    }
    
    // 账号只在首次连接时上报一次，避免覆盖云平台数据
    static bool first_connect = true;
    if (first_connect) {
        publishUserAccounts();
        first_connect = false;
    }
}

void WiFiMQTT::publishUserAccounts() {
    Serial.println("上报用户账号...");
    
    // 从 storage 获取用户数据（已经是JSON格式）
    String usersJson = storage.getUsersJson();
    
    // 手动解析简单JSON，避免使用StaticJsonDocument
    int accStart = usersJson.indexOf("\"account\":\"") + 11;
    int accEnd = usersJson.indexOf("\",\"password\"");
    int pwdStart = usersJson.indexOf("\"password\":\"") + 12;
    int pwdEnd = usersJson.indexOf("\",\"count\"");
    
    if (accStart < 11 || accEnd < 0 || pwdStart < 12 || pwdEnd < 0) {
        Serial.println("解析用户数据失败");
        return;
    }
    
    String accounts = usersJson.substring(accStart, accEnd);
    String passwords = usersJson.substring(pwdStart, pwdEnd);
    
    if (accounts.length() == 0) {
        Serial.println("没有用户数据");
        return;
    }
    
    // 直接构建JSON字符串
    String output = "{\"id\":\"" + String(millis()) + "\",\"version\":\"1.0\",\"params\":{";
    output += "\"account\":{\"value\":\"" + accounts + "\"},";
    output += "\"password\":{\"value\":\"" + passwords + "\"}}}";
    
    Serial.println(output);
    
    bool success = mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
    Serial.println(success ? "账号上报成功" : "账号上报失败");
}

void WiFiMQTT::publishToolStatusBitmap() {
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject tool_status_obj = params.createNestedObject("tool_status");
    tool_status_obj["value"] = tool_status_bitmap;
    
    // 同时上报借用中数量
    uint8_t borrowed = 0;
    for (int i = 0; i < 6; i++) {
        if ((tool_status_bitmap >> i) & 0x01) {
            borrowed++;
        }
    }
    JsonObject borrowed_count_obj = params.createNestedObject("borrowed_count");
    borrowed_count_obj["value"] = borrowed;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报工具状态bitmap: 0x%02X, 借用中: %d个\n", tool_status_bitmap, borrowed);
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishDoorStatus(bool open) {
    door_status = open;
    
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject door_status_obj = params.createNestedObject("door_status");
    door_status_obj["value"] = open;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报门状态: %s\n", open ? "开启" : "关闭");
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishAlarmStatus(bool triggered) {
    alarm_status = triggered;
    
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject alarm_status_obj = params.createNestedObject("alarm_status");
    alarm_status_obj["value"] = triggered;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报报警状态: %s\n", triggered ? "报警中" : "正常");
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishLockStatus(bool locked) {
    lock_status = locked;
    
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject lock_status_obj = params.createNestedObject("lock_status");
    lock_status_obj["value"] = !locked; // 注意：物模型中true=开启，false=关闭
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报电磁锁状态: %s\n", locked ? "关闭" : "开启");
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishWiFiSignal() {
    updateWiFiSignal();
    
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject wifi_signal_obj = params.createNestedObject("wifi_signal");
    wifi_signal_obj["value"] = wifi_signal;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报WiFi信号强度: %d dBm\n", wifi_signal);
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishSystemStatus(uint8_t status) {
    system_status = status;
    
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject system_status_obj = params.createNestedObject("system_status");
    system_status_obj["value"] = status;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报系统状态: %d\n", status);
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishErrorCode(uint32_t code) {
    error_code = code;
    
    StaticJsonDocument<512> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject error_code_obj = params.createNestedObject("error_code");
    error_code_obj["value"] = code;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报错误代码: %lu\n", code);
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getPropertyPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishBorrowEvent(uint8_t tool_index, const String& rfid_code, const String& time_str) {
    StaticJsonDocument<1024> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject event_obj = params.createNestedObject("BorrowEvent");
    
    JsonArray value_array = event_obj.createNestedArray("value");
    
    JsonObject item1 = value_array.createNestedObject();
    item1["ToolIndex"] = tool_index + 1;
    
    JsonObject item2 = value_array.createNestedObject();
    item2["RFIDCode"] = rfid_code;
    
    JsonObject item3 = value_array.createNestedObject();
    item3["BorrowTime"] = time_str;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报借用事件: 工具%d, RFID: %s, 时间: %s\n", 
                  tool_index + 1, rfid_code.c_str(), time_str.c_str());
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getEventPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishReturnEvent(uint8_t tool_index, const String& rfid_code, const String& time_str) {
    StaticJsonDocument<1024> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject event_obj = params.createNestedObject("ReturnEvent");
    
    JsonArray value_array = event_obj.createNestedArray("value");
    
    JsonObject item1 = value_array.createNestedObject();
    item1["ToolIndex"] = tool_index + 1;
    
    JsonObject item2 = value_array.createNestedObject();
    item2["RFIDCode"] = rfid_code;
    
    JsonObject item3 = value_array.createNestedObject();
    item3["ReturnTime"] = time_str;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报归还事件: 工具%d, RFID: %s, 时间: %s\n", 
                  tool_index + 1, rfid_code.c_str(), time_str.c_str());
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getEventPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishIllegalOpenEvent(uint8_t alarm_code, uint32_t duration) {
    StaticJsonDocument<1024> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject event_obj = params.createNestedObject("IllegalOpen");
    
    JsonArray value_array = event_obj.createNestedArray("value");
    
    JsonObject item1 = value_array.createNestedObject();
    item1["AlarmCode"] = alarm_code;
    
    JsonObject item2 = value_array.createNestedObject();
    item2["Duration"] = duration;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报非法开门事件: 代码%d, 时长%lu秒\n", alarm_code, duration);
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getEventPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishTimeoutAlarm(uint8_t tool_index, uint32_t overdue_minutes) {
    StaticJsonDocument<1024> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject event_obj = params.createNestedObject("TimeoutAlarm");
    
    JsonArray value_array = event_obj.createNestedArray("value");
    
    JsonObject item1 = value_array.createNestedObject();
    item1["ToolIndex"] = tool_index + 1;
    
    JsonObject item2 = value_array.createNestedObject();
    item2["OverdueTime"] = overdue_minutes;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报超时报警事件: 工具%d, 超时%lu分钟\n", tool_index + 1, overdue_minutes);
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getEventPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::publishSystemError(uint8_t error_code, const String& error_msg) {
    StaticJsonDocument<1024> doc;
    doc["id"] = String(millis());
    doc["version"] = "1.0";
    
    JsonObject params = doc.createNestedObject("params");
    
    JsonObject event_obj = params.createNestedObject("SystemError");
    
    JsonArray value_array = event_obj.createNestedArray("value");
    
    JsonObject item1 = value_array.createNestedObject();
    item1["ErrorCode"] = error_code;
    
    JsonObject item2 = value_array.createNestedObject();
    item2["ErrorMsg"] = error_msg;
    
    String output;
    serializeJson(doc, output);
    
    Serial.printf("上报系统错误事件: 代码%d, 信息: %s\n", error_code, error_msg.c_str());
    Serial.println("JSON: " + output);
    
    mqtt_client.publish(getEventPostTopic().c_str(), output.c_str());
}

void WiFiMQTT::setToolStatusBit(uint8_t tool_index, bool borrowed) {
    if (tool_index < 6) {
        if (borrowed) {
            tool_status_bitmap |= (1 << tool_index);
        } else {
            tool_status_bitmap &= ~(1 << tool_index);
        }
    }
}

bool WiFiMQTT::getToolStatusBit(uint8_t tool_index) {
    if (tool_index < 6) {
        return (tool_status_bitmap >> tool_index) & 0x01;
    }
    return false;
}

void WiFiMQTT::sendHeartbeat() {
    // 心跳包通常发送WiFi信号强度
    publishWiFiSignal();
    Serial.println("发送心跳（WiFi信号上报）");
}

String WiFiMQTT::getCurrentTime() {
    unsigned long seconds = millis() / 1000;
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;
    
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    return String(buffer);
}

String WiFiMQTT::formatErrorCode(uint32_t code) {
    switch (code) {
        case 0: return "恢复正常";
        case 1: return "RFID模块故障";
        case 2: return "门磁传感器故障";
        case 3: return "电磁锁故障";
        case 4: return "WiFi连接失败";
        case 5: return "摄像头故障";
        case 6: return "存储异常";
        default: return "未知错误";
    }
}