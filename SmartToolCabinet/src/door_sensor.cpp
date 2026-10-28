// door_sensor.cpp
#include "door_sensor.h"

DoorSensor door;

DoorSensor::DoorSensor() 
    : last_state(false), 
      current_state(false),
      last_change_time(0) {
}

void DoorSensor::init() {
    Serial.println("初始化门磁传感器...");
    pinMode(DOOR_PIN, INPUT_PULLUP);
    last_state = digitalRead(DOOR_PIN) == HIGH;
    current_state = last_state;
    
    Serial.printf("门磁初始状态: %s\n", last_state ? "开启" : "关闭");
}

bool DoorSensor::isOpen() {
    current_state = digitalRead(DOOR_PIN) == HIGH;
    return current_state;
}

bool DoorSensor::stateChanged() {
    bool new_state = (digitalRead(DOOR_PIN) == HIGH);
    
    if (new_state != last_state) {
        delay(10); 
        bool confirmed = (digitalRead(DOOR_PIN) == HIGH);
        
        if (confirmed == new_state) {
            last_state = new_state;
            last_change_time = millis();
            return true;
        }
    }
    return false;
}

unsigned long DoorSensor::getOpenDuration() {
    if (!current_state) return 0;
    return millis() - last_change_time;
}