// button_control.h
#pragma once
#include <Arduino.h>
#include "config.h"
#include <ESP32Encoder.h>

class ButtonControl {
private:
    struct ButtonState {
        bool current;
        bool last;
        bool pressed;
        bool clicked;
        unsigned long press_start_time;
        unsigned long last_debounce_time;
    };
    
    ButtonState encoder_btn;
    ButtonState back_btn;  
    
    int8_t encoder_position;
    int8_t last_encoder_delta;
    ESP32Encoder encoder;
    int last_encoder_count;
    unsigned long last_encoder_change;
    bool encoder_enabled;
    
public:
    ButtonControl();
    void init();
    void update();
    
    // 旋钮状态查询
    bool isEncoderPressed();
    bool isEncoderClicked();
    bool isEncoderLongPressed(unsigned long threshold = LONG_PRESS_MS);
    
    // BACK按钮状态查询
    bool isBackPressed();
    bool isBackClicked();
    bool isBackLongPressed(unsigned long threshold = LONG_PRESS_MS);
    
    // 编码器旋转（带步进限制）
    int8_t getEncoderDelta();
    
    // 获取按键动作
    ButtonAction getButtonAction();
    
    // 检查是否有活动
    bool hasActivity();
    
    // 旋钮控制
    void enableEncoder(bool enable) { encoder_enabled = enable; }
    
private:
    void updateButton(ButtonState& btn, uint8_t pin);
    void updateEncoder();
};

extern ButtonControl buttons;