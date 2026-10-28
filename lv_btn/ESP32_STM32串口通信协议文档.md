# ESP32-STM32 串口通信协议文档

## 📋 项目概述

- **项目名称**: ESP32物联网网关 + STM32传感器节点
- **通信方式**: UART串口 (115200波特率, 8N1)
- **云平台**: 阿里云IoT平台
- **ESP32引脚**: RX=18, TX=17

---

## 🔌 硬件连接

```
STM32 TX  →  ESP32 RX (GPIO 18)
STM32 RX  →  ESP32 TX (GPIO 17)
GND       →  GND
```

---

## 📡 阿里云IoT配置

### 设备三元组
```
ProductKey:   a1xT9S4rOnA
DeviceName:   shebei2025
DeviceSecret: e7b3f398b8d55879929e6bba83a16931
Region:       cn-shanghai
```

### MQTT连接信息
```
Server: a1xT9S4rOnA.iot-as-mqtt.cn-shanghai.aliyuncs.com
Port:   1883
认证方式: HMAC-SHA256动态签名
```

### 物模型属性

#### 订阅属性 (云端→设备)
| 属性标识符 | 数据类型 | 说明 | 取值 |
|-----------|---------|------|------|
| Buzzer | bool | 蜂鸣器控制 | 0-关闭, 1-开启 |

#### 上报属性 (设备→云端)
| 属性标识符 | 数据类型 | 说明 | 取值范围 |
|-----------|---------|------|---------|
| GPS_zhuangtai | bool | GPS状态 | 0-异常, 1-正常 |
| LEIDA_zhuangtai | bool | 雷达状态 | 0-异常, 1-正常 |
| WENDU | float | 温度 | 0~100 |
| SHIDU | float | 湿度 | 0~100 |
| GPS_dingwei1 | text | GPS定位1 | 字符串(最大10240) |
| GPS_dingwei2 | text | GPS定位2 | 字符串(最大10240) |
| XINLV1 | int32 | 心率1 | 0~200 |
| XINLV2 | int32 | 心率2 | 0~200 |
| SHIJIAN1 | text | 时间1 | 字符串(最大10240) |
| SHIJIAN2 | text | 时间2 | 字符串(最大10240) |

---

## 📤 STM32 → ESP32 通信协议

### 1. 传感器数据上报 (推荐使用)

**命令格式**:
```
DATA:<JSON字符串>\n
```

**JSON字段**:
```json
{
  "GPS_zhuangtai": true,           // bool: GPS状态
  "LEIDA_zhuangtai": true,         // bool: 雷达状态
  "WENDU": 25.5,                   // float: 温度
  "SHIDU": 60.0,                   // float: 湿度
  "GPS_dingwei1": "31.2304,121.4737",  // string: GPS定位1
  "GPS_dingwei2": "31.2305,121.4738",  // string: GPS定位2
  "XINLV1": 75,                    // int: 心率1
  "XINLV2": 80,                    // int: 心率2
  "SHIJIAN1": "2025-12-02 11:20:00",   // string: 时间1
  "SHIJIAN2": "2025-12-02 11:21:00"    // string: 时间2
}
```

**完整示例**:
```
DATA:{"GPS_zhuangtai":true,"LEIDA_zhuangtai":true,"WENDU":25.5,"SHIDU":60.0,"GPS_dingwei1":"31.2304,121.4737","GPS_dingwei2":"31.2305,121.4738","XINLV1":75,"XINLV2":80,"SHIJIAN1":"2025-12-02 11:20:00","SHIJIAN2":"2025-12-02 11:21:00"}
```

**ESP32回复**:
- 成功: `DATA:OK\n`
- 失败: `DATA:ERROR\n`

**说明**:
- ESP32收到数据后会缓存，每5秒自动上报到阿里云
- 建议STM32每5-10秒发送一次，避免过于频繁

---

### 2. 查询ESP32在线状态

**命令格式**:
```
STATUS?\n
```

**ESP32回复**:
- WiFi和MQTT都连接: `STATUS:ONLINE\n`
- 未连接: `STATUS:OFFLINE\n`

**使用场景**: STM32需要确认ESP32网络状态时

---

### 3. 查询网络时间

**命令格式**:
```
TIME?\n
```

**ESP32回复**:
- 时间已同步: `TIME:2025-12-02 11:30:00\n`
- 时间未同步: `TIME:--\n`

**使用场景**: STM32需要获取准确的网络时间

---

### 4. 直接发布自定义数据 (高级用法)

**命令格式**:
```
POST <数据长度>\n
<JSON数据内容>
```

**示例**:
```
POST 45
{"temperature":25.5,"humidity":60.0}
```

**ESP32回复**:
- 成功: `POST:OK\n`
- 失败: `POST:FAIL\n`

**说明**: 
- 直接发布到阿里云，不经过数据缓存
- 适合发送自定义格式的数据

---

## 📥 ESP32 → STM32 通信协议

### 蜂鸣器控制命令

**命令格式**:
```
BUZZER:ON\n    // 打开蜂鸣器
BUZZER:OFF\n   // 关闭蜂鸣器
```

**触发条件**: 阿里云平台下发Buzzer属性设置

**STM32处理**: 
- 收到 `BUZZER:ON` → 打开蜂鸣器
- 收到 `BUZZER:OFF` → 关闭蜂鸣器

---

## 💻 STM32端示例代码

### 发送传感器数据

```c
void sendSensorDataToESP32(void) {
    char jsonBuffer[512];
    
    // 构建JSON字符串
    sprintf(jsonBuffer, 
        "DATA:{\"GPS_zhuangtai\":%s,\"LEIDA_zhuangtai\":%s,"
        "\"WENDU\":%.2f,\"SHIDU\":%.2f,"
        "\"GPS_dingwei1\":\"%s\",\"GPS_dingwei2\":\"%s\","
        "\"XINLV1\":%d,\"XINLV2\":%d,"
        "\"SHIJIAN1\":\"%s\",\"SHIJIAN2\":\"%s\"}",
        gps_status ? "true" : "false",
        radar_status ? "true" : "false",
        temperature,
        humidity,
        gps_position1,
        gps_position2,
        heartrate1,
        heartrate2,
        time_string1,
        time_string2
    );
    


## 🔄 完整工作流程

```
┌─────────────────────────────────────────────────────────────┐
│                        工作流程图                             │
└─────────────────────────────────────────────────────────────┘

STM32采集传感器数据
    ↓
每5秒发送: DATA:{...JSON...}
    ↓
ESP32接收并解析 → 回复: DATA:OK
    ↓
ESP32缓存数据 (sensorData.dataReady = true)
    ↓
ESP32每5秒检查 → 如有新数据则上报到阿里云
    ↓
阿里云IoT平台接收数据
    ↓
用户在阿里云控制台/APP设置Buzzer属性
    ↓
阿里云下发MQTT消息到ESP32
    ↓
ESP32解析并转发: BUZZER:ON 或 BUZZER:OFF
    ↓
STM32接收命令并控制蜂鸣器
```

---

## ⚠️ 重要注意事项

### 数据格式要求
1. ✅ **所有命令必须以换行符 `\n` 结尾**
2. ✅ **JSON字符串中的字符串值必须用双引号包裹**
3. ✅ **布尔值使用 `true`/`false`，不要用 `1`/`0`**
4. ✅ **浮点数建议保留2位小数**
5. ✅ **整数直接使用数字，不加引号**

### 性能建议
- 📊 STM32发送频率: 建议5-10秒一次
- 📊 ESP32上报频率: 自动每5秒上报一次
- 📊 串口波特率: 115200 (8N1)
- 📊 JSON缓冲区: 建议512字节

### 字符串长度限制
- GPS定位字符串: 最大10240字节 (建议<100字节)
- 时间字符串: 建议使用标准格式 "YYYY-MM-DD HH:MM:SS"
- JSON总长度: 当前缓冲区2048字节

### 错误处理
- 如果ESP32回复 `DATA:ERROR`，说明JSON解析失败，检查格式
- 如果无回复，检查串口连接和波特率设置
- 如果数据未上报到云端，检查WiFi和MQTT连接状态

---

## 🧪 测试方法

### 1. 串口调试工具测试
使用串口助手连接ESP32的UART1 (RX=18, TX=17)，发送测试命令：

```
STATUS?
TIME?
DATA:{"GPS_zhuangtai":true,"LEIDA_zhuangtai":false,"WENDU":25.5,"SHIDU":60.0,"GPS_dingwei1":"31.23,121.47","GPS_dingwei2":"31.24,121.48","XINLV1":75,"XINLV2":80,"SHIJIAN1":"2025-12-02 11:30:00","SHIJIAN2":"2025-12-02 11:31:00"}
```

### 2. 阿里云控制台测试
1. 登录阿里云IoT平台
2. 进入设备管理 → shebei2025
3. 查看设备属性，确认数据是否上报
4. 在线调试 → 设置Buzzer属性为1，观察STM32是否收到命令

---

## 📞 故障排查

| 问题 | 可能原因 | 解决方法 |
|------|---------|---------|
| ESP32无回复 | 串口连接错误 | 检查TX/RX是否交叉连接 |
| DATA:ERROR | JSON格式错误 | 检查引号、逗号、括号是否正确 |
| 数据未上报云端 | WiFi/MQTT断开 | 发送STATUS?查询连接状态 |
| 时间显示-- | NTP未同步 | 等待1-2分钟让ESP32同步时间 |
| 蜂鸣器无响应 | 未订阅MQTT主题 | 检查ESP32是否成功连接MQTT |

---

## 📝 版本历史

- **v1.0** (2025-12-02)
  - 初始版本
  - 支持10个传感器属性上报
  - 支持蜂鸣器控制订阅
  - 支持STATUS/TIME查询
  - 支持POST自定义数据

---

## 📧 技术支持

如有问题，请检查：
1. ESP32串口监视器输出的调试信息
2. 阿里云IoT平台的设备日志
3. STM32的UART配置是否正确

---

**文档生成时间**: 2025-12-02  
**适用固件**: ESP32_uart.c  
**阿里云设备**: shebei2025 (a1xT9S4rOnA)
