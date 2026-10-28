// button_control.cpp
#include "button_control.h"

ButtonControl buttons;

ButtonControl::ButtonControl() 
    : encoder_position(0), 
      last_encoder_delta(0),
      last_encoder_count(0),
      last_encoder_change(0),
      encoder_enabled(true) {
}

void ButtonControl::init() {
    Serial.println("初始化按键控制...");
    
    // 设置ESP32Encoder使用内部上拉电阻
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    
    // 设置编码器引脚 - 使用全步进模式，更稳定
    encoder.attachFullQuad(ENCODER_TRA, ENCODER_TRB);
    
    // 设置按钮引脚（内部上拉）
    pinMode(ENCODER_PSH, INPUT_PULLUP);
    pinMode(BUTTON_BACK, INPUT_PULLUP);
    
    // 获取初始编码器计数值
    encoder.clearCount();
    last_encoder_count = 0;
    
    Serial.println("按键控制初始化完成");
}

void ButtonControl::update() {
    updateButton(encoder_btn, ENCODER_PSH);
    updateButton(back_btn, BUTTON_BACK);
    
    if (encoder_enabled) {
        updateEncoder();
    }
}

void ButtonControl::updateButton(ButtonState& btn, uint8_t pin) {
    bool reading = digitalRead(pin) == LOW;
    
    if (reading != btn.last) {
        btn.last_debounce_time = millis();
    }
    
    if ((millis() - btn.last_debounce_time) > BUTTON_DEBOUNCE_MS) {
        if (reading != btn.current) {
            btn.current = reading;
            
            if (btn.current) {
                btn.pressed = true;
                btn.press_start_time = millis();
            } else {
                if (millis() - btn.press_start_time < 5000) {
                    btn.clicked = true;
                }
            }
        }
    }
    
    btn.last = reading;
}

void ButtonControl::updateEncoder() {
    static int last_stable_count = 0;
    unsigned long now = millis();
    
    // 读取当前编码器计数值
    int current_count = encoder.getCount();
    
    // 计算当前计数与上一次稳定计数的差值
    int delta = current_count - last_stable_count;
    
    // 只有当变化达到2个脉冲时才认为是有效旋转
    if (abs(delta) >= 4) {
        // 去抖延迟
        if (now - last_encoder_change < 80) {
            return;
        }
        
        last_encoder_change = now;
        
        // 确定旋转方向（只取符号，不取实际值）
        if (delta > 0) {
            encoder_position = 1;  // 顺时针
            Serial.println("顺时针旋转");
        } else {
            encoder_position = -1; // 逆时针
            Serial.println("逆时针旋转");
        }
        
        // 更新稳定计数
        last_stable_count = current_count;
        last_encoder_count = current_count;
    } else {
        // 只是小幅度变化，更新计数但不触发动作
        last_encoder_count = current_count;
    }
}

bool ButtonControl::isEncoderPressed() {
    bool pressed = encoder_btn.pressed;
    encoder_btn.pressed = false;
    return pressed;
}

bool ButtonControl::isEncoderClicked() {
    bool clicked = encoder_btn.clicked;
    encoder_btn.clicked = false;
    return clicked;
}

bool ButtonControl::isEncoderLongPressed(unsigned long threshold) {
    if (encoder_btn.current && (millis() - encoder_btn.press_start_time >= threshold)) {
        encoder_btn.current = false;
        return true;
    }
    return false;
}

bool ButtonControl::isBackPressed() {
    bool pressed = back_btn.pressed;
    back_btn.pressed = false;
    return pressed;
}

bool ButtonControl::isBackClicked() {
    bool clicked = back_btn.clicked;
    back_btn.clicked = false;
    return clicked;
}

bool ButtonControl::isBackLongPressed(unsigned long threshold) {
    if (back_btn.current && (millis() - back_btn.press_start_time >= threshold)) {
        back_btn.current = false;
        return true;
    }
    return false;
}

// 注意：这个函数应该只有一个定义！
int8_t ButtonControl::getEncoderDelta() {
    int8_t delta = encoder_position;
    encoder_position = 0;  // 重置位置
    
    // 只有有变化时才输出调试信息
    if (delta != 0) {
        Serial.printf("编码器变化: %d\n", delta);
    }
    
    return delta;
}

ButtonAction ButtonControl::getButtonAction() {
    if (isEncoderClicked()) return ButtonAction::ENCODER_PRESSED;
    if (isEncoderLongPressed()) return ButtonAction::ENCODER_LONG_PRESSED;
    if (isBackClicked()) return ButtonAction::BACK_PRESSED;
    if (isBackLongPressed()) return ButtonAction::BACK_LONG_PRESSED;
    
    int8_t delta = getEncoderDelta();
    if (delta > 0) return ButtonAction::ENCODER_ROTATE_CW;
    if (delta < 0) return ButtonAction::ENCODER_ROTATE_CCW;
    
    return ButtonAction::NONE;
}

bool ButtonControl::hasActivity() {
    // 检查是否有任何按钮或编码器活动
    return encoder_btn.pressed || encoder_btn.clicked || 
           back_btn.pressed || back_btn.clicked ||
           encoder_position != 0;
}