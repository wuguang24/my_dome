// ws2812_control.cpp
#include "ws2812_control.h"
#include "config.h"

WS2812Control ws2812;

WS2812Control::WS2812Control() 
    : brightness(50) {
    strip = new Adafruit_NeoPixel(WS2812_NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
}

WS2812Control::~WS2812Control() {
    delete strip;
}

void WS2812Control::init() {
    strip->begin();
    strip->show();
    strip->setBrightness(brightness);
    rainbow(5);
    showSystemReady();
}

void WS2812Control::setColor(uint32_t color) {
    strip->setPixelColor(0, color);
    strip->show();
}

void WS2812Control::setRGB(uint8_t r, uint8_t g, uint8_t b) {
    strip->setPixelColor(0, strip->Color(r, g, b));
    strip->show();
}

void WS2812Control::setBrightness(uint8_t level) {
    if (level > 100) level = 100;
    brightness = level;
    strip->setBrightness(map(brightness, 0, 100, 0, 255));
    strip->show();
}

void WS2812Control::clear() {
    strip->clear();
    strip->show();
}

void WS2812Control::off() {
    strip->clear();
    strip->show();
    strip->setBrightness(0);
}

void WS2812Control::showSystemReady() {
    setRGB(0, 255, 0);
    setBrightness(30);
}

void WS2812Control::showSystemError() {
    blink(0xFF0000, 5, 200);
}

void WS2812Control::showWiFiConnecting() {
    breathing(0x0000FF, 1500);
}

void WS2812Control::showWiFiConnected() {
    setRGB(0, 0, 255);
    setBrightness(50);
}

void WS2812Control::showLockOpening() {
    breathing(0xFFFF00, 1000);
}

void WS2812Control::showLockOpened() {
    blink(0x00FF00, 3, 300);
    showSystemReady();
}

void WS2812Control::showAlarm() {
    blink(0xFF0000, 3, 200);
}

void WS2812Control::breathing(uint32_t color, uint16_t duration_ms) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    for (int i = 0; i <= 100; i++) {
        uint8_t current = map(i, 0, 100, 0, brightness);
        setBrightness(current);
        setRGB(r, g, b);
        delay(duration_ms / 200);
    }
    
    for (int i = 100; i >= 0; i--) {
        uint8_t current = map(i, 0, 100, 0, brightness);
        setBrightness(current);
        setRGB(r, g, b);
        delay(duration_ms / 200);
    }
    
    setBrightness(brightness);
}

void WS2812Control::blink(uint32_t color, uint8_t times, uint16_t interval_ms) {
    for (int i = 0; i < times; i++) {
        setColor(color);
        delay(interval_ms / 2);
        clear();
        delay(interval_ms / 2);
    }
}

void WS2812Control::rainbow(uint16_t wait) {
    for (long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
        strip->setPixelColor(0, strip->gamma32(strip->ColorHSV(firstPixelHue)));
        strip->show();
        delay(wait);
    }
}