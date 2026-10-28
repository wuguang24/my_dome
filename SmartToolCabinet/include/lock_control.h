// lock_control.h - 单工具柜版本
#pragma once
#include "config.h"

class LockControl {
private:
    bool lock_status;  // 单工具柜，只有一个状态
public:
    LockControl();
    void init();
    void open();                  // 开锁（无参数）
    void close();                 // 关锁
    bool getStatus();             // 获取锁状态
    void test();                  // 测试锁
};

extern LockControl lock;