// oled_display.cpp - U8g2实现
#include "oled_display.h"
#include "config.h"

extern ToolInfo g_tools[TOTAL_TOOLS];

U8G2Display display;

U8G2Display::U8G2Display() 
    : last_interaction(0), u8g2(nullptr),
      small_font(nullptr), medium_font(nullptr),
      large_font(nullptr) {
}

U8G2Display::~U8G2Display() {
    if (u8g2) {
        delete u8g2;
    }
}

void U8G2Display::init() {
    // 使用Serial代替DEBUG_PRINT，因为OLED代码可能在其他模块之前初始化
    Serial.println("初始化OLED显示屏(U8g2)...");
    
    // 使用U8g2库初始化SH1106 128x64 OLED
    u8g2 = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
    
    u8g2->begin();
    u8g2->clearBuffer();
    u8g2->setFontPosTop();
    
    // 设置字体
    small_font = u8g2_font_6x10_tr;
    medium_font = u8g2_font_7x13_tr;  
    large_font = u8g2_font_10x20_tr;
    
    u8g2->setFont(small_font);
    
    // 显示启动信息
    clear();
    u8g2->setFont(medium_font);
    u8g2->drawStr(15, 20, "Smart Toolbox");
    u8g2->setFont(small_font);
    u8g2->drawStr(35, 40, "U8g2 OLED");
    update();
    
    delay(500);
    
    Serial.println("OLED显示屏(U8g2)初始化完成");
}

int U8G2Display::getFontHeight(bool large) {
    if (large) {
        return 20;
    } else {
        return 13;
    }
}

void U8G2Display::clear() {
    if (u8g2) {
        u8g2->clearBuffer();
    }
}

void U8G2Display::update() {
    if (u8g2) {
        u8g2->sendBuffer();
    }
}

void U8G2Display::off() {
    if (u8g2) {
        u8g2->setPowerSave(1);  // 进入省电模式
    }
}

void U8G2Display::setFontForText(const String& text, bool large) {
    if (!u8g2) return;
    
    if (large) {
        u8g2->setFont(large_font);
    } else {
        u8g2->setFont(medium_font);
    }
}

int U8G2Display::calculateTextWidth(const String& text, bool large) {
    if (!u8g2) return 0;
    
    const uint8_t* current_font = u8g2->getU8g2()->font;
    setFontForText(text, large);
    int width = u8g2->getStrWidth(text.c_str());
    u8g2->getU8g2()->font = current_font;
    
    return width;
}

void U8G2Display::drawText(int x, int y, const String& text, bool center, bool large, bool small_font_mode) {
    if (!u8g2) return;
    
    if (small_font_mode) {
        u8g2->setFont(small_font);
    } else if (large) {
        u8g2->setFont(large_font);
    } else {
        u8g2->setFont(medium_font);
    }
    
    // 检查文本宽度是否超出屏幕
    int textWidth = calculateTextWidth(text, large);
    String displayText = text;
    
    if (textWidth > 128) {
        if (large) {
            if (text.length() > 8) {
                displayText = text.substring(0, 8);
            }
        } else {
            if (text.length() > 15) {
                displayText = text.substring(0, 15);
            }
        }
        textWidth = calculateTextWidth(displayText, large);
    }
    
    if (center) {
        x = (128 - textWidth) / 2;
        if (x < 0) x = 0;
    }
    
    u8g2->drawStr(x, y, displayText.c_str());
    u8g2->setFont(small_font);
}

void U8G2Display::drawCenterText(int y, const String& text, bool large) {
    drawText(0, y, text, true, large);
}

void U8G2Display::drawSmallText(int x, int y, const String& text, bool center) {
    drawText(x, y, text, center, false, true);
}

void U8G2Display::drawHeader(const String& title) {
    if (!u8g2) return;
    
    String displayTitle = title;
    if (calculateTextWidth(title, false) > 128) {
        if (title == "SELECT TOOL TO RETURN") {
            displayTitle = "SELECT TOOL RETURN";
        } else if (title == "SELECT TOOL TO BORROW") {
            displayTitle = "SELECT TOOL BORROW";
        } else if (title.length() > 15) {
            displayTitle = title.substring(0, 15);
        }
    }
    
    drawCenterText(3, displayTitle);
    u8g2->drawHLine(0, 16, 128);
}

void U8G2Display::showMessage(const String& message, bool persistent) {
    if (!u8g2) return;
    
    clear();
    drawHeader("Message");
    
    if (calculateTextWidth(message, false) > 128) {
        int spacePos = message.lastIndexOf(' ', message.length() / 2);
        if (spacePos == -1) spacePos = message.length() / 2;
        
        String line1 = message.substring(0, spacePos);
        String line2 = message.substring(spacePos + 1);
        
        if (line2.length() > 0) {
            drawCenterText(25, line1);
            drawCenterText(40, line2);
        } else {
            drawCenterText(30, line1);
        }
    } else {
        drawCenterText(30, message);
    }
    
    if (!persistent) {
        drawText(30, 52, "Auto return");
    }
    
    update();
    resetIdleTimer();
}

void U8G2Display::showBootScreen() {
    if (!u8g2) return;
    
    clear();
    u8g2->setFont(medium_font);
    u8g2->drawStr(15, 20, "Smart Toolbox");
    u8g2->setFont(small_font);
    u8g2->drawStr(35, 40, "U8g2 OLED Ready");
    update();
}

void U8G2Display::showMainMenu(uint8_t selection) {
    if (!u8g2) return;
    
    clear();
    drawHeader("MAIN MENU");
    
    if (selection == 0) {
        drawText(20, 22, "> 1. BORROW");
        drawText(20, 37, "  2. RETURN");
    } else {
        drawText(20, 22, "  1. BORROW");
        drawText(20, 37, "> 2. RETURN");
    }
    
    drawText(4, 52, "Rotate & Press OK");
    update();
    resetIdleTimer();
}

void U8G2Display::showToolSelect(uint8_t selection, bool isBorrow) {
    if (!u8g2) return;
    
    clear();
    String title = isBorrow ? "SELECT TOOL" : "RETURN TOOL";
    drawHeader(title);
    
    int start_index = (selection / 3) * 3;
    for (int i = 0; i < 3 && (start_index + i) < TOTAL_TOOLS; i++) {
        int tool_index = start_index + i;
        int y = 20 + i * 12;
        
        String item;
        if (tool_index == selection) {
            item = "> ";
        } else {
            item = "  ";
        }
        
        // 使用物模型中的工具名称缩写
        String toolName;
        switch (tool_index) {
            case 0: toolName = "Screwdriver"; break;
            case 1: toolName = "Pliers"; break;
            case 2: toolName = "Wrench"; break;
            case 3: toolName = "Hammer"; break;
            case 4: toolName = "TapeMeas"; break;
            case 5: toolName = "Knife"; break;
            default: toolName = g_tools[tool_index].name;
        }
        
        item += String(tool_index + 1) + ". " + toolName;
        
        if (g_tools[tool_index].is_borrowed) {
            item += " [B]";
        }
        
        if (calculateTextWidth(item, false) > 120) {
            item = item.substring(0, 15);
        }
        
        drawText(5, y, item);
    }
    
    String page = String(selection + 1) + "/" + String(TOTAL_TOOLS);
    drawSmallText(64, 52, page, true);
    update();
    resetIdleTimer();
}

void U8G2Display::showInputScreen(uint8_t number) {
    if (!u8g2) return;
    
    clear();
    drawHeader("ENTER NUMBER");
    
    drawCenterText(22, "Enter tool #:");
    
    String input = String(number);
    if (input.length() == 1) {
        input = "0" + input;
    }
    drawCenterText(37, input, true);
    
    drawText(20, 52, "Rotate Adjust");
    update();
    resetIdleTimer();
}

void U8G2Display::showConfirmScreen(uint8_t tool_index, bool isBorrow) {
    if (!u8g2) return;
    
    clear();
    drawHeader("CONFIRM");
    
    // 使用物模型中的工具名称缩写
    String toolName;
    switch (tool_index) {
        case 0: toolName = "Screwdriver"; break;
        case 1: toolName = "Pliers"; break;
        case 2: toolName = "Wrench"; break;
        case 3: toolName = "Hammer"; break;
        case 4: toolName = "TapeMeas"; break;
        case 5: toolName = "Knife"; break;
        default: toolName = g_tools[tool_index].name;
    }
    
    drawCenterText(22, toolName, true);
    
    String status = g_tools[tool_index].is_borrowed ? "BORROWED" : "AVAILABLE";
    drawCenterText(40, "Status: " + status);
    
    drawText(20, 50, "Back Cancel OK");
    update();
    resetIdleTimer();
}

void U8G2Display::showRFIDScreen(uint8_t tool_index) {
    if (!u8g2) return;
    
    clear();
    drawHeader("SCAN RFID");
    
    // 使用物模型中的工具名称缩写
    String toolName;
    switch (tool_index) {
        case 0: toolName = "Screwdriver"; break;
        case 1: toolName = "Pliers"; break;
        case 2: toolName = "Wrench"; break;
        case 3: toolName = "Hammer"; break;
        case 4: toolName = "TapeMeas"; break;
        case 5: toolName = "Knife"; break;
        default: toolName = g_tools[tool_index].name;
    }
    
    drawCenterText(22, toolName, true);
    drawCenterText(40, "Scan Tool RFID");
    
    drawText(30, 50, "Back Cancel");
    update();
    resetIdleTimer();
}

void U8G2Display::showUnlockedScreen(uint8_t tool_index) {
    if (!u8g2) return;
    
    clear();
    drawHeader("UNLOCKED");
    
    // 使用物模型中的工具名称缩写
    String toolName;
    switch (tool_index) {
        case 0: toolName = "Screwdriver"; break;
        case 1: toolName = "Pliers"; break;
        case 2: toolName = "Wrench"; break;
        case 3: toolName = "Hammer"; break;
        case 4: toolName = "TapeMeas"; break;
        case 5: toolName = "Knife"; break;
        default: toolName = g_tools[tool_index].name;
    }
    
    drawCenterText(22, toolName, true);
    drawCenterText(40, "Take/Return");
    update();
    resetIdleTimer();
}

void U8G2Display::showAlarmScreen() {
    if (!u8g2) return;
    
    clear();
    static bool blink = false;
    
    if (blink) {
        drawCenterText(12, "ALARM", true);
    }
    
    drawCenterText(32, "Unauthorized");
    drawCenterText(47, "Door Open!");
    update();
    blink = !blink;
}

void U8G2Display::showConnectingScreen() {
    if (!u8g2) return;
    
    clear();
    drawHeader("NETWORK");
    
    static uint8_t dotCount = 0;
    String dots = "";
    for (int i = 0; i < (dotCount % 4); i++) {
        dots += ".";
    }
    
    drawCenterText(32, "Connecting" + dots);
    update();
    resetIdleTimer();
    
    dotCount++;
    if (dotCount > 3) dotCount = 0;
}

void U8G2Display::showConnectedScreen() {
    if (!u8g2) return;
    
    clear();
    drawHeader("SYSTEM READY");
    drawCenterText(32, "Connected");
    update();
}

void U8G2Display::showStatusScreen() {
    if (!u8g2) return;
    
    clear();
    drawHeader("SYSTEM STATUS");
    
    int borrowedCount = 0;
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        if (g_tools[i].is_borrowed) borrowedCount++;
    }
    
    drawText(10, 22, "Total: " + String(TOTAL_TOOLS));
    drawText(10, 37, "Borrowed: " + String(borrowedCount));
    
    String status = "Avail: " + String(TOTAL_TOOLS - borrowedCount);
    drawCenterText(50, status);
    
    drawText(15, 56, "Long press Back");
    update();
    resetIdleTimer();
}

String U8G2Display::getToolStatus(uint8_t tool_index) {
    if (tool_index >= TOTAL_TOOLS) return "ERROR";
    return g_tools[tool_index].is_borrowed ? "[BORROWED]" : "[AVAILABLE]";
}

void U8G2Display::resetIdleTimer() {
    last_interaction = millis();
}

bool U8G2Display::isIdle(unsigned long timeout) {
    return (millis() - last_interaction) > timeout;
}