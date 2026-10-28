// ws2812_control.h
#pragma once
#include <Adafruit_NeoPixel.h>
#include "config.h"

class WS2812Control {
private:
    Adafruit_NeoPixel* strip;
    uint8_t brightness;
    
public:
    WS2812Control();
    ~WS2812Control();
    void init();
    void setColor(uint32_t color);
    void setRGB(uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t level);
    void clear();
    void off();  // 关闭所有LED
    
    void showSystemReady();
    void showSystemError();
    void showWiFiConnecting();
    void showWiFiConnected();
    void showLockOpening();
    void showLockOpened();
    void showAlarm();
    
    void breathing(uint32_t color, uint16_t duration_ms = 2000);
    void blink(uint32_t color, uint8_t times = 3, uint16_t interval_ms = 300);
    void rainbow(uint16_t wait = 10);
};

extern WS2812Control ws2812;