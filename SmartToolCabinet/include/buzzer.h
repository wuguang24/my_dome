// buzzer.h
#pragma once
#include "config.h"

class Buzzer {
private:
    bool enabled;
    
public:
    Buzzer();
    void init();
    void on();
    void off();
    void beep(uint16_t duration_ms = 100);
    void beepSuccess();
    void beepError();
    void beepWarning();
    void beepAlarm();
    void beepStartup();
    
    void enable(bool state) { enabled = state; }
    bool isEnabled() { return enabled; }
};

extern Buzzer buzzer;