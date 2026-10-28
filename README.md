# STM32 + 阿里云 IoT 智能温室控制系统

基于 **STM32F103 + ESP8266 + 阿里云 IoT 平台** 的智能温室(大棚)环境控制系统,实现环境数据采集、本地自动控制与云端远程控制,涵盖**水泵灌溉、风扇通风、补光灯补光**三大执行器。

## 系统架构

```
┌──────────────┐   MQTT(JSON)   ┌──────────────┐   USART3(AT)  ┌──────────────┐
│   阿里云 IoT  │ ←────────────→ │    ESP8266    │ ←───────────→ │  STM32F103   │
│  (物模型/规则) │   属性上报/下发  │   (WiFi网关)  │    115200     │  (采集/控制)  │
└──────────────┘                └──────────────┘               └──────────────┘
```

## 功能特性

### 🌡️ 环境采集
- **DHT11** 温湿度采集
- **BH1750 (GY-302)** 光照强度采集(I2C)
- **土壤湿度**模拟量采集(ADC)
- **OLED** 实时显示(I2C)

### ⚙️ 执行器控制(水泵 / 风扇 / 补光灯)
- **自动模式**:根据目标阈值自动开关水泵、风扇、补光灯
- **手动模式**:云端(或按键)直接控制执行器
- **报警功能**:高温/低温/光照不足/土壤过干等组合报警,蜂鸣器提示

### ☁️ 云端功能(阿里云 MQTT + 物模型)
- 属性周期上报:传感器数据 + 执行器状态 + 模式/报警/阈值
- 属性下发控制:修改阈值、开关报警、切换自动/手动模式
- 手动模式下云端直接控制风扇/水泵/补光灯

### 🔘 本地交互
- 4 按键本地操作
- 红/绿指示灯状态指示

## 目录结构

```
├── User/                    # 主程序入口
├── my_lib/                  # 自建驱动库
│   ├── aliyun_conf.h        # ★ MQTT/WiFi 配置(需修改)
│   ├── usart3.c             # ESP8266 串口通信
│   ├── DHT11 / Soil / GY302 # 传感器驱动
│   ├── fan / bump / led_module  # 执行器驱动
│   ├── OLED / Key / Buzz    # 交互驱动
│   └── ...
├── library/                 # STM32 标准外设库
├── start/                   # 启动文件
├── Project.uvprojx          # Keil MDK 工程
└── docs/                    # 项目文档
    ├── 阿里云物模型创建与对接说明.md
    └── 项目引脚接线说明.md
```

## 快速开始

### 1. 云平台配置

1. 在[阿里云物联网平台](https://iot.aliyun.com/)创建产品(Wi-Fi 联网、JSON 数据格式),记录 `ProductKey`
2. 产品下添加设备,记录三元组 `ProductKey / DeviceName / DeviceSecret`
3. 按物模型说明创建属性(标识符需与上报字段一致),见 `docs/阿里云物模型创建与对接说明.md`

### 2. 修改工程配置

编辑 `my_lib/aliyun_conf.h`:

```c
#define ALIYUN_WIFI_SSID   "你的WiFi名称"
#define ALIYUN_WIFI_PASS   "你的WiFi密码"
#define ALIYUN_MQTT_HOST   "xxx.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define ALIYUN_MQTT_USER   "平台计算的username"
#define ALIYUN_MQTT_PASS   "平台计算的password"
#define ALIYUN_MQTT_CLIENTID "平台计算的clientId"
```

### 3. 编译烧录

使用 Keil MDK 打开 `Project.uvprojx`,编译下载至 STM32F103C8T6。

## 核心控制逻辑

| 模式 | 控制来源 |
|------|---------|
| `auto_mode = 1`(自动) | 本地根据阈值自动控制,云端执行器指令不生效 |
| `auto_mode = 0`(手动) | 云端下发的 `fan/pump/lamp` 指令直接生效 |

## 引脚速查

| 模块 | 引脚 | 说明 |
|------|------|------|
| ESP8266 | PB10(TX)/PB11(RX) | USART3 @115200 |
| DHT11 | PA8 | 单总线 |
| 土壤湿度 | PA0 | ADC1 通道 0 |
| BH1750 光照 | PB6(SCL)/PB7(SDA) | 软件 I2C |
| OLED | PB8(SCL)/PB9(SDA) | 软件 I2C |
| 风扇 | PB0 | — |
| 水泵 | PB1 | — |
| 补光灯 | PA3 | — |
| 蜂鸣器 | PB14 | — |
| 按键 KEY1~KEY4 | PA6 / PA11 / PA15 / PA1 | 上拉输入 |

## 技术栈

`STM32F103C8T6` · `标准外设库 (SPL)` · `ESP8266 AT 指令` · `MQTT` · `阿里云 IoT 物模型` · `Keil MDK`

## 详细文档

- [阿里云物模型创建与对接说明](docs/阿里云物模型创建与对接说明.md)
- [项目引脚接线说明](docs/项目引脚接线说明.md)
