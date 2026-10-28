// door_sensor.h
#pragma once
#include "config.h"

class DoorSensor {
private:
    bool last_state;
    bool current_state;
    unsigned long last_change_time;
    int debounce_counter;  // 新增：防抖计数器
    bool confirmed_state;  // 新增：确认状态
    
public:
    DoorSensor();
    void init();
    bool isOpen();
    bool stateChanged();
    unsigned long getOpenDuration();
};

extern DoorSensor door;