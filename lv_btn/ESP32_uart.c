#include <Arduino.h>  // 引入Arduino库
#include <WiFi.h>  // 引入WiFi库
#include "PubSubClient.h"  // 引入MQTT库
#include <ArduinoJson.h>  // 引入Arduino JSON库
#include "FastLED.h"  // 引入FastLED库（用于控制LED）
#include <time.h>
#include "mbedtls/md.h"
#include "esp_log.h"  // ESP-IDF日志库

static const char* TAG = "ESP32";

// WS2812灯配置
#define NUM_LEDS 1  // 定义LED数量为1
#define DATA_PIN 48  // 定义数据引脚为48
#define LED_TYPE WS2812  // 定义LED类型为WS2812
#define COLOR_ORDER RGB  // 定义颜色顺序为RGB
CRGB leds[NUM_LEDS];  // 创建一个LED数组

// 串口配置
#define STM32_RX_PIN 18  // STM32接收引脚为18
#define STM32_TX_PIN 17  // STM32发送引脚为17
HardwareSerial SerialSTM32(1);  // 创建一个HardwareSerial对象用于与STM32通信

// 网络配置
#define WIFI_SSID "wu"  // WiFi名称
#define WIFI_PASSWD "wu12345678"  // WiFi密码

// 阿里云配置（按用户提供的当前项目三元组）
#define PRODUCT_KEY   "a1xT9S4rOnA"
#define DEVICE_NAME   "shebei2025"
#define DEVICE_SECRET "e7b3f398b8d55879929e6bba83a16931"
#define REGION_ID     "cn-shanghai"

// MQTT配置（动态生成 clientId/username/password）
#define MQTT_SERVER PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT   1883

// 主题定义
#define ALINK_BODY_FORMAT "{\"id\":\"wu2025\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"  // 定义阿里云数据上报格式
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"  // 定义上报数据的主题
// 下行属性设置主题
const char* TOPIC_PROP_SET = "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set";
// 通配符订阅所有下行消息（用于调试）
const char* TOPIC_ALL = "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/#";

// 全局变量
WiFiClient espClient;  // 创建WiFi客户端
PubSubClient client(espClient);  // 创建MQTT客户端
unsigned long lastMs = 0;  // 定时器
unsigned long lastWifiCheck = 0;  // WiFi检测定时器
unsigned long lastLedBlink = 0;   // LED闪烁定时器
unsigned long lastDataActivity = 0;  // 最后一次数据交互时间
unsigned long lastPurpleBlink = 0;   // 紫灯闪烁定时器
String serialLine;         // 串口行缓存
int post_expect_len = -1;  // POST 负载长度，-1表示未进入POST接收
String post_payload;       // POST接收缓存
bool light_status = false; // 阿里云LED控制状态（优先级最高）
bool wifi_connected = false;  // WiFi连接状态
bool led_blink_state = false; // LED闪烁状态
bool data_activity_flag = false;  // 数据交互标志（用于触发紫灯）

// 传感器数据缓存（从STM32接收）
struct SensorData {
    bool GPS_zhuangtai = false;
    bool LEIDA_zhuangtai = false;
    float WENDU = 0.0;
    float SHIDU = 0.0;
    String GPS_dingwei1 = "";
    String GPS_dingwei2 = "";
    int XINLV1 = 0;
    int XINLV2 = 0;
    String SHIJIAN1 = "";
    String SHIJIAN2 = "";
    bool dataReady = false;  // 标记数据是否就绪
} sensorData;

// WiFi断线重连（非阻塞）
// 返回：true-已连接，false-未连接
bool wifiCheckAndReconnect() 
{
    if (WiFi.status() == WL_CONNECTED) {
        if (!wifi_connected) {
            // 刚连接成功
            wifi_connected = true;
            ESP_LOGI(TAG, "WiFi Connected! IP: %s", WiFi.localIP().toString().c_str());
            // 连接成功后熄灭红灯
            leds[0] = CRGB::Black;
            FastLED.show();
        }
        return true;
    }
    
    // WiFi未连接
    if (wifi_connected) {
        // 刚断开连接
        wifi_connected = false;
        ESP_LOGW(TAG, "WiFi Disconnected!");
    }
    
    // 尝试重连（每3秒尝试一次）
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt >= 3000) {
        lastReconnectAttempt = millis();
        ESP_LOGI(TAG, "Attempting WiFi reconnect...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    }
    
    return false;
}

// 初始化WiFi连接（启动时调用，带LED指示）
void wifiInit() 
{
    ESP_LOGI(TAG, "Starting WiFi connection...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    
    // 等待连接，最多等待10秒，期间红灯闪烁
    int timeout = 20;  // 20 * 500ms = 10秒
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        // 红灯闪烁指示正在连接
        leds[0] = CRGB::Red;
        FastLED.show();
        delay(250);
        leds[0] = CRGB::Black;
        FastLED.show();
        delay(250);
        
        ESP_LOGI(TAG, "Connecting WiFi... (%d)", timeout);
        timeout--;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        ESP_LOGI(TAG, "WiFi Connected! IP: %s", WiFi.localIP().toString().c_str());
        // 连接成功，绿灯闪烁3次
        for (int i = 0; i < 3; i++) {
            leds[0] = CRGB::Green;
            FastLED.show();
            delay(200);
            leds[0] = CRGB::Black;
            FastLED.show();
            delay(200);
        }
    } else {
        wifi_connected = false;
        ESP_LOGW(TAG, "WiFi connection timeout, will retry in background...");
    }
}

// 标记有数据交互（在收到STM32数据时调用）
void markDataActivity()
{
    lastDataActivity = millis();
    data_activity_flag = true;
}

// LED状态更新（在loop中调用）
// 优先级：阿里云控制 > WiFi断开(红) > MQTT断开(黄) > 运行状态(紫)
void updateLedStatus()
{
    // 最高优先级：阿里云LED控制（常亮蓝灯）
    if (light_status) {
        // 阿里云指令开启，常亮蓝灯，不做其他处理
        static bool cloud_led_set = false;
        if (!cloud_led_set) {
            leds[0] = CRGB::Blue;
            FastLED.show();
            cloud_led_set = true;
        }
        return;  // 直接返回，不执行其他LED逻辑
    }
    
    // 阿里云LED关闭后，恢复状态指示灯逻辑
    static bool cloud_led_set = false;
    cloud_led_set = false;  // 重置标志
    
    // 优先级1：WiFi未连接，红灯闪烁
    if (!wifi_connected) {
        if (millis() - lastLedBlink >= 500) {  // 500ms闪烁周期
            lastLedBlink = millis();
            led_blink_state = !led_blink_state;
            if (led_blink_state) {
                leds[0] = CRGB::Red;
            } else {
                leds[0] = CRGB::Black;
            }
            FastLED.show();
        }
        return;
    }
    
    // 优先级2：WiFi已连接但MQTT未连接，黄灯闪烁
    if (!client.connected()) {
        if (millis() - lastLedBlink >= 300) {  // 300ms闪烁周期
            lastLedBlink = millis();
            led_blink_state = !led_blink_state;
            if (led_blink_state) {
                leds[0] = CRGB::Yellow;
            } else {
                leds[0] = CRGB::Black;
            }
            FastLED.show();
        }
        return;
    }
    
    // 优先级3：WiFi和MQTT都已连接，显示运行状态（紫灯闪烁）
    // 每2秒闪一次紫灯，表示系统正常运行
    // 如果有数据交互，立即闪一次
    
    // 检查是否有数据交互触发
    if (data_activity_flag) {
        data_activity_flag = false;
        // 立即闪一次紫灯
        leds[0] = CRGB::Purple;
        FastLED.show();
        lastPurpleBlink = millis();
        led_blink_state = true;
        return;
    }
    
    // 紫灯亮了100ms后熄灭
    if (led_blink_state && (millis() - lastPurpleBlink >= 100)) {
        leds[0] = CRGB::Black;
        FastLED.show();
        led_blink_state = false;
        return;
    }
    
    // 每2秒自动闪一次紫灯（心跳指示）
    if (!led_blink_state && (millis() - lastPurpleBlink >= 2000)) {
        leds[0] = CRGB::Purple;
        FastLED.show();
        lastPurpleBlink = millis();
        led_blink_state = true;
    }
}

// 检查MQTT连接
static void hmac_sha256_hex(const char* key, const char* msg, char* out_hex, size_t out_len)
{
    unsigned char mac[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_setup(&ctx, info, 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key, strlen(key));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)msg, strlen(msg));
    mbedtls_md_hmac_finish(&ctx, mac);
    mbedtls_md_free(&ctx);
    static const char* hex = "0123456789abcdef";
    for(int i=0;i<32;i++) { out_hex[i*2] = hex[(mac[i]>>4)&0xF]; out_hex[i*2+1]=hex[mac[i]&0xF]; }
    if(out_len>64) out_hex[64]='\0';
}

static void build_mqtt_params(char* out_clientId, size_t cid_sz, char* out_username, size_t un_sz, char* out_password, size_t pw_sz)
{
    // 使用当前epoch毫秒作为timestamp（若NTP已同步则更准确）
    unsigned long long ts_ms = (unsigned long long) (time(NULL)) * 1000ULL;
    char clientIdRaw[96];
    snprintf(clientIdRaw, sizeof(clientIdRaw), "%s.%s", PRODUCT_KEY, DEVICE_NAME); // 与用户给出的格式一致
    snprintf(out_clientId, cid_sz, "%s|securemode=2,signmethod=hmacsha256,timestamp=%llu|", clientIdRaw, ts_ms);
    snprintf(out_username, un_sz, "%s&%s", DEVICE_NAME, PRODUCT_KEY);
    char sign_content[256];
    // 按阿里云规则拼接待签名串
    // signcontent = "clientId" + clientIdRaw + "deviceName" + DEVICE_NAME + "productKey" + PRODUCT_KEY + "timestamp" + ts
    snprintf(sign_content, sizeof(sign_content), "clientId%sdeviceName%sproductKey%stimestamp%llu", clientIdRaw, DEVICE_NAME, PRODUCT_KEY, ts_ms);
    char sign_hex[65]={0};
    hmac_sha256_hex(DEVICE_SECRET, sign_content, sign_hex, sizeof(sign_hex));
    snprintf(out_password, pw_sz, "%s", sign_hex);
}

// MQTT连接检查（非阻塞）
// 返回：true-已连接，false-未连接
bool mqttCheckConnect() 
{
    // 如果WiFi未连接，不尝试MQTT连接
    if (!wifi_connected) {
        return false;
    }
    
    if (client.connected()) {
        return true;
    }
    
    // MQTT未连接，尝试重连（非阻塞，每5秒尝试一次）
    static unsigned long lastMqttAttempt = 0;
    if (millis() - lastMqttAttempt < 5000) {
        return false;  // 还没到重试时间
    }
    lastMqttAttempt = millis();
    
    ESP_LOGI(TAG, "Connecting MQTT...");
    char cid[128], usr[96], pwd[96];
    build_mqtt_params(cid, sizeof(cid), usr, sizeof(usr), pwd, sizeof(pwd));
    
    if (client.connect(cid, usr, pwd)) {
        ESP_LOGI(TAG, "MQTT Connected!");
        // 订阅属性设置主题
        client.subscribe(TOPIC_PROP_SET);
        ESP_LOGI(TAG, "Subscribed to: %s", TOPIC_PROP_SET);
        
        // 也订阅通配符主题用于调试
        client.subscribe(TOPIC_ALL);
        ESP_LOGI(TAG, "Subscribed to: %s", TOPIC_ALL);
        
        // MQTT连接成功，绿灯亮一下
        leds[0] = CRGB::Green;
        FastLED.show();
        delay(500);
        leds[0] = CRGB::Black;
        FastLED.show();
        
        return true;
    } else {
        ESP_LOGE(TAG, "MQTT Connect failed, state: %d", client.state());
        return false;
    }
}

// 定时发送MQTT状态（已禁用，因为物模型中没有online属性）
void mqttIntervalPost() 
{ 
    // 不再上报online属性，传感器数据由publishSensorData()上报
}

// 处理来自STM32的串口协议：
// STATUS?               -> 回复: STATUS:ONLINE/STATUS:OFFLINE\n
// TIME?                 -> 回复: TIME:YYYY-MM-DD HH:MM:SS\n
// POST <len>\n<payload...> -> 发布到 ALINK_TOPIC_PROP_POST，回复: POST:OK/POST:FAIL\n
// DATA:<JSON>           -> 接收传感器数据JSON并缓存
// BUZZER:<cmd>          -> 向STM32发送蜂鸣器控制命令（由云端触发）
void checkSerialData() 
{
    static String receivedData = "";  // 静态变量存储接收到的数据
    
    while (SerialSTM32.available()) {
        char c = (char)SerialSTM32.read();
        
        // 打印接收到的原始数据（调试用）
        Serial.print("[ESP32] RX: 0x");
        Serial.print((uint8_t)c, HEX);
        Serial.print(" ('");
        Serial.print(c);
        Serial.println("')");
        
        // 累积接收到的数据
        receivedData += c;
        
        // 如果收到换行符，处理完整的一行
        if (c == '\n') {
            // 移除行尾的\r\n
            String line = receivedData;
            line.replace("\r", "");
            line.replace("\n", "");
            
            Serial.print("[ESP32] Processing complete line: '");
            Serial.print(line);
            Serial.println("'");
            
            // 处理POST模式
            if (post_expect_len >= 0) {
                post_payload += line;
                if ((int)post_payload.length() >= post_expect_len) {
                    // 达到长度，发布
                    bool ok = client.publish(ALINK_TOPIC_PROP_POST, post_payload.c_str());
                    SerialSTM32.println(ok?"POST:OK":"POST:FAIL");
                    post_expect_len = -1;
                    post_payload = "";
                }
                receivedData = "";
                continue;
            }
            
            // 处理命令
            if (line == "STATUS?") {
                bool online = (WiFi.isConnected() && client.connected());
                Serial.print("[ESP32] Reply to STATUS?: ");
                Serial.println(online?"STATUS:ONLINE":"STATUS:OFFLINE");
                SerialSTM32.println(online?"STATUS:ONLINE":"STATUS:OFFLINE");
                markDataActivity();  // 标记数据交互
            } else if (line == "TIME?") {
                struct tm t; char buf[32];
                if (getLocalTime(&t, 1000)) {
                    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
                    SerialSTM32.print("TIME:"); SerialSTM32.println(buf);
                } else {
                    SerialSTM32.println("TIME:--");
                }
                markDataActivity();  // 标记数据交互
            } else if (line.startsWith("POST ")) {
                int len = line.substring(5).toInt();
                post_expect_len = len > 0 ? len : -1;
                post_payload = "";
                markDataActivity();  // 标记数据交互
            } else if (line.startsWith("DATA:")) {
                // 接收STM32发送的传感器数据JSON
                String jsonData = line.substring(5);
                Serial.print("[ESP32] Received DATA JSON: ");
                Serial.println(jsonData);
                
                DynamicJsonDocument doc(2048);
                DeserializationError error = deserializeJson(doc, jsonData);
                if (!error) {
                    if (doc.containsKey("GPS_zhuangtai")) sensorData.GPS_zhuangtai = doc["GPS_zhuangtai"];
                    if (doc.containsKey("LEIDA_zhuangtai")) sensorData.LEIDA_zhuangtai = doc["LEIDA_zhuangtai"];
                    if (doc.containsKey("WENDU")) sensorData.WENDU = doc["WENDU"];
                    if (doc.containsKey("SHIDU")) sensorData.SHIDU = doc["SHIDU"];
                    if (doc.containsKey("GPS_dingwei1")) sensorData.GPS_dingwei1 = doc["GPS_dingwei1"].as<String>();
                    if (doc.containsKey("GPS_dingwei2")) sensorData.GPS_dingwei2 = doc["GPS_dingwei2"].as<String>();
                    if (doc.containsKey("XINLV1")) sensorData.XINLV1 = doc["XINLV1"];
                    if (doc.containsKey("XINLV2")) sensorData.XINLV2 = doc["XINLV2"];
                    if (doc.containsKey("SHIJIAN1")) sensorData.SHIJIAN1 = doc["SHIJIAN1"].as<String>();
                    if (doc.containsKey("SHIJIAN2")) sensorData.SHIJIAN2 = doc["SHIJIAN2"].as<String>();
                    sensorData.dataReady = true;
                    SerialSTM32.println("DATA:OK");
                    Serial.println("[ESP32] Sensor data processed successfully");
                    markDataActivity();  // 标记数据交互
                } else {
                    SerialSTM32.println("DATA:ERROR");
                    Serial.print("[ESP32] JSON parse error: ");
                    Serial.println(error.c_str());
                }
            }
            
            receivedData = "";  // 清空接收缓冲区
        }
    }
}

// 获取当前时间字符串
String getCurrentTimeString() 
{
    struct tm timeinfo;
    char timeString[32];
    
    if (getLocalTime(&timeinfo, 1000)) {
        strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(timeString);
    } else {
        return String("--");
    }
}

// 上报传感器数据到阿里云
void publishSensorData() 
{
    if (!sensorData.dataReady) {
        return;  // 没有新数据，不上报
    }

    char param[2048];
    char jsonBuf[3072];

    // 构建JSON参数
    // 注意：阿里云bool类型用0/1表示，不是true/false
    snprintf(param, sizeof(param),
        "{\"GPS_zhuangtai\":%d,\"LEIDA_zhuangtai\":%d,\"WENDU\":%.2f,\"SHIDU\":%.2f,"
        "\"GPS_dingwei1\":\"%s\",\"GPS_dingwei2\":\"%s\","
        "\"XINLV1\":%d,\"XINLV2\":%d,"
        "\"SHIJIAN1\":\"%s\",\"SHIJIAN2\":\"%s\"}",
        sensorData.GPS_zhuangtai ? 1 : 0,
        sensorData.LEIDA_zhuangtai ? 1 : 0,
        sensorData.WENDU,
        sensorData.SHIDU,
        sensorData.GPS_dingwei1.c_str(),
        sensorData.GPS_dingwei2.c_str(),
        sensorData.XINLV1,
        sensorData.XINLV2,
        sensorData.SHIJIAN1.c_str(),
        sensorData.SHIJIAN2.c_str()
    );

    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);

    if (client.publish(ALINK_TOPIC_PROP_POST, jsonBuf)) {
        Serial.println("Sensor data published to Aliyun");
        sensorData.dataReady = false;  // 清除就绪标志
    } else {
        Serial.println("Sensor data publish failed");
    }
}

// MQTT消息回调函数
void callback(char* topic, byte* payload, unsigned int length) 
{
    ESP_LOGI(TAG, "========== MQTT MESSAGE ARRIVED ==========");
    ESP_LOGI(TAG, "Topic: %s", topic);
    ESP_LOGI(TAG, "Length: %u", length);
    
    // 打印完整的payload用于调试
    String payloadStr = "";
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    ESP_LOGI(TAG, "Payload: %s", payloadStr.c_str());
    
    // 打印原始字节（十六进制）
    ESP_LOGI(TAG, "Raw bytes:");
    for (unsigned int i = 0; i < length; i++) {
        ESP_LOGI(TAG, "  [%u] = 0x%02X '%c'", i, payload[i], (payload[i] >= 32 && payload[i] < 127) ? payload[i] : '.');
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        ESP_LOGE(TAG, "JSON parse error: %s", error.c_str());
        return;
    }

    // 阿里云物模型下发格式可能是 {"method":"thing.service.property.set","params":{...}}
    // 或者直接是 {"params":{...}}
    JsonObject params;
    if (doc.containsKey("params")) {
        params = doc["params"];
        ESP_LOGI(TAG, "Found 'params' key in JSON");
    } else {
        params = doc.as<JsonObject>();  // 直接使用根对象
        ESP_LOGI(TAG, "Using root object as params");
    }
    
    // 打印所有收到的参数键值
    ESP_LOGI(TAG, "Params content:");
    for (JsonPair kv : params) {
        ESP_LOGI(TAG, "  Key: '%s', Value: '%s'", kv.key().c_str(), kv.value().as<String>().c_str());
    }

    // 处理远程开关 (yuanchengkaiguan)
    if (params.containsKey("yuanchengkaiguan")) 
    {
        ESP_LOGI(TAG, ">>> Found 'yuanchengkaiguan' key!");
        
        // 阿里云可能发送 true/false 或 1/0 或 "1"/"0"
        JsonVariant val = params["yuanchengkaiguan"];
        ESP_LOGI(TAG, "Raw value type check - is<bool>:%d, is<int>:%d", val.is<bool>(), val.is<int>());
        
        if (val.is<bool>()) {
            light_status = val.as<bool>();
            ESP_LOGI(TAG, "Parsed as bool: %d", light_status);
        } else if (val.is<int>()) {
            light_status = (val.as<int>() != 0);
            ESP_LOGI(TAG, "Parsed as int: %d -> %d", val.as<int>(), light_status);
        } else {
            String strVal = val.as<String>();
            light_status = (strVal == "1" || strVal == "true");
            ESP_LOGI(TAG, "Parsed as string: '%s' -> %d", strVal.c_str(), light_status);
        }
        
        ESP_LOGI(TAG, "LED control: %s", light_status ? "ON (Blue)" : "OFF (Black)");

        if (light_status) {
            leds[0] = CRGB::Blue;
        } else {
            leds[0] = CRGB::Black;
        }
        FastLED.show();
        ESP_LOGI(TAG, "FastLED.show() called");
    } else {
        ESP_LOGW(TAG, "'yuanchengkaiguan' key NOT found in params");
    }

    // 处理蜂鸣器控制 (Buzzer)
    if (params.containsKey("Buzzer")) 
    {
        ESP_LOGI(TAG, ">>> Found 'Buzzer' key!");
        
        // 阿里云可能发送 true/false 或 1/0 或 "1"/"0"
        JsonVariant val = params["Buzzer"];
        bool buzzer_status;
        if (val.is<bool>()) {
            buzzer_status = val.as<bool>();
        } else if (val.is<int>()) {
            buzzer_status = (val.as<int>() != 0);
        } else {
            buzzer_status = (String(val.as<const char*>()) == "1" || String(val.as<const char*>()) == "true");
        }
        
        ESP_LOGI(TAG, "Buzzer control: %s", buzzer_status ? "ON" : "OFF");

        // 通过串口发送蜂鸣器控制命令到STM32
        if (buzzer_status) {
            SerialSTM32.println("BUZZER:ON");
            ESP_LOGI(TAG, "Sent BUZZER:ON to STM32");
        } else {
            SerialSTM32.println("BUZZER:OFF");
            ESP_LOGI(TAG, "Sent BUZZER:OFF to STM32");
        }
    } else {
        ESP_LOGW(TAG, "'Buzzer' key NOT found in params");
    }
    
    ESP_LOGI(TAG, "========== CALLBACK END ==========");
}

// 初始化设置
void setup() 
{
    Serial.begin(115200);  // 初始化串口通信
    SerialSTM32.begin(115200, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);  // 初始化STM32串口通信
    
    // 添加调试信息
    Serial.println("[ESP32] Setup completed");
    Serial.print("[ESP32] STM32 UART initialized: RX_PIN=");
    Serial.print(STM32_RX_PIN);
    Serial.print(", TX_PIN=");
    Serial.println(STM32_TX_PIN);
    Serial.println("[ESP32] Waiting for STM32 commands...");

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);  // 初始化LED
    FastLED.setBrightness(255);  // 设置LED亮度
    
    // 启动时闪烁LED 3次，确认硬件正常
    Serial.println("[ESP32] LED test - blinking 3 times");
    for(int i = 0; i < 3; i++) {
        leds[0] = CRGB::Blue;
        FastLED.show();
        delay(200);
        leds[0] = CRGB::Black;
        FastLED.show();
        delay(200);
    }
    Serial.println("[ESP32] LED test done");

    wifiInit();  // 初始化WiFi连接
    // 配置NTP，东八区
    configTime(8 * 3600, 0, "ntp.aliyun.com", "pool.ntp.org", "time.windows.com");
    client.setServer(MQTT_SERVER, MQTT_PORT);  // 设置MQTT服务器
    client.setCallback(callback);  // 设置MQTT消息回调函数
}

// 主循环
void loop() 
{
    // 检查WiFi连接状态（非阻塞，自动重连）
    wifiCheckAndReconnect();
    
    // 更新LED状态指示
    updateLedStatus();
    
    // 处理串口协议
    checkSerialData();

    // 每5秒执行一次的任务
    if (millis() - lastMs >= 5000) 
    {
        lastMs = millis();
        
        // 检查MQTT连接（非阻塞）
        mqttCheckConnect();
        
        // 只有在WiFi和MQTT都连接时才执行以下操作
        if (wifi_connected && client.connected()) {
            mqttIntervalPost();  // 发布MQTT数据
            publishSensorData(); // 上报传感器数据（如果有新数据）
        }
        
        // 定期发送时间数据给STM32（每5秒一次）
        String currentTime = getCurrentTimeString();
        Serial.print("[ESP32] Current time: ");
        Serial.println(currentTime);
        if (currentTime != "--" && currentTime.length() > 10) {
            SerialSTM32.print("TIME:");
            SerialSTM32.println(currentTime);
            Serial.print("[ESP32] Sent to STM32: TIME:");
            Serial.println(currentTime);
        } else {
            Serial.println("[ESP32] Time not ready yet (NTP sync pending)");
        }
        
        // 打印连接状态
        Serial.printf("[ESP32] Status: WiFi=%s, MQTT=%s\n", 
                      wifi_connected ? "OK" : "DISCONNECTED",
                      client.connected() ? "OK" : "DISCONNECTED");
    }
    
    // MQTT循环（只有连接时才有效）
    if (client.connected()) {
        client.loop();
    }
    
    delay(10);  // 延迟10ms
}