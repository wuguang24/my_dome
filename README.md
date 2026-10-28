# my_dome — 嵌入式项目合集

本仓库以**独立分支**的形式收录我的嵌入式/物联网项目合集,每个分支是一个完整可编译的工程。切换分支即可查看对应项目。

## 📚 项目导航(切换分支查看)

| 分支 | 项目 | 简介 | 技术栈 |
|------|------|------|--------|
| [`esp32s3-smart-tool-cabinet`](https://github.com/wuguang24/my_dome/tree/esp32s3-smart-tool-cabinet) | ESP32-S3 智能工具柜管理系统 | OneNet 云平台 + 微信小程序 + RFID 工具借还管理、远程开锁、超时/非法开门报警 | ESP32-S3 · MQTT · MFRC522 · 微信小程序 |
| [`stm32-smart-agriculture`](https://github.com/wuguang24/my_dome/tree/stm32-smart-agriculture) | STM32 智能温室控制系统 | 水泵灌溉/风扇通风/补光灯,自动+手动双模式,阈值报警 | STM32F103 · ESP8266 · 阿里云 IoT |
| [`stm32-voice-aromatherapy-lamp`](https://github.com/wuguang24/my_dome/tree/stm32-voice-aromatherapy-lamp) | STM32 语音控制智能香薰灯 | ASR 语音指令 + PWM 调光 + 加湿档位 + 缺水保护 | STM32F103 · PWM · 串口协议 |
| [`stm32-greenhouse-env-monitor`](https://github.com/wuguang24/my_dome/tree/stm32-greenhouse-env-monitor) | STM32 大棚环境监测与控制系统 | 温湿度/光照/CO₂ 采集,滞回控制,云端远程接管 | STM32F103 · ESP8266 · OneNet |
| [`stm32-life-detection-radar`](https://github.com/wuguang24/my_dome/tree/stm32-life-detection-radar) | STM32 生命探测雷达搜救系统 | 雷达探测 + GPS 定位 + LVGL 触摸屏 GUI + 阿里云上报 | STM32F407 · FreeRTOS · LVGL · ESP32 |
| [`mspm0g3507-smart-car`](https://github.com/wuguang24/my_dome/tree/mspm0g3507-smart-car) | MSPM0G3507 差速智能小车 | 蓝牙遥控 + 红外巡线 + MPU6050 姿态 + 超声波近距保护 | MSPM0G3507 · DriverLib · DMP |

## 🚀 快速使用

```bash
# 克隆指定项目(以智能小车为例)
git clone -b mspm0g3507-smart-car https://github.com/wuguang24/my_dome.git smart-car

# 或克隆后切换分支
git clone https://github.com/wuguang24/my_dome.git
cd my_dome
git checkout stm32-life-detection-radar
```

每个分支根目录都有详细的 `README.md`(系统架构、引脚接线表、快速开始教程)与中文文档。

> 主分支 `main` 仅作为目录导航页,不含项目代码。

---

*wuguang24 · 嵌入式项目合集*
