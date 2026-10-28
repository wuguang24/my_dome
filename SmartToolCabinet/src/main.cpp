// main.cpp - Single Cabinet RFID Tool Management System (English Interface)
#include <Arduino.h>
#include "config.h"
#include "button_control.h"
#include "oled_display.h"
#include "wifi_mqtt.h"
#include "rfid_reader.h"
#include "lock_control.h"
#include "door_sensor.h"
#include "buzzer.h"
#include "ws2812_control.h"
#include "camera_capture.h"
#include "storage.h"

// ========== 全局变量 ==========
AppState g_state = AppState::INITIALIZING;
OperationMode g_mode = OperationMode::LOCAL_CONTROL;
uint8_t g_selected_tool_index = 0;
uint8_t g_menu_selection = 0;  // 主菜单选择（0=借用，1=归还）
bool g_DoorUnlockAuthorized = false;
bool g_AlarmTriggered = false;
bool system_initialized = false;
bool wifi_connected = false;
bool mqtt_connected = false;
uint32_t alarm_count = 0;
bool g_RemoteUnlockRequested = false;
uint8_t g_RemoteUnlockToolIndex = 0;
uint8_t g_RemoteUnlockDuration = 5;
uint8_t g_RemoteUnlockMode = 1;  // 0=归还模式, 1=借用模式

// 借用超时检测
unsigned long g_borrow_times[TOTAL_TOOLS] = {0}; // 每个工具的借用时间
const unsigned long BORROW_TIMEOUT = 3600000;    // 借用超时时间（1小时）
const unsigned long BORROW_WARNING = 2700000;    // 借用警告时间（45分钟）

// 低功耗模式
unsigned long g_last_activity_time = 0;
const unsigned long SLEEP_TIMEOUT = 300000;      // 5分钟无操作进入休眠

// 工具定义（与物模型对应）
ToolInfo g_tools[TOTAL_TOOLS] = {
    {"Screwdriver", 0xE143F705, false},     // 螺丝刀
    {"Pliers", 0x0435E232, false},          // 钳子
    {"Wrench", 0x3456789A, false},          // 扳手
    {"Hammer", 0x456789AB, false},          // 锤子
    {"Tape Measure", 0x56789ABC, false},    // 卷尺
    {"Utility Knife", 0x6789ABCD, false}    // 工具刀
};

// 网络连接相关
unsigned long g_lastNetworkAttempt = 0;
uint8_t g_networkAttemptCount = 0;

//门授权和报警相关
unsigned long g_door_auth_time = 0;      // 门授权开始时间
unsigned long g_alarm_start_time = 0;    // 报警开始时间
bool g_alarm_triggered = false;          // 报警触发标志

// ========== 函数声明 ==========
void systemInit();
void tryConnectNetwork();
void processRFIDScan(bool isBorrow);
void handleUserInput();
void checkIllegalOpen();
void handleRemoteUnlock();
void checkBorrowTimeout();
void checkAutoReturn();
void enterDeepSleep();
void updateActivityTime();
String getCurrentTimeString();

// ========== 工具状态管理 ==========
void SaveToolStatus() {
    storage.saveAllToolsStatus();
    
    // 更新物模型状态
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        network.setToolStatusBit(i, g_tools[i].is_borrowed);
        // 记录借用时间
        if (g_tools[i].is_borrowed && g_borrow_times[i] == 0) {
            g_borrow_times[i] = millis();
        } else if (!g_tools[i].is_borrowed) {
            g_borrow_times[i] = 0;
        }
    }
    if (network.isConnected()) {
        network.publishToolStatusBitmap();
    }
}

void LoadToolStatus() {
    storage.loadAllToolsStatus();
    
    // 同步到网络模块
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        network.setToolStatusBit(i, g_tools[i].is_borrowed);
    }
}

// 验证RFID UID并返回工具索引
bool IsValidToolUID(uint32_t uid, uint8_t* tool_index) {
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        if (g_tools[i].rfid_uid == uid) {
            *tool_index = i;
            return true;
        }
    }
    return false;
}

// ========== 系统初始化 ==========
void systemInit() {
    Serial.begin(115200);
    delay(100);
    Serial.println("智能工具柜系统启动 - 单柜RFID版本");
    Serial.println("========================================");
    
    // 初始化各模块
    display.init();
    buzzer.init();
    buttons.init();
    storage.init();
    rfid.init();
    lock.init();
    door.init();
    ws2812.init();
    camera.init();
    network.init();
    
    // 设置物模型回调函数
    network.setRemoteUnlockCallback([](uint8_t tool_index, uint8_t duration) {
        Serial.printf("收到OneNet远程开锁指令: 工具 %d, 时长 %d秒\n", tool_index + 1, duration);
        g_RemoteUnlockToolIndex = tool_index;
        g_RemoteUnlockDuration = duration;
        g_RemoteUnlockRequested = true;
    });
    
    network.setSetToolStatusCallback([](uint8_t tool_index, bool status) {
        Serial.printf("收到OneNet设置工具状态: 工具 %d -> %s\n", 
                    tool_index + 1, status ? "借出" : "归还");
        g_tools[tool_index].is_borrowed = status;
        SaveToolStatus();
    });
    
    network.setSystemResetCallback([](uint8_t reset_type) {
        Serial.printf("收到系统复位指令: 类型 %d\n", reset_type);
        switch (reset_type) {
            case 1: // 重启系统
                ESP.restart();
                break;
            case 2: // 恢复默认配置
                // 可以添加恢复默认配置的逻辑
                break;
            case 3: // 清除借用记录
                storage.clearRecords();
                break;
        }
    });
    
    network.setTestAlarmCallback([](uint8_t alarm_type, uint8_t duration) {
        Serial.printf("收到测试报警指令: 类型 %d, 时长 %d秒\n", alarm_type, duration);
        // 这里可以添加测试报警的逻辑
    });
    
    network.setTakePhotoCallback([](uint8_t photo_type) {
        Serial.printf("收到拍照指令: 类型 %d\n", photo_type);
        // 这里可以添加拍照的逻辑
    });
    
    // 加载工具状态
    LoadToolStatus();
    
    // 设置初始系统状态
    network.setSystemStatus(0); // 初始化中
    network.setLockStatus(true); // 锁关闭
    
    // 显示启动成功
    buzzer.beepSuccess();
    display.showBootScreen();
    delay(1000);
    
    // 系统初始化完成
    system_initialized = true;
    network.setSystemStatus(1); // 运行正常
    
    Serial.println("系统初始化完成");
    
    // 直接进入主菜单
    g_menu_selection = 0;  // 重置菜单选择
    g_state = AppState::MAIN_MENU;
    display.showMainMenu(g_menu_selection);  // 使用当前选择值
}

// ========== 获取当前时间字符串 ==========
String getCurrentTimeString() {
    unsigned long totalSeconds = millis() / 1000;
    unsigned long hours = totalSeconds / 3600;
    unsigned long minutes = (totalSeconds % 3600) / 60;
    unsigned long seconds = totalSeconds % 60;
    
    char timeStr[12];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    return String(timeStr);
}

// ========== 更新活动时间 ==========
void updateActivityTime() {
    g_last_activity_time = millis();
}

// ========== 检查借用超时 ==========
void checkBorrowTimeout() {
    static unsigned long last_check = 0;
    
    // 每分钟检查一次
    if (millis() - last_check < 60000) return;
    last_check = millis();
    
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        if (g_tools[i].is_borrowed && g_borrow_times[i] > 0) {
            unsigned long borrow_duration = millis() - g_borrow_times[i];
            
            // 超时报警
            if (borrow_duration > BORROW_TIMEOUT) {
                Serial.printf("工具 %d 借用超时!\n", i + 1);
                
                // 上报超时事件到云平台
                if (network.isConnected()) {
                    network.publishTimeoutAlarm(i, borrow_duration / 60000);
                }
                
                // 本地提示
                buzzer.beepWarning();
                display.showMessage("Timeout!", false);
                delay(1000);
                display.showMainMenu(0);
            }
            // 即将超时警告
            else if (borrow_duration > BORROW_WARNING) {
                Serial.printf("工具 %d 即将超时\n", i + 1);
                // 可以添加警告提示
            }
        }
    }
}

// ========== 自动检测RFID归还 ==========
void checkAutoReturn() {
    // 只在主菜单状态下检测自动归还
    if (g_state != AppState::MAIN_MENU) return;
    
    uint8_t uid[4];
    if (rfid.readUID(uid)) {
        uint32_t read_uid = ((uint32_t)uid[0] << 24) |
                           ((uint32_t)uid[1] << 16) |
                           ((uint32_t)uid[2] << 8) |
                           ((uint32_t)uid[3]);
        
        uint8_t tool_index;
        if (IsValidToolUID(read_uid, &tool_index)) {
            // 检查工具是否已借出
            if (g_tools[tool_index].is_borrowed) {
                // 自动归还
                char rfid_str[9];
                snprintf(rfid_str, sizeof(rfid_str), "%08X", read_uid);
                
                Serial.printf("自动检测到工具 %d 归还\n", tool_index + 1);
                
                g_tools[tool_index].is_borrowed = false;
                SaveToolStatus();
                
                // 上报归还事件
                if (network.isConnected()) {
                    network.publishReturnEvent(tool_index, String(rfid_str), getCurrentTimeString());
                }
                
                // 提示归还成功
                display.showMessage("Auto Return", false);
                buzzer.beepSuccess();
                delay(1000);
                display.showMainMenu(0);
                
                // 记录日志
                storage.addRecord("AUTO_RETURN", g_tools[tool_index].name);
                
                updateActivityTime();
            }
        }
    }
}

// ========== 进入深度睡眠 ==========
void enterDeepSleep() {
    Serial.println("进入深度睡眠模式...");
    
    // 保存状态
    storage.saveAllToolsStatus();
    
    // 关闭外设
    display.showMessage("Sleep...", false);
    delay(500);
    display.off();
    ws2812.off();
    buzzer.off();
    
    // 配置唤醒源（GPIO唤醒）
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0); // 门磁传感器唤醒
    
    // 进入深度睡眠
    esp_deep_sleep_start();
}

// ========== 网络连接尝试 ==========
void tryConnectNetwork() {
    unsigned long currentTime = millis();
    
    static unsigned long network_last_attempt = 0;
    static uint8_t network_attempt_count = 0;
    
    // 限制尝试频率
    if (currentTime - network_last_attempt < 2000 && network_attempt_count > 0) {
        return;
    }
    
    network_last_attempt = currentTime;
    network_attempt_count++;
    
    Serial.printf("网络连接尝试 %d\n", network_attempt_count);
    
    // 首次尝试显示连接界面
    if (network_attempt_count == 1) {
        display.showConnectingScreen();
        ws2812.showWiFiConnecting();
    }
    
    // 尝试连接网络
    if (network.connect()) {
        display.showConnectedScreen();
        ws2812.showWiFiConnected();
        buzzer.beepSuccess();
        Serial.println("网络连接成功");
        wifi_connected = true;
        mqtt_connected = true;
        network_attempt_count = 0;
        
        // 上报所有属性
        network.publishAllProperties();
        
        // 显示1秒后自动进入主菜单
        delay(1000);
        g_menu_selection = 0;  // 重置菜单选择
        g_state = AppState::MAIN_MENU;
        display.showMainMenu(0);
    } else if (network_attempt_count >= 3) {
        // 3次失败后进入离线模式
        display.showMessage("Offline Mode", true);
        delay(1000);
        Serial.println("网络连接失败，进入离线模式");
        network.setSystemStatus(3); // 离线模式
        network_attempt_count = 0;
        g_menu_selection = 0;  // 重置菜单选择
        g_state = AppState::MAIN_MENU;
        display.showMainMenu(0);
    }
}

// ========== 返回主菜单辅助函数 ==========
void returnToMainMenu() {
    g_menu_selection = 0;  // 重置菜单选择到"借用"
    g_state = AppState::MAIN_MENU;
    display.showMainMenu(0);
}

// ========== 处理RFID扫描 ==========
void processRFIDScan(bool isBorrow) {
    uint8_t uid[4];
    if (rfid.readUID(uid)) {
        uint32_t read_uid = ((uint32_t)uid[0] << 24) |
                           ((uint32_t)uid[1] << 16) |
                           ((uint32_t)uid[2] << 8) |
                           ((uint32_t)uid[3]);
        
        // 转换为16进制字符串
        char rfid_str[9];
        snprintf(rfid_str, sizeof(rfid_str), "%08X", read_uid);
        
        Serial.printf("读取RFID UID: %s\n", rfid_str);
        
        uint8_t tool_index;
        if (IsValidToolUID(read_uid, &tool_index)) {
            // RFID验证成功
            display.showMessage("RFID OK", false);
            buzzer.beepSuccess();
            delay(500);
            
            if (isBorrow) {
                // === 借用模式 ===
                if (tool_index != g_selected_tool_index) {
                    // 拿错工具
                    display.showMessage("Wrong Tool", false);
                    buzzer.beepError();
                    delay(1500);
                    display.showRFIDScreen(g_selected_tool_index);
                    return;
                }
                
                // 验证通过，更新工具状态
                g_tools[tool_index].is_borrowed = true;
                SaveToolStatus();
                
                // 上报到OneNet
                if (network.isConnected()) {
                    network.publishBorrowEvent(tool_index, String(rfid_str), getCurrentTimeString());
                }
                
                // 显示借用成功
                display.showMessage("Take Tool", false);
                buzzer.beepSuccess();
                delay(1000);
                
                // 进入等待关门状态
                g_state = AppState::WAIT_DOOR_CLOSE;
                display.showMessage("Close Door", false);
                
                // 记录日志
                storage.addRecord("BORROW", g_tools[tool_index].name);
                
            } else {
                // === 归还模式 ===
                if (tool_index != g_selected_tool_index) {
                    // 扫描的工具与选择的不符
                    display.showMessage("Wrong Tool", false);
                    buzzer.beepError();
                    delay(1500);
                    display.showRFIDScreen(g_selected_tool_index);  // 继续等待正确的RFID
                    return;
                }
                
                // 验证通过，更新工具状态
                g_tools[tool_index].is_borrowed = false;
                SaveToolStatus();
                
                // 上报到OneNet
                if (network.isConnected()) {
                    network.publishReturnEvent(tool_index, String(rfid_str), getCurrentTimeString());
                }
                
                // 显示归还成功
                display.showMessage("Return OK", false);
                buzzer.beepSuccess();
                delay(1000);
                
                // 进入等待关门状态
                g_state = AppState::WAIT_DOOR_CLOSE;
                display.showMessage("Close Door", false);
                
                // 记录日志
                storage.addRecord("RETURN", g_tools[tool_index].name);
            }
            
        } else {
            // RFID验证失败
            Serial.println("RFID验证失败: 无效工具");
            display.showMessage("Invalid", false);
            buzzer.beepError();
            delay(1500);
            
            // 返回原状态
            display.showRFIDScreen(g_selected_tool_index);
        }
    }
}

// ========== 处理远程开锁请求 ==========
void handleRemoteUnlock() {
    if (!g_RemoteUnlockRequested) return;

    Serial.printf("执行远程开锁: 工具 %d, 时长 %d秒, 模式 %s\n", 
                  g_RemoteUnlockToolIndex + 1, g_RemoteUnlockDuration,
                  g_RemoteUnlockMode ? "借用" : "归还");
    
    // 借用模式：检查工具是否已被借出
    if (g_RemoteUnlockMode == 1 && g_tools[g_RemoteUnlockToolIndex].is_borrowed) {
        Serial.println("错误: 工具已被借出，拒绝开锁");
        display.showMessage("Already Out", false);
        buzzer.beepError();
        delay(1500);
        display.showMainMenu(0);
        network.publishErrorCode(1);
        g_RemoteUnlockRequested = false;
        return;
    }
    
    // 归还模式：检查工具是否在库
    if (g_RemoteUnlockMode == 0 && !g_tools[g_RemoteUnlockToolIndex].is_borrowed) {
        Serial.println("错误: 工具在库中，无需归还");
        display.showMessage("Already In", false);
        buzzer.beepError();
        delay(1500);
        display.showMainMenu(0);
        network.publishErrorCode(2);
        g_RemoteUnlockRequested = false;
        return;
    }

    // 执行开锁
    if (g_RemoteUnlockMode == 1) {
        display.showMessage("Remote Borrow", false);
    } else {
        display.showMessage("Remote Return", false);
    }
    
    lock.open(); // 通电0.5秒
    buzzer.beepSuccess();
    
    // 继电器切换后重新初始化RFID模块，避免电磁干扰
    delay(50);  // 等待电压稳定
    rfid.init();
    delay(50);
    
    // 更新门状态
    network.setDoorStatus(true);
    network.setLockStatus(false); // 锁开启
    
    if (network.isConnected()) {
        network.publishDoorStatus(true);
        network.publishLockStatus(false);
    }
    
    g_door_auth_time = millis();
    g_DoorUnlockAuthorized = true;
    g_selected_tool_index = g_RemoteUnlockToolIndex;
    
    // 根据模式进入不同的RFID等待状态
    if (g_RemoteUnlockMode == 1) {
        g_state = AppState::WAIT_RFID_FOR_BORROW;
        display.showRFIDScreen(g_selected_tool_index);
    } else {
        g_state = AppState::WAIT_RFID_FOR_RETURN;
        display.showRFIDScreen(g_selected_tool_index);
    }

    g_RemoteUnlockRequested = false;
}

// ========== 处理用户输入 ==========
void handleUserInput() {
    ButtonAction action = buttons.getButtonAction();
    
    switch (g_state) {
        case AppState::INITIALIZING:
            // 不应在此状态
            break;
            
        case AppState::CONNECTING:
            // 网络连接中，处理取消
            if (action == ButtonAction::BACK_PRESSED) {
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            break;
            
        case AppState::MAIN_MENU:
            // 主菜单状态
            if (action == ButtonAction::ENCODER_ROTATE_CW) {
                g_menu_selection = (g_menu_selection + 1) % 2;
                display.showMainMenu(g_menu_selection);
            } 
            else if (action == ButtonAction::ENCODER_ROTATE_CCW) {
                g_menu_selection = (g_menu_selection - 1 + 2) % 2;
                display.showMainMenu(g_menu_selection);
            }
            else if (action == ButtonAction::ENCODER_PRESSED) {
                // 根据选择进入相应模式
                if (g_menu_selection == 0) {
                    // 借用模式
                    g_state = AppState::BORROW_TOOL_SELECT;
                    g_selected_tool_index = 0;
                    display.showToolSelect(g_selected_tool_index, true);
                } else {
                    // 归还模式
                    g_state = AppState::RETURN_TOOL_SELECT;
                    g_selected_tool_index = 0;
                    display.showToolSelect(g_selected_tool_index, false);
                }
            }
            else if (action == ButtonAction::BACK_LONG_PRESSED) {
                // 长按查看状态
                display.showStatusScreen();
            }
            else if (action == ButtonAction::BACK_PRESSED) {
                // 短按无操作
            }
            break;
            
        case AppState::BORROW_TOOL_SELECT:
            // 借用工具选择
            if (action == ButtonAction::ENCODER_ROTATE_CW) {
                g_selected_tool_index = (g_selected_tool_index + 1) % TOTAL_TOOLS;
                display.showToolSelect(g_selected_tool_index, true);
            }
            else if (action == ButtonAction::ENCODER_ROTATE_CCW) {
                g_selected_tool_index = (g_selected_tool_index - 1 + TOTAL_TOOLS) % TOTAL_TOOLS;
                display.showToolSelect(g_selected_tool_index, true);
            }
            else if (action == ButtonAction::ENCODER_PRESSED) {
                // 确认选择，进入确认界面
                g_state = AppState::BORROW_CONFIRM;
                display.showConfirmScreen(g_selected_tool_index, true);
            }
            else if (action == ButtonAction::BACK_PRESSED) {
                // 返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            else if (action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 旋钮长按直接返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            break;
            
        case AppState::BORROW_CONFIRM:
            // 借用确认界面
            if (action == ButtonAction::BACK_PRESSED) {
                // 返回工具选择
                g_state = AppState::BORROW_TOOL_SELECT;
                display.showToolSelect(g_selected_tool_index, true);
            }
            else if (action == ButtonAction::ENCODER_PRESSED) {
                // 确认借用，先检查是否已被借出
                if (g_tools[g_selected_tool_index].is_borrowed) {
                    // 工具已被借走
                    display.showMessage("Borrowed", false);
                    buzzer.beepError();
                    delay(1500);
                    // 返回工具选择
                    g_state = AppState::BORROW_TOOL_SELECT;
                    display.showToolSelect(g_selected_tool_index, true);
                } else {
                    // 开门并进入RFID扫描
                    display.showMessage("Open...", false);
                    lock.open();  // 使用现有的open函数（自动0.5秒后关闭）
                    
                    // 继电器切换后重新初始化RFID模块
                    delay(50);
                    rfid.init();
                    delay(50);
                    
                    // 更新门和锁状态
                    network.setDoorStatus(true);
                    network.setLockStatus(false);
                    
                    if (network.isConnected()) {
                        network.publishDoorStatus(true);
                        network.publishLockStatus(false);
                    }
                    
                    g_door_auth_time = millis();
                    g_DoorUnlockAuthorized = true;
                    
                    g_state = AppState::WAIT_RFID_FOR_BORROW;
                    display.showRFIDScreen(g_selected_tool_index);
                }
            }
            else if (action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 长按返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            break;

        case AppState::RETURN_TOOL_SELECT:
            // 归还工具选择
            if (action == ButtonAction::ENCODER_ROTATE_CW) {
                g_selected_tool_index = (g_selected_tool_index + 1) % TOTAL_TOOLS;
                display.showToolSelect(g_selected_tool_index, false);
            }
            else if (action == ButtonAction::ENCODER_ROTATE_CCW) {
                g_selected_tool_index = (g_selected_tool_index - 1 + TOTAL_TOOLS) % TOTAL_TOOLS;
                display.showToolSelect(g_selected_tool_index, false);
            }
            else if (action == ButtonAction::ENCODER_PRESSED) {
                // 确认选择，进入确认界面
                g_state = AppState::RETURN_CONFIRM;
                display.showConfirmScreen(g_selected_tool_index, false);
            }
            else if (action == ButtonAction::BACK_PRESSED) {
                // 返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            else if (action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 旋钮长按直接返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            break;
            
        case AppState::RETURN_CONFIRM:
            // 归还确认界面
            if (action == ButtonAction::BACK_PRESSED) {
                // 返回工具选择
                g_state = AppState::RETURN_TOOL_SELECT;
                display.showToolSelect(g_selected_tool_index, false);
            }
            else if (action == ButtonAction::ENCODER_PRESSED) {
                // 确认归还，先开门
                display.showMessage("Open...", false);
                lock.open();  // 使用现有的open函数（自动0.5秒后关闭）
                
                // 继电器切换后重新初始化RFID模块
                delay(50);
                rfid.init();
                delay(50);
                
                // 更新门和锁状态
                network.setDoorStatus(true);
                network.setLockStatus(false);
                
                if (network.isConnected()) {
                    network.publishDoorStatus(true);
                    network.publishLockStatus(false);
                }
                
                g_door_auth_time = millis();
                g_DoorUnlockAuthorized = true;
                
                g_state = AppState::WAIT_RFID_FOR_RETURN;
                display.showRFIDScreen(g_selected_tool_index);
            }
            else if (action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 长按返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            break;
            
        case AppState::WAIT_RFID_FOR_BORROW:
            // 等待借用RFID扫描
            if (action == ButtonAction::BACK_PRESSED) {
                // 返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            else if (action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 长按返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            else {
                // 处理RFID扫描
                processRFIDScan(true);
            }
            break;
            
        case AppState::WAIT_RFID_FOR_RETURN:
            // 等待归还RFID扫描
            if (action == ButtonAction::BACK_PRESSED) {
                // 返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            else if (action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 长按返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            else {
                // 处理RFID扫描
                processRFIDScan(false);
            }
            break;
            
        case AppState::WAIT_DOOR_CLOSE:
            // 等待关门状态，不响应空闲超时
            if (action == ButtonAction::ENCODER_PRESSED || 
                action == ButtonAction::BACK_PRESSED ||
                action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 任何按钮操作返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
            }
            
            // 检查门是否已关闭
            if (!door.isOpen()) {
                // 门已关闭，更新状态
                network.setDoorStatus(false);
                network.setLockStatus(true); // 锁关闭
                
                if (network.isConnected()) {
                    network.publishDoorStatus(false);
                    network.publishLockStatus(true);
                }
                
                // 返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
                Serial.println("门已关闭，返回主菜单");
            }
            break;
            
        case AppState::ALARM:
            // 报警状态
            if (action == ButtonAction::ENCODER_PRESSED || 
                action == ButtonAction::BACK_PRESSED ||
                action == ButtonAction::ENCODER_LONG_PRESSED) {
                // 任何按钮操作返回主菜单
                g_state = AppState::MAIN_MENU;
                display.showMainMenu(0);
                g_AlarmTriggered = false;
                network.setAlarmStatus(false);
                if (network.isConnected()) {
                    network.publishAlarmStatus(false);
                }
                buzzer.off();
                ws2812.showSystemReady();
            }
            break;
    }
}

// ========== 检查非法开门 ==========
void checkIllegalOpen() {
    static bool last_door_state = false;
    
    if (door.stateChanged()) {
        bool door_open = door.isOpen();
        
        if (door_open) {
            // 门被打开
            if (!g_DoorUnlockAuthorized) {
                // 未授权开门，触发报警
                g_AlarmTriggered = true;
                alarm_count = millis();
                
                Serial.println("报警: 未授权开门!");
                display.showAlarmScreen();
                buzzer.on();
                ws2812.showAlarm();
                
                // 更新报警状态
                network.setAlarmStatus(true);
                
                // 上报到OneNet
                if (network.isConnected()) {
                    network.publishAlarmStatus(true);
                    network.publishIllegalOpenEvent(1, 0); // 1=未授权开门
                }
                
                // 记录日志
                storage.addRecord("ALARM", "", "Unauthorized door opening");
                
                g_state = AppState::ALARM;
            } else {
                // 授权开门
                Serial.println("授权开门");
                // 门状态已在开门时更新
            }
        } else {
            // 门被关闭
            if (g_DoorUnlockAuthorized) {
                // 授权开门后门关闭，重置授权
                g_DoorUnlockAuthorized = false;
                g_door_auth_time = 0;
                
                // 门状态已在handleUserInput中更新
            }
            
            if (g_AlarmTriggered) {
                // 报警状态下门关闭，停止报警
                g_AlarmTriggered = false;
                network.setAlarmStatus(false);
                
                if (network.isConnected()) {
                    network.publishAlarmStatus(false);
                }
                
                buzzer.off();
                ws2812.showSystemReady();
                
                if (g_state == AppState::ALARM) {
                    g_state = AppState::MAIN_MENU;
                    display.showMainMenu(0);
                }
            }
        }
        
        last_door_state = door_open;
    }
    
    // 检查开门超时
    if (g_DoorUnlockAuthorized && door.isOpen()) {
        unsigned long open_duration = millis() - g_door_auth_time;
        if (open_duration > DOOR_OPEN_TIMEOUT) {
            if (!g_AlarmTriggered) {
                Serial.println("警告: 开门超时，触发报警");
                g_AlarmTriggered = true;
                alarm_count = millis();
                
                display.showAlarmScreen();
                buzzer.on();
                ws2812.showAlarm();
                
                // 更新报警状态
                network.setAlarmStatus(true);
                
                // 上报到OneNet
                if (network.isConnected()) {
                    network.publishAlarmStatus(true);
                    network.publishIllegalOpenEvent(2, open_duration / 1000); // 2=开门超时
                }
                
                g_state = AppState::ALARM;
            }
        }
    }
}

// ========== Arduino主循环 ==========
void loop() {
    // 更新按钮状态
    buttons.update();
    
    // 检测按钮活动
    if (buttons.hasActivity()) {
        updateActivityTime();
    }
    
    static unsigned long last_network_check = 0;
    if (!network.isConnected() && g_state != AppState::CONNECTING) {
        // 限制网络尝试频率（每2秒检查一次）
        if (millis() - last_network_check > 2000) {
            last_network_check = millis();
            tryConnectNetwork();
        }
    } else if (network.isConnected()) {
        network.loop();
    }
    
    // 状态机处理
    handleRemoteUnlock();
    handleUserInput();
    
    // 检查非法开门
    checkIllegalOpen();
    
    // 检查借用超时
    checkBorrowTimeout();
    
    // 自动检测RFID归还（主菜单状态下）
    checkAutoReturn();
    
    // 检查空闲超时（排除报警和等待关门状态）
    if (display.isIdle() && g_state != AppState::ALARM && g_state != AppState::WAIT_DOOR_CLOSE) {
        // 空闲超时，返回主菜单
        if (g_state != AppState::MAIN_MENU) {
            g_state = AppState::MAIN_MENU;
            display.showMainMenu(0);
            Serial.println("空闲超时，返回主菜单");
        }
    }
    
    // 检查是否进入低功耗模式（5分钟无操作）
    // 注意：实际使用时可以取消注释
    // if (millis() - g_last_activity_time > SLEEP_TIMEOUT && 
    //     g_state == AppState::MAIN_MENU && 
    //     !g_AlarmTriggered) {
    //     enterDeepSleep();
    // }
    
    // 短暂延迟
    delay(10);
}

// ========== Arduino setup函数 ==========
void setup() {
    systemInit();
}