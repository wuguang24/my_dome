// buzzer.cpp - 低电平触发版本
#include "buzzer.h"

Buzzer buzzer;

Buzzer::Buzzer() : enabled(true) {
}

void Buzzer::init() {
    Serial.println("初始化蜂鸣器（低电平触发）...");
    delay(100);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);  // 初始状态：高电平关闭（低电平触发）
    beepStartup();
}

void Buzzer::on() {
    if (enabled) {
        pinMode(BUZZER_PIN, OUTPUT);
        digitalWrite(BUZZER_PIN, LOW);  // 低电平触发
    }
}

void Buzzer::off() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);  // 高电平关闭
}

void Buzzer::beep(uint16_t duration_ms) {
    if (enabled) {
        on();
        delay(duration_ms);
        off();
    }
}

void Buzzer::beepSuccess() {
    beep(100);
    delay(50);
    beep(100);
}

void Buzzer::beepError() {
    for (int i = 0; i < 3; i++) {
        beep(200);
        delay(100);
    }
}

void Buzzer::beepWarning() {
    beep(300);
    delay(200);
    beep(300);
}

void Buzzer::beepAlarm() {
    for (int i = 0; i < 10; i++) {
        beep(100);
        delay(100);
    }
}

void Buzzer::beepStartup() {
    beep(50);
    delay(50);
    beep(50);
    delay(50);
    beep(100);
}