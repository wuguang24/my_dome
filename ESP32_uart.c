#include <Arduino.h>  // 引入Arduino库
#include <WiFi.h>  // 引入WiFi库
#include "PubSubClient.h"  // 引入MQTT库
#include <ArduinoJson.h>  // 引入Arduino JSON库
#include "FastLED.h"  // 引入FastLED库（用于控制LED）
#include <time.h>
#include "mbedtls/md.h"

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
const char* TOPIC = "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set";  // 下行属性设置

// 全局变量
WiFiClient espClient;  // 创建WiFi客户端
PubSubClient client(espClient);  // 创建MQTT客户端
unsigned long lastMs = 0;  // 定时器
String serialLine;         // 串口行缓存
int post_expect_len = -1;  // POST 负载长度，-1表示未进入POST接收
String post_payload;       // POST接收缓存
bool light_status = false; // Demo: LED指示

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

// 初始化WiFi连接
void wifiInit() 
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);  // 开始连接WiFi
    while (WiFi.status() != WL_CONNECTED)
     {  // 如果WiFi未连接
        delay(1000);  // 延迟1秒
        Serial.println("Connecting WiFi...");  // 输出连接WiFi信息
    }
    Serial.println("WiFi Connected!");  // 输出WiFi连接成功信息
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

void mqttCheckConnect() 
{
    while (!client.connected()) 
    {  // 如果MQTT客户端未连接
        Serial.println("Connecting MQTT...");
        char cid[128], usr[96], pwd[96];
        build_mqtt_params(cid, sizeof(cid), usr, sizeof(usr), pwd, sizeof(pwd));
        if (client.connect(cid, usr, pwd)) 
        {  // 尝试连接MQTT
            Serial.println("MQTT Connected!");  // 输出连接成功信息
            client.subscribe(TOPIC);  // 订阅主题
        } else 
        {
            Serial.print("MQTT Connect failed, state:");  // 输出连接失败信息
            Serial.println(client.state());
            delay(5000);  // 延迟5秒后重试
        }
    }
}

// 定时发送MQTT状态
void mqttIntervalPost() 
{ 
    char param[256];  // 定义参数字符串
    char jsonBuf[1024];  // 定义JSON格式的字符串

    // Demo: 周期性发布在线状态
    sprintf(param, "{\"online\":%s}", WiFi.isConnected() && client.connected()?"true":"false");

    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);  // 格式化为阿里云数据格式

    if (client.publish(ALINK_TOPIC_PROP_POST, jsonBuf)) 
    {  // 发布消息到阿里云
        Serial.println("Status and zhaopian published");
    } else
    {
        Serial.println("Publish failed");  // 发布失败时输出信息
    }
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
            } else if (line == "TIME?") {
                struct tm t; char buf[32];
                if (getLocalTime(&t, 1000)) {
                    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
                    SerialSTM32.print("TIME:"); SerialSTM32.println(buf);
                } else {
                    SerialSTM32.println("TIME:--");
                }
            } else if (line.startsWith("POST ")) {
                int len = line.substring(5).toInt();
                post_expect_len = len > 0 ? len : -1;
                post_payload = "";
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

    // 构建JSON参数（转义字符串中的引号）
    snprintf(param, sizeof(param),
        "{\"GPS_zhuangtai\":%s,\"LEIDA_zhuangtai\":%s,\"WENDU\":%.2f,\"SHIDU\":%.2f,"
        "\"GPS_dingwei1\":\"%s\",\"GPS_dingwei2\":\"%s\","
        "\"XINLV1\":%d,\"XINLV2\":%d,"
        "\"SHIJIAN1\":\"%s\",\"SHIJIAN2\":\"%s\"}",
        sensorData.GPS_zhuangtai ? "true" : "false",
        sensorData.LEIDA_zhuangtai ? "true" : "false",
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
    Serial.print("Message arrived [");  // 输出消息到达的信息
    Serial.print(topic);
    Serial.print("] ");

    DynamicJsonDocument doc(1024);  // 创建一个JSON文档对象
    deserializeJson(doc, payload, length);  // 解析收到的JSON数据

    // 处理远程开关（保留原有功能）
    if (doc["params"].containsKey("yuanchengkaiguan")) 
    {  // 如果参数中包含"yuanchengkaiguan"
        light_status = doc["params"]["yuanchengkaiguan"];  // 获取灯的状态
        Serial.print("Light status changed to: ");
        Serial.println(light_status);

        if (light_status) 
        {  // 如果灯的状态是开
            leds[0] = CRGB::Blue;  // 设置LED为蓝色
        } else 
        {  // 如果灯的状态是关
            leds[0] = CRGB::Black;  // 设置LED为黑色（关）
        }
        FastLED.show();  // 更新LED状态
    }

    // 处理蜂鸣器控制（新增功能）
    if (doc["params"].containsKey("Buzzer")) 
    {
        bool buzzer_status = doc["params"]["Buzzer"];
        Serial.print("Buzzer command received: ");
        Serial.println(buzzer_status ? "ON" : "OFF");

        // 通过串口发送蜂鸣器控制命令到STM32
        if (buzzer_status) {
            SerialSTM32.println("BUZZER:ON");
        } else {
            SerialSTM32.println("BUZZER:OFF");
        }
    }
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
    leds[0] = CRGB::Black;  // 设置LED为关
    FastLED.show();  // 更新LED显示

    wifiInit();  // 初始化WiFi连接
    // 配置NTP，东八区
    configTime(8 * 3600, 0, "ntp.aliyun.com", "pool.ntp.org", "time.windows.com");
    client.setServer(MQTT_SERVER, MQTT_PORT);  // 设置MQTT服务器
    client.setCallback(callback);  // 设置MQTT消息回调函数
}

// 主循环
void loop() 
{
    checkSerialData();  // 处理串口协议

    if (millis() - lastMs >= 5000) 
    {  // 每5秒执行一次
        lastMs = millis();  // 更新时间
        mqttCheckConnect();  // 检查MQTT连接
        mqttIntervalPost();  // 发布MQTT数据
        publishSensorData(); // 上报传感器数据（如果有新数据）
        
        // 定期发送时间数据给STM32（每5秒一次）
        String currentTime = getCurrentTimeString();
        if (currentTime != "--") {
            SerialSTM32.print("TIME:");
            SerialSTM32.println(currentTime);
            Serial.print("[ESP32] Auto-send time to STM32: ");
            Serial.println(currentTime);
        }
    }
    client.loop();  // 执行MQTT循环
    delay(10);  // 延迟10ms
}