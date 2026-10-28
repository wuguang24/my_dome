#ifndef __OLED_H
#define __OLED_H

// OLED初始化函数
void OLED_Init(void);
// OLED清屏函数
void OLED_Clear(void);
// 显示单个字符
// 参数：Line - 行号（1-4），Column - 列号（1-16），Char - 要显示的字符
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
// 显示字符串
// 参数：Line - 行号（1-4），Column - 列号（1-16），String - 要显示的字符串
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
// 显示无符号数字
// 参数：Line - 行号（1-4），Column - 列号（1-16），Number - 要显示的数字，Length - 数字长度
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// 显示有符号数字
// 参数：Line - 行号（1-4），Column - 列号（1-16），Number - 要显示的数字，Length - 数字长度
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
// 显示十六进制数字
// 参数：Line - 行号（1-4），Column - 列号（1-16），Number - 要显示的数字，Length - 数字长度
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// 显示二进制数字
// 参数：Line - 行号（1-4），Column - 列号（1-16），Number - 要显示的数字，Length - 数字长度
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
// 显示中文字符
// 参数：Line - 行号（1-4），Column - 列号（1-8），Index - 中文字符索引
void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t Index);

#endif
