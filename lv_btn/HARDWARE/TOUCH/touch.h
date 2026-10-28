/*
 * 模块名称: 触摸屏驱动 (touch)
 * 功能概述:
 *   - 支持电阻式和电容式触摸屏
 *   - 电阻屏: 使用SPI接口读取ADC值，需要校准
 *   - 电容屏: 使用IIC接口，支持多点触控
 *   - 自动识别触摸屏类型并初始化
 * 
 * 支持的触摸IC:
 *   - 电容屏: GT1151, GT9147, FT5206, OTT2001A
 *   - 电阻屏: XPT2046等
 * 
 * 硬件连接(电阻屏):
 *   - T_PEN:  PB13 (触摸中断，低电平有效)
 *   - T_MISO: PB11 (SPI数据输出)
 *   - T_MOSI: PB12 (SPI数据输入)
 *   - T_SCK:  PB15 (SPI时钟)
 *   - T_CS:   PB14 (片选)
 * 
 * 硬件连接(电容屏):
 *   - CT_SCL: PB15 (IIC时钟)
 *   - CT_SDA: PB12 (IIC数据)
 *   - CT_INT: 触摸中断
 *   - CT_RST: 复位
 * 
 * 数据结构:
 *   - tp_dev: 触摸屏控制结构体，包含坐标、状态、校准参数
 *   - 支持最多5点触控(电容屏)
 * 
 * 使用方法:
 *   1. 调用 TP_Init() 初始化(自动识别类型)
 *   2. 调用 tp_dev.scan(0) 扫描触摸状态
 *   3. 读取 tp_dev.x[0], tp_dev.y[0] 获取坐标
 *   4. 检查 tp_dev.sta 获取触摸状态
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "sys.h"
#include "ott2001a.h"
#include "gt9147.h"
#include "ft5206.h"
#include "gt1151.h"

/* 触摸状态标志位定义 */
#define TP_PRES_DOWN 0x80  // 触屏被按下
#define TP_CATH_PRES 0x40  // 有按键按过
#define CT_MAX_TOUCH  5    // 电容屏支持的最大触点数

/*
 * 触摸屏控制结构体
 * 包含初始化、扫描函数指针，坐标数据，校准参数等
 */
typedef struct
{
    u8 (*init)(void);       // 初始化触摸屏控制器
    u8 (*scan)(u8);         // 扫描触摸屏 (0=屏幕坐标, 1=物理坐标)
    void (*adjust)(void);   // 触摸屏校准(电阻屏)
    
    u16 x[CT_MAX_TOUCH];    // 当前X坐标(最多5点)
    u16 y[CT_MAX_TOUCH];    // 当前Y坐标
                            // x[4],y[4]存储第一次按下时的坐标
    
    u8  sta;                // 触摸状态
                            // bit7: 按下(1)/松开(0)
                            // bit6: 0=没有按键按过, 1=有按键按过
                            // bit5: 保留
                            // bit4~0: 电容触摸屏按下的点数(0=未按下,1=1点...)
    
    /* 电阻屏校准参数(电容屏不需要校准) */
    float xfac;             // X方向校准系数
    float yfac;             // Y方向校准系数
    short xoff;             // X方向偏移量
    short yoff;             // Y方向偏移量
    
    /*
     * 触摸类型标志
     * bit0: 0=竖屏(适合X为X方向,Y为Y方向的TP)
     *       1=横屏(适合X为Y方向,Y为X方向的TP)
     * bit1~6: 保留
     * bit7: 0=电阻屏, 1=电容屏
     */
    u8 touchtype;
} _m_tp_dev;

extern _m_tp_dev tp_dev;    // 触摸屏控制器(在touch.c中定义)

/* 电阻屏SPI引脚定义 */
#define PEN     PBin(13)    // T_PEN 触摸中断
#define DOUT    PBin(11)    // T_MISO
#define TDIN    PBout(12)   // T_MOSI
#define TCLK    PBout(15)   // T_SCK
#define TCS     PBout(14)   // T_CS

/* 函数声明 */
void TP_Write_Byte(u8 num);                         // 向触摸芯片写入一个数据
u16 TP_Read_AD(u8 CMD);                             // 读取ADC转换值
u16 TP_Read_XOY(u8 xy);                             // 带滤波的读取(X/Y)
u8 TP_Read_XY(u16 *x, u16 *y);                      // 双方向读取(X+Y)
u8 TP_Read_XY2(u16 *x, u16 *y);                     // 增强滤波双方向读取
void TP_Drow_Touch_Point(u16 x, u16 y, u16 color);  // 画一个触摸校准点
void TP_Draw_Big_Point(u16 x, u16 y, u16 color);    // 画一个大点
void TP_Save_Adjdata(void);                         // 保存校准参数
u8 TP_Get_Adjdata(void);                            // 读取校准参数
void TP_Adjust(void);                               // 触摸屏校准
void TP_Adj_Info_Show(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 fac);

u8 TP_Scan(u8 tp);          // 扫描触摸屏
u8 TP_Init(void);           // 触摸屏初始化(自动识别类型)
u8 RTP_Init(void);          // 电阻触摸屏初始化

#endif
