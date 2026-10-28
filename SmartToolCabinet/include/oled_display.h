// oled_display.h - U8g2实现
#pragma once
#include <U8g2lib.h>
#include <Arduino.h>
#include "config.h"


class U8G2Display {
private:
    U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2;
    unsigned long last_interaction;
    
    // 字体定义
    const uint8_t* small_font;
    const uint8_t* medium_font;  
    const uint8_t* large_font;
    
public:
    U8G2Display();
    ~U8G2Display();
    
    // 基本功能
    void init();
    void clear();
    void update();
    void off();  // 关闭显示
    
    // 文本显示函数
    void drawText(int x, int y, const String& text, bool center = false, bool large = false, bool small_font_mode = false);
    void drawSmallText(int x, int y, const String& text, bool center = false);
    void drawCenterText(int y, const String& text, bool large = false);
    void drawHeader(const String& title);
    void showMessage(const String& message, bool persistent = false);
    
    // 系统界面
    void showBootScreen();
    void showMainMenu(uint8_t selection = 0);
    void showToolSelect(uint8_t selection, bool isBorrow);
    void showInputScreen(uint8_t number);
    void showConfirmScreen(uint8_t tool_index, bool isBorrow);
    void showRFIDScreen(uint8_t tool_index);
    void showUnlockedScreen(uint8_t tool_index);
    void showAlarmScreen();
    void showConnectingScreen();
    void showConnectedScreen();
    void showStatusScreen();
    
    // 工具函数
    void resetIdleTimer();
    bool isIdle(unsigned long timeout = 30000);
    
private:
    String getToolStatus(uint8_t tool_index);
    int calculateTextWidth(const String& text, bool large = false);
    void setFontForText(const String& text, bool large = false);
    int getFontHeight(bool large);
};

extern U8G2Display display;