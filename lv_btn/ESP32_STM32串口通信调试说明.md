# ESP32与STM32串口通信调试说明

## 问题现象
从日志看，所有ESP32通信都超时：
- `[ESP32] Status: TIMEOUT`
- `[ESP32] Query time failed`
- `[ESP32] Data send FAIL (timeout or error)`

## 可能原因

### 1. 硬件连接问题（最可能）
**正确的连接方式：**
```
STM32 USART6          ESP32 UART1
PC6 (TX)  --------->  GPIO18 (RX)
PC7 (RX)  <---------  GPIO17 (TX)
GND       ----------  GND
```

**检查方法：**
- 用万用表测量连线是否正确
- 确认TX-RX交叉连接（不是TX-TX）
- 确认共地

### 2. ESP32端WiFi未连接
ESP32代码中WiFi配置：
```cpp
#define WIFI_SSID "wu"
#define WIFI_PASSWD "wu12345678"
```

**检查方法：**
- 确认WiFi名称和密码正确
- 查看ESP32串口监视器输出，确认WiFi连接状态
- ESP32的LED指示：
  - 红灯闪烁 = WiFi未连接
  - 黄灯闪烁 = WiFi已连接但MQTT未连接
  - 紫灯闪烁 = 正常运行

### 3. ESP32未正常启动
**检查方法：**
- 用USB连接ESP32到电脑
- 打开Arduino IDE串口监视器（115200波特率）
- 查看ESP32启动日志

## 调试步骤

### 步骤1：检查ESP32是否正常运行
1. 用USB连接ESP32到电脑
2. 打开串口监视器（115200波特率）
3. 应该看到类似输出：
   ```
   [ESP32] Setup completed
   [ESP32] STM32 UART initialized: RX_PIN=18, TX_PIN=17
   [ESP32] Waiting for STM32 commands...
   [ESP32] LED test - blinking 3 times
   ```

### 步骤2：检查WiFi连接
1. 在ESP32串口监视器中查看：
   ```
   Connecting WiFi... (20)
   Connecting WiFi... (19)
   ...
   WiFi Connected! IP: 192.168.x.x
   ```
2. 如果看到`WiFi connection timeout`，说明WiFi配置有问题

### 步骤3：检查串口通信
1. 在ESP32串口监视器中，当STM32发送命令时应该看到：
   ```
   [ESP32] RX: 0x53 ('S')
   [ESP32] RX: 0x54 ('T')
   ...
   [ESP32] Processing complete line: 'STATUS?'
   [ESP32] Reply to STATUS?: STATUS:ONLINE
   ```
2. 如果没有看到RX数据，说明硬件连接有问题

### 步骤4：手动测试串口
1. 断开STM32
2. 用USB转TTL模块连接ESP32的GPIO17/18
3. 发送`STATUS?\n`测试ESP32是否响应

## 快速修复建议

### 如果是WiFi问题
修改ESP32代码中的WiFi配置：
```cpp
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWD "你的WiFi密码"
```

### 如果是硬件连接问题
检查并重新连接：
- STM32 PC6 -> ESP32 GPIO18
- STM32 PC7 -> ESP32 GPIO17
- 共地

### 如果ESP32引脚定义错误
ESP32代码中的引脚定义是正确的：
```cpp
#define STM32_RX_PIN 18  // ESP32的RX引脚，接STM32的TX(PC6)
#define STM32_TX_PIN 17  // ESP32的TX引脚，接STM32的RX(PC7)
```

## 蜂鸣器控制问题

### 云端控制蜂鸣器
1. ESP32收到阿里云下发的`Buzzer`属性
2. ESP32通过串口发送`BUZZER:ON\n`或`BUZZER:OFF\n`到STM32
3. STM32调用`ESP32_Get_BuzzerCmd()`解析命令

**检查点：**
- 确认阿里云物模型中有`Buzzer`属性
- 确认ESP32的MQTT订阅正常
- 确认STM32定期调用`ESP32_Get_BuzzerCmd()`

### 本地按键控制蜂鸣器
这个需要检查STM32端的按键处理代码，与ESP32通信无关。

## 时间同步问题

ESP32通过NTP获取时间，然后主动发送给STM32：
```cpp
// 每5秒发送一次时间
SerialSTM32.print("TIME:");
SerialSTM32.println(currentTime);
```

**如果时间不更新：**
1. ESP32的WiFi未连接 -> NTP无法同步
2. 串口通信不通 -> STM32收不到TIME数据
