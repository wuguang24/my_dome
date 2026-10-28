# STM32 生命探测雷达搜救系统

基于 **STM32F407ZGT6 + FreeRTOS + LVGL** 的生命探测雷达搜救终端:通过雷达模块探测生命信号,结合 **GPS 定位、环境温湿度监测**,在 LCD 触摸屏上以图形化界面呈现;并由 **ESP32 物联网网关**接入**阿里云 IoT**,实现蜂鸣器远程控制与设备状态云端上报。

## 系统架构

```
┌──────────────┐   MQTT    ┌──────────────┐  UART 115200  ┌─────────────────────┐
│   阿里云 IoT  │ ←───────→ │    ESP32     │ ←───────────→ │      STM32F407ZGT6   │
│ (物模型/远程) │           │ (WiFi 网关)  │   RX=18 TX=17  │  FreeRTOS + LVGL GUI │
└──────────────┘           └──────────────┘               └─────────────────────┘
                                                            ├─ 雷达模块 (USART3)
                                                            ├─ GPS 模块  (UART4)
                                                            ├─ DHT11 温湿度
                                                            └─ LCD 触摸屏 + 蜂鸣器
```

## 功能特性

- 🎯 **雷达生命探测**:雷达模块经 USART3 接入,数据解析与状态监测
- 🛰️ **GPS 定位**:UART4 接入 GPS 模块,获取定位信息
- 🌡️ **环境监测**:DHT11 温湿度采集
- 🖥️ **LVGL 图形界面**:LCD 触摸屏多页面 UI(主界面 / SAR 搜救 / 故障诊断),SquareLine 生成
- 🧵 **FreeRTOS 多任务调度**:显示、传感器、通信等任务实时调度
- 📡 **ESP32 云端网关**:接入阿里云 IoT,上报 GPS/雷达/照片状态,下发蜂鸣器控制
- 🚨 **故障诊断**:设备异常检测与界面报警
- ⏱️ **通信超时检测**:ESP32→STM32 状态协议带 12s 超时判断

## 目录结构

```
├── lv_btn/                     # STM32F407 主工程(Keil MDK)
│   ├── USER/                   # main.c、app_tasks.c(FreeRTOS 任务)
│   ├── HARDWARE/               # 驱动:RADAR/GPS/ESP32/DHT11/LCD/TOUCH/BEEP/KEY...
│   ├── Middlewares/            # FreeRTOS + LVGL
│   ├── generated/              # SquareLine/LVGL 生成 UI(MAIN/SAR/Fault_Diagnosis)
│   ├── custom/                 # LVGL 自定义组件
│   ├── CORE/ HALLIB/ SYSTEM/   # 内核/HAL/系统
│   ├── ESP32_STM32串口通信协议文档.md
│   ├── 引脚接线文档.md / 项目分析汇总文档.md / 项目结构文档.md
│   └── LVGL.uvprojx            # Keil 工程入口
├── ESP32_usart/                # ESP32 网关程序(Arduino)
│   └── ESP_WIFI_USART/ESP_WIFI_USART.ino
├── ESP32_uart.c                # ESP32 代码副本
├── STM32F407ZGT6 V1.0_SCH.pdf  # 硬件原理图
```

## 快速开始

### STM32 端
1. Keil MDK 打开 `lv_btn/USER/LVGL.uvprojx`,编译下载至 STM32F407ZGT6
2. 外接:雷达模块(USART3)、GPS(UART4)、DHT11、LCD 触摸屏

### ESP32 网关
1. Arduino IDE 打开 `ESP32_usart/ESP_WIFI_USART/ESP_WIFI_USART.ino`
2. 修改阿里云设备三元组与 WiFi 配置,烧录至 ESP32
3. 与 STM32 交叉接线:STM32 TX → ESP32 RX(GPIO18),STM32 RX → ESP32 TX(GPIO17)

### ESP32 ↔ STM32 状态协议

| 命令 | 含义 |
|------|------|
| `zhaopian/N\r\n` | 发送照片数量 N |
| `zhuangtai0\r\n` | 置为异常状态 |
| `zhuangtai1\r\n` | 置为正常状态(仅异常态有效,12s 超时保持异常) |

## 引脚速查(STM32F407)

| 功能 | 引脚 | 说明 |
|------|------|------|
| 雷达模块 | PB10(TX)/PB11(RX) | USART3 @115200 |
| GPS 模块 | PC10(TX)/PC11(RX) | UART4 @9600 |
| 调试串口 | PA9(TX)/PA10(RX) | USART1 @115200 |
| DHT11 | PA5 | 单总线 |
| 蜂鸣器 | PG13 | 低电平有效 |
| 触摸屏 I2C | PB15(SCL)/PB12(SDA) | 400kHz |
| LED0/LED1 | PA1 / PA2 | 红灯 PWM / 绿灯 |

## 技术栈

`STM32F407ZGT6` · `FreeRTOS` · `LVGL 8` · `ESP32 (Arduino)` · `阿里云 IoT MQTT` · `GPS NMEA` · `Keil MDK`

## 详细文档

- [项目分析汇总文档](lv_btn/项目分析汇总文档.md)
- [项目结构文档](lv_btn/项目结构文档.md)
- [引脚接线文档](lv_btn/引脚接线文档.md)
- [ESP32-STM32 串口通信协议文档](lv_btn/ESP32_STM32串口通信协议文档.md)
- [ESP32-STM32 串口通信调试说明](lv_btn/ESP32_STM32串口通信调试说明.md)
