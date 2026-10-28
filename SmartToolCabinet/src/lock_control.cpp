// lock_control.cpp - 通电解锁版本
#include "lock_control.h"
#include "buzzer.h"

LockControl lock;

LockControl::LockControl() : lock_status(false) {
}

void LockControl::init() {
    Serial.println("初始化电磁锁控制（通电解锁）...");
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // 初始断电状态
    Serial.println("锁控制初始化完成");
}

void LockControl::open() {
    Serial.println("打开电磁锁...");
    
    // 通电解锁
    digitalWrite(RELAY_PIN, HIGH);
    lock_status = true;
    
    // 保持通电0.5秒解锁
    delay(500);
    
    // 断电保持锁状态
    digitalWrite(RELAY_PIN, LOW);
    
    // 等待继电器稳定，避免电磁干扰影响后续操作
    delay(100);
    
    Serial.println("电磁锁已打开");
}

void LockControl::close() {
    Serial.println("电磁锁自动关闭");
    lock_status = false;
    // 不需要物理操作，断电即关闭
}

bool LockControl::getStatus() {
    return lock_status;
}

void LockControl::test() {
    Serial.println("测试电磁锁...");
    open();
    delay(1000);
    close();
    Serial.println("锁测试完成");
}