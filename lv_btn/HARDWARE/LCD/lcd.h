/*
 * 模块名称: TFT LCD显示驱动 (lcd)
 * 功能概述:
 *   - 支持2.8寸/3.5寸/4.3寸 TFT LCD显示屏
 *   - 支持多种驱动IC: NT35310, NT35510, ST7796, ST7789
 *   - 使用FSMC接口实现高速数据传输
 *   - 提供基本绘图函数: 画点、画线、画矩形、填充、显示字符等
 * 
 * 硬件接口:
 *   - 使用FSMC Bank1 NOR/SRAM模式
 *   - 数据总线: 16位并行
 *   - 控制信号: RS(A18), CS, WR, RD
 *   - 背光控制: PD12
 *   - 复位: PD11
 * 
 * 分辨率:
 *   - ST7789: 240x320
 *   - ST7796/NT35310: 320x480
 *   - NT35510: 480x800
 * 
 * 颜色格式:
 *   - RGB565 (16位色)
 *   - 高字节: RRRRRGGG, 低字节: GGGBBBBB
 * 
 * 使用方法:
 *   1. 调用 LCD_Init() 初始化LCD
 *   2. 使用 LCD_Clear(color) 清屏
 *   3. 使用 LCD_DrawPoint/LCD_DrawLine 等绘图
 *   4. 使用 LCD_ShowString 显示文字
 * 
 * 硬件平台: STM32F407ZGT6开发板
 * 版本: V1.0
 */
#ifndef __LCD_H
#define __LCD_H
#include "sys.h"
#include "stdlib.h"

extern SRAM_HandleTypeDef TFTSRAM_Handler;  // SRAM句柄(用于控制LCD)

/*
 * LCD重要参数结构体
 * 存储LCD的宽高、ID、方向等信息
 */
typedef struct
{
    u16 width;      // LCD 宽度
    u16 height;     // LCD 高度
    u16 id;         // LCD ID (如0x7796, 0x7789等)
    u8  dir;        // 横屏或竖屏控制: 0=竖屏, 1=横屏
    u16 wramcmd;    // 开始写GRAM指令
    u16 setxcmd;    // 设置X坐标指令
    u16 setycmd;    // 设置Y坐标指令
} _lcd_dev;

extern _lcd_dev lcddev;     // LCD参数结构体
extern u16 POINT_COLOR;     // 画笔颜色(默认黑色)
extern u16 BACK_COLOR;      // 背景颜色(默认白色)

/*
 * LCD控制引脚定义
 */
#define LCD_LED  PDout(12)  // LCD背光控制 (PD12)
#define LCD_RST  PDout(11)  // LCD复位 (PD11)

/*
 * LCD地址结构体
 * 使用FSMC Bank1.sector1, 地址位HADDR[27,26]=00
 * A18作为数据/命令选择线
 */
typedef struct
{
    vu16 LCD_REG;   // 寄存器地址(RS=0)
    vu16 LCD_RAM;   // 数据地址(RS=1)
} LCD_TypeDef;

#define LCD_BASE    ((u32)(0x60000000 + (0x4000000 * (1 - 1))) | (((1 << 18) * 2) - 2))
#define LCD         ((LCD_TypeDef *) LCD_BASE)

/*
 * 扫描方向定义
 * 用于设置LCD的显示方向
 */
#define L2R_U2D  0   // 从左到右,从上到下
#define L2R_D2U  1   // 从左到右,从下到上
#define R2L_U2D  2   // 从右到左,从上到下
#define R2L_D2U  3   // 从右到左,从下到上
#define U2D_L2R  4   // 从上到下,从左到右
#define U2D_R2L  5   // 从上到下,从右到左
#define D2U_L2R  6   // 从下到上,从左到右
#define D2U_R2L  7   // 从下到上,从右到左

#define DFT_SCAN_DIR  L2R_U2D  // 默认扫描方向

/*
 * 屏幕方向设置
 * 0=竖屏, 1=横屏
 */
#define LCD_DIR  1

/*
 * 常用颜色定义 (RGB565格式)
 */
#define WHITE       0xFFFF   // 白色
#define BLACK       0x0000   // 黑色
#define BLUE        0x001F   // 蓝色
#define BRED        0XF81F   // 品红
#define GRED        0XFFE0   // 黄绿
#define GBLUE       0X07FF   // 青色
#define RED         0xF800   // 红色
#define MAGENTA     0xF81F   // 洋红
#define GREEN       0x07E0   // 绿色
#define CYAN        0x7FFF   // 青色
#define YELLOW      0xFFE0   // 黄色
#define BROWN       0XBC40   // 棕色
#define BRRED       0XFC07   // 棕红色
#define GRAY        0X8430   // 灰色

/* GUI颜色 */
#define DARKBLUE    0X01CF   // 深蓝色
#define LIGHTBLUE   0X7D7C   // 浅蓝色
#define GRAYBLUE    0X5458   // 灰蓝色
#define LIGHTGREEN  0X841F   // 浅绿色
#define LGRAY       0XC618   // 浅灰色(PANNEL背景色)
#define LGRAYBLUE   0XA651   // 浅灰蓝色
#define LBBLUE      0X2B12   // 浅棕蓝色

/*
 * 函数声明
 */
void LCD_Init(void);                                            // 初始化LCD
void LCD_DisplayOn(void);                                       // 开显示
void LCD_DisplayOff(void);                                      // 关显示
void LCD_Clear(u16 Color);                                      // 清屏
void LCD_SetCursor(u16 Xpos, u16 Ypos);                        // 设置光标
void LCD_DrawPoint(u16 x, u16 y);                              // 画点
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color);              // 快速画点
u16  LCD_ReadPoint(u16 x, u16 y);                              // 读点
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r);                    // 画圆
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);             // 画线
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);        // 画矩形
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color);      // 填充单色
void LCD_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color); // 填充指定颜色
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode);     // 显示一个字符
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size);      // 显示数字(高位为0不显示)
void LCD_ShowxNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode); // 显示数字(可选显示高位0)
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p); // 显示字符串

void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);  // 写寄存器
u16 LCD_ReadReg(u16 LCD_Reg);                      // 读寄存器
void LCD_WriteRAM_Prepare(void);                   // 开始写GRAM
void LCD_WriteRAM(u16 RGB_Code);                   // 写GRAM
void LCD_Scan_Dir(u8 dir);                         // 设置扫描方向
void LCD_Display_Dir(u8 dir);                      // 设置屏幕显示方向
void LCD_Set_Window(u16 sx, u16 sy, u16 width, u16 height); // 设置窗口

#endif
