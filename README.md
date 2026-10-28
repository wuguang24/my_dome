# MSPM0G3507 差速智能小车

基于 **MSPM0G3507 + TI DriverLib + SysConfig + Keil** 的差速驱动智能小车，支持蓝牙遥控、红外巡线、超声波近距保护与 MPU6050 姿态解算。

`MSPM0G3507` · `TI DriverLib` · `SysConfig` · `Keil MDK` · `MPU6050 DMP` · `HC-SR04` · `红外巡线` · `蓝牙遥控`

## 功能概览

| 功能 | 状态 | 说明 |
|------|------|------|
| 蓝牙/APP 遥控 | 可用 | Mode=0，支持方向控制、自转、速度调节 |
| 红外巡线 | 可用 | Mode=1，参考官方例程的纯离散状态机 |
| 超声波测距 | 可用 | 主循环 100ms 周期测距，用于蓝牙近距保护 |
| 蓝牙近距保护 | 可用 | 前方 < 10cm 时自动禁止前进 |
| MPU6050 姿态 | 可用 | DMP 中断更新 pitch/roll/yaw |
| 惯性导航 | 可选 | 三击按键开关，仅蓝牙模式有直线航向保持 |
| 舵机摆头跟随 | 可用 | 蓝牙/巡线时跟随转向摆头 |
| OLED 显示 | 可用 | 模式、传感器状态、速度、电压 |
| 串口调试输出 | 可用 | 距离、姿态、系统状态、电机数据 |

## 快速开始

1. 用 Keil 打开 `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx`
2. 编译下载（F7 + F8）
3. 串口连接 UART0（PA10/PA11），波特率 115200
4. 单击按键启动小车，双击切换模式

详细说明见 `小车控制说明.md` 和 `Docs/` 目录。

## 模式说明

- **Mode=0（APP）**：蓝牙遥控模式
- **Mode=1（IRF）**：红外巡线模式

避障模式已移除。

## 按键操作

| 操作 | 功能 |
|------|------|
| 单击 | 启动/停车（Flag_Stop 切换） |
| 双击 | 切换模式（0 <-> 1） |
| 三击 | 惯性导航开关（Odom_Enable 切换） |

切换模式时自动解除停车（Flag_Stop=0）并舵机回中。

## 项目结构

```text
./
  empty.c              # 主入口、主循环、超声波测距、模式切换
  empty.syscfg         # SysConfig 配置源
  ti_msp_dl_config.*   # SysConfig 生成文件
  Hardware/            # 硬件驱动层
    board.c/.h         # 延时、时基、舵机偏移宏
    hcsr04.c/.h        # 超声波阻塞式测距
    IR_Module.c/.h     # 红外巡线（纯离散状态机）
    encoder.c/.h       # 编码器中断 + MPU6050 INT
    motor.c/.h         # 电机 PWM
    MPU6050.c/.h       # MPU6050 驱动
    oled.c/.h          # OLED 显示（软件 I2C）
    key.c/.h           # 按键扫描
    led.c/.h           # LED 控制
    adc.c/.h           # 电压采样
    bsp_iic.c/.h       # I2C 适配层
    ...
  Control/             # 控制算法层
    control.c/.h       # 200Hz 控制环、PI 控制、蓝牙解析
    show.c/.h          # OLED 页面、APP 输出
    uart_callback.c/.h # 蓝牙 DMA 接收与解析
    DataScope_DP.C/.h  # 虚拟示波器
  Docs/                # 文档
    01_使用操作文档.md
    02_项目文件说明.md
    03_功能实现说明.md
  keil/                # Keil 工程与编译输出
  tools/keil/          # SysConfig 菜单导入与低功耗补丁
  ti/                  # TI MSPM0 SDK DriverLib 源码
  蓝牙巡线/            # 官方参考例程
```

## 重要参数

| 参数 | 值 | 位置 |
|------|-----|------|
| 控制频率 | 200Hz（5ms） | control.h |
| 巡线基础速度 | RC_Velocity/1000 | IR_Module.c |
| 蓝牙近距阈值 | 10cm | empty.c |
| 舵机偏移补偿 | +25 | board.h |
| PID 参数 | KP=400, KI=300 | control.c |
| 直角弯转向 | 2.3 rad/s | IR_Module.c |
| 大弯转向 | 1.03 rad/s | IR_Module.c |
| 中等转向 | 0.50 rad/s | IR_Module.c |
| 微调转向 | 0.25 rad/s | IR_Module.c |

## 引脚速查

| 模块 | 信号 | 引脚 |
|------|------|------|
| MPU6050 | SDA | PA0 |
| MPU6050 | SCL | PA1 |
| MPU6050 | INT | PA7 |
| HC-SR04 | Trig | PA24 |
| HC-SR04 | Echo | PA9 |
| 舵机 | PWM | PA8 |
| 红外 DH1 | GPIO | PA27 |
| 红外 DH2 | GPIO | PA12 |
| 红外 DH3 | GPIO | PA16 |
| 红外 DH4 | GPIO | PA17 |
| 编码器 E1A | GPIO | PA25 |
| 编码器 E1B | GPIO | PA26 |
| 编码器 E2A | GPIO | PB20 |
| 编码器 E2B | GPIO | PB24 |
| LED | GPIO | PB9 |
| UART0 TX | 调试串口 | PA10 |
| UART0 RX | 调试串口 | PA11 |
| UART1 | 蓝牙模块 | 见 syscfg |

## 串口日志

波特率 115200，关键输出：

- `[DIST] raw=xx.xcm, Dist=xxx ...` — 超声波测距
- `[LINE] DH=x ...` — 红外巡线状态
- `[MPU] pitch=.. roll=.. yaw=..` — 姿态
- `[SYS] ... mode=... stop=...` — 系统状态
- `[BT] ...` — 蓝牙控制状态
- `[ENC] ...` — 编码器速度
- `[PWM] ...` — 电机 PWM
- `[KEY] ...` — 按键事件

## 版本记录

- **v3.0** (2026-04-25): 移除避障模式，巡线改为纯红外例程方式，蓝牙增加 10cm 近距保护，更新文档
- **v2.x**: 惯性导航集成（后从巡线中移除）、超声波修复、编码器方向修正、舵机偏移补偿
- **v1.x**: 基础蓝牙遥控与巡线
