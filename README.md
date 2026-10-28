# ESP32-S3 智能工具柜管理系统

基于 **ESP32-S3 + OneNet 云平台 + 微信小程序** 的智能工具柜管理系统,实现工具的智能化借还管理、远程开锁与安全监控。

## 系统架构

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  微信小程序  │ ←→  │  OneNet云平台 │ ←→  │   ESP32设备  │
│  (用户端)   │     │  (MQTT通信)  │     │  (硬件控制)  │
└─────────────┘     └─────────────┘     └─────────────┘
```

## 功能特性

- 🔐 **用户管理**:用户注册/登录(8 位数字账号 + 8 位密码),账号云端同步,支持修改密码
- 🔧 **工具状态实时显示**:工具列表状态云端实时查询(位图标识 6 件工具)
- 📱 **远程开锁借还工具**:小程序远程下发开锁指令,借用/归还一键操作
- 🏷️ **RFID 工具识别确认**:MFRC522 读取工具标签,借还自动确认
- ⏰ **借用超时报警**:超时(默认 1 小时)蜂鸣器报警
- 🚨 **非法开柜报警**:门磁传感器检测非法开门立即报警
- 📋 **借还记录查询**:小程序查看历史借还记录
- 🖥️ **本地人机交互**:OLED 显示 + 旋转编码器菜单操作

## 硬件组成

| 元器件 | 型号/规格 | 说明 |
|--------|----------|------|
| 主控板 | ESP32-S3 | 带 WiFi/蓝牙 |
| OLED 显示屏 | SSD1306 128x64 | I2C 接口 |
| RFID 模块 | MFRC522 | SPI 接口 |
| 电磁锁 | 12V 通电解锁型 | 继电器控制 |
| 门磁传感器 | 常闭型 | 检测柜门状态 |
| 蜂鸣器 | 有源 5V | 低电平触发 |
| 旋转编码器 | EC11 | 带按键 |
| RFID 标签 | 13.56MHz | 贴在工具上 |

## 目录结构

```
├── SmartToolCabinet/          # ESP32-S3 固件(PlatformIO 工程)
│   ├── include/               # 头文件(config.h 配置 WiFi/OneNet)
│   ├── src/                   # 源文件(main.cpp 主程序、MQTT 通信、存储等)
│   ├── lib/                   # 依赖库
│   └── platformio.ini         # PlatformIO 配置
├── pages/                     # 微信小程序页面
│   ├── index/                 # 工具列表页
│   ├── login/                 # 登录注册页
│   ├── profile/               # 个人中心页
│   ├── records/               # 借还记录页
│   ├── detail/                # 工具详情页
│   └── ...
├── app.js                     # 小程序入口(配置 OneNet API)
├── app.json                   # 小程序配置
└── thingmodel-json-import - 副本.json  # OneNet 物模型定义
```

## 快速开始

### 1. 云平台配置

1. 注册 [OneNet 账号](https://open.iot.10086.cn/),创建 MQTT 产品并添加设备
2. 进入产品 → 物模型定义 → 导入物模型(选择 `thingmodel-json-import - 副本.json`)并发布
3. 记录 **产品 ID / 设备名称 / 鉴权 Token**

### 2. ESP32 固件烧录

1. 修改 `SmartToolCabinet/include/config.h`:填入 WiFi 名称密码、OneNet 产品 ID / 设备名 / Token
2. 使用 VS Code + PlatformIO 插件打开 `SmartToolCabinet` 目录:

```bash
cd SmartToolCabinet
pio run -t upload       # 编译并烧录
pio device monitor      # 查看串口输出
```

### 3. 微信小程序部署

1. 安装 [微信开发者工具](https://developers.weixin.qq.com/miniprogram/dev/devtools/download.html),注册小程序账号获取 AppID
2. 导入项目根目录,修改 `app.js` 中的 `globalData`(产品 ID / 设备名 / API 鉴权 Token)
3. 在微信公众平台配置 request 合法域名:`https://iot-api.heclouds.com`
4. 预览扫码测试,或上传发布

> 默认登录账号:`10000001`,密码:`12345678`

## 技术栈

`ESP32-S3` · `PlatformIO/Arduino` · `MQTT (PubSubClient)` · `MFRC522 RFID` · `U8g2 OLED` · `SPIFFS 存储` · `微信小程序` · `OneNet 物联网平台`

## 详细文档

完整的引脚接线、物模型定义、代码部署教程与常见问题请查看:[项目说明文档.md](项目说明文档.md)
