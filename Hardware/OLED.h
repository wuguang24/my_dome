#ifndef __OLED_H
#define __OLED_H

// OLED 显示模块接口（对应 OLED.c）
// 坐标约定：
// - Line：1~4（每行 16 像素高，适配 8x16 字符）
// - Column：1~16（每列 8 像素宽）
// 注意：16x16 中文会占用 2 列（Column 以字符宽度为单位）。

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
// 显示字符串：以 '\0' 结尾
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String);
//void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// 显示中文：Index 为字模表中的索引（在 OLED_Font.h 的 Hzk1[][] 定义）
void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t Index);

#endif
