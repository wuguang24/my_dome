// storage.cpp - 单工具柜版本
#include "storage.h"
#include "config.h"
#include "wifi_mqtt.h"

extern ToolInfo g_tools[TOTAL_TOOLS];
extern WiFiMQTT network;

Storage storage;

Storage::Storage() : mounted(false), user_count(0) {
    // 初始化用户数组
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].valid = false;
        memset(users[i].username, 0, sizeof(users[i].username));
        memset(users[i].password, 0, sizeof(users[i].password));
    }
}

void Storage::init() {
    Serial.println("初始化存储...");
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS挂载失败");
        if (network.isConnected()) {
            network.publishErrorCode(6);
        }
        return;
    }
    
    mounted = true;
    loadAllToolsStatus();
    loadUsers();  // 加载用户账号
    Serial.println("存储初始化完成");
}

// ========== 用户账号管理 ==========

bool Storage::loadUsers() {
    if (!mounted) {
        Serial.println("loadUsers: SPIFFS未挂载");
        return false;
    }
    
    File file = SPIFFS.open("/users.json", "r");
    if (!file || file.size() == 0) {
        if (file) file.close();
        
        Serial.println("创建默认账号: 10000001 / 12345678");
        strncpy(users[0].username, "10000001", 16);
        strncpy(users[0].password, "12345678", 16);
        users[0].valid = true;
        user_count = 1;
        saveUsers();
        return true;
    }
    
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.println("用户文件损坏，重建默认账号");
        SPIFFS.remove("/users.json");
        strncpy(users[0].username, "10000001", 16);
        strncpy(users[0].password, "12345678", 16);
        users[0].valid = true;
        user_count = 1;
        saveUsers();
        return true;
    }
    
    user_count = 0;
    JsonArray usersArray = doc["users"];
    for (JsonObject userObj : usersArray) {
        if (user_count >= MAX_USERS) break;
        
        const char* username = userObj["username"];
        const char* password = userObj["password"];
        
        if (username && password) {
            strncpy(users[user_count].username, username, 16);
            strncpy(users[user_count].password, password, 16);
            users[user_count].valid = true;
            user_count++;
        }
    }
    
    Serial.printf("加载 %d 个用户\n", user_count);
    return true;
}

bool Storage::saveUsers() {
    if (!mounted) return false;
    
    StaticJsonDocument<2048> doc;
    JsonArray usersArray = doc.createNestedArray("users");
    
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].valid) {
            JsonObject userObj = usersArray.createNestedObject();
            userObj["username"] = users[i].username;
            userObj["password"] = users[i].password;
        }
    }
    
    doc["count"] = user_count;
    doc["last_update"] = millis() / 1000;
    
    File file = SPIFFS.open("/users.json", "w");
    if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.println("用户数据保存成功");
        return true;
    }
    
    Serial.println("用户数据保存失败");
    return false;
}

bool Storage::addUser(const String& username, const String& password) {
    if (userExists(username)) {
        Serial.println("用户名已存在");
        return false;
    }
    
    // 如果已满，覆盖第一个用户（最早注册的）
    if (user_count >= MAX_USERS) {
        Serial.println("用户已满，覆盖最早的账号");
        // 将所有用户前移一位，腾出最后一个位置
        for (int i = 0; i < MAX_USERS - 1; i++) {
            strcpy(users[i].username, users[i + 1].username);
            strcpy(users[i].password, users[i + 1].password);
            users[i].valid = users[i + 1].valid;
        }
        // 在最后一个位置添加新用户
        strncpy(users[MAX_USERS - 1].username, username.c_str(), 16);
        strncpy(users[MAX_USERS - 1].password, password.c_str(), 16);
        users[MAX_USERS - 1].valid = true;
        
        saveUsers();
        Serial.printf("添加用户(覆盖): %s\n", username.c_str());
        return true;
    }
    
    // 找到空位
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].valid) {
            strncpy(users[i].username, username.c_str(), 16);
            strncpy(users[i].password, password.c_str(), 16);
            users[i].valid = true;
            user_count++;
            
            saveUsers();
            Serial.printf("添加用户: %s\n", username.c_str());
            return true;
        }
    }
    
    return false;
}

bool Storage::removeUser(const String& username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].valid && strcmp(users[i].username, username.c_str()) == 0) {
            users[i].valid = false;
            memset(users[i].username, 0, sizeof(users[i].username));
            memset(users[i].password, 0, sizeof(users[i].password));
            user_count--;
            
            saveUsers();
            Serial.printf("删除用户: %s\n", username.c_str());
            return true;
        }
    }
    return false;
}

bool Storage::updatePassword(const String& username, const String& newPassword) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].valid && strcmp(users[i].username, username.c_str()) == 0) {
            strncpy(users[i].password, newPassword.c_str(), 16);
            
            saveUsers();
            Serial.printf("更新密码: %s\n", username.c_str());
            return true;
        }
    }
    return false;
}

bool Storage::verifyUser(const String& username, const String& password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].valid && 
            strcmp(users[i].username, username.c_str()) == 0 &&
            strcmp(users[i].password, password.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

bool Storage::userExists(const String& username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].valid && strcmp(users[i].username, username.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

String Storage::getUsersJson() {
    // 构建账号字符串（用逗号分隔）
    String accounts = "";
    String passwords = "";
    
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].valid) {
            if (accounts.length() > 0) {
                accounts += ",";
                passwords += ",";
            }
            accounts += users[i].username;
            passwords += users[i].password;
        }
    }
    
    // 直接构建JSON字符串，避免使用StaticJsonDocument占用栈空间
    String output = "{\"account\":\"" + accounts + "\",\"password\":\"" + passwords + "\",\"count\":" + String(user_count) + "}";
    
    return output;
}

void Storage::syncUsersFromCloud(const String& accountStr, const String& passwordStr) {
    // 先统计总共有多少个账号
    int totalCount = 1;
    for (int i = 0; i < accountStr.length(); i++) {
        if (accountStr[i] == ',') totalCount++;
    }
    
    // 如果超过MAX_USERS，跳过前面的，只保留最新的MAX_USERS个
    int skipCount = (totalCount > MAX_USERS) ? (totalCount - MAX_USERS) : 0;
    
    // 清空现有用户
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].valid = false;
    }
    user_count = 0;
    
    // 解析账号字符串（逗号分隔）
    int accStart = 0, pwdStart = 0;
    int accEnd, pwdEnd;
    int currentIndex = 0;
    
    while (accStart < accountStr.length()) {
        accEnd = accountStr.indexOf(',', accStart);
        pwdEnd = passwordStr.indexOf(',', pwdStart);
        
        if (accEnd < 0) accEnd = accountStr.length();
        if (pwdEnd < 0) pwdEnd = passwordStr.length();
        
        // 跳过前面的旧账号
        if (currentIndex >= skipCount && user_count < MAX_USERS) {
            String username = accountStr.substring(accStart, accEnd);
            String password = passwordStr.substring(pwdStart, pwdEnd);
            
            username.trim();
            password.trim();
            
            if (username.length() > 0 && password.length() > 0) {
                strncpy(users[user_count].username, username.c_str(), 16);
                strncpy(users[user_count].password, password.c_str(), 16);
                users[user_count].valid = true;
                user_count++;
            }
        }
        
        accStart = accEnd + 1;
        pwdStart = pwdEnd + 1;
        currentIndex++;
    }
    
    saveUsers();
    Serial.printf("从云平台同步了 %d 个用户\n", user_count);
}

void Storage::loadAllToolsStatus() {
    if (!mounted) return;
    
    File file = SPIFFS.open("/tools_status.json", "r");
    if (!file) {
        Serial.println("无保存的工具状态，使用默认值");
        return;
    }
    
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.println("工具状态文件解析失败");
        return;
    }

    JsonArray tools = doc["tools"];
    for (int i = 0; i < TOTAL_TOOLS && i < tools.size(); i++) {
        g_tools[i].is_borrowed = tools[i];
    }
    
    Serial.println("工具状态加载完成");
}

void Storage::saveAllToolsStatus() {
    if (!mounted) return;
    
    StaticJsonDocument<1024> doc;
    JsonArray tools = doc.createNestedArray("tools");
    
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        tools.add(g_tools[i].is_borrowed);
    }
    
    doc["last_save"] = millis() / 1000;
    
    File file = SPIFFS.open("/tools_status.json", "w");
    if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.println("所有工具状态保存完成");
    } else {
        Serial.println("工具状态保存失败");
        if (network.isConnected()) {  // 检查网络连接
            network.publishErrorCode(6); // 存储异常
        }
    }
}

void Storage::load() {
    // 兼容性函数
    loadAllToolsStatus();
}

void Storage::save() {
    // 兼容性函数
    saveAllToolsStatus();
}

void Storage::saveToolStatus(uint8_t tool_index, bool borrowed) {
    if (tool_index >= TOTAL_TOOLS) return;
    
    g_tools[tool_index].is_borrowed = borrowed;
    saveAllToolsStatus();
    
    Serial.printf("保存工具状态: %s %s\n", 
                  g_tools[tool_index].name, 
                  borrowed ? "借用" : "归还");
}

bool Storage::loadToolStatus(uint8_t tool_index) {
    if (tool_index >= TOTAL_TOOLS) return false;
    
    if (!mounted) init();
    
    return g_tools[tool_index].is_borrowed;
}

void Storage::addRecord(const String& operation, const String& tool_name, const String& user) {
    if (!mounted) return;
    
    File file = SPIFFS.open("/records.txt", FILE_APPEND);
    if (file) {
        String record = String(millis() / 1000) + "," + 
                       operation + "," + 
                       tool_name + "," + 
                       user + "\n";
        file.print(record);
        file.close();
    }
}

void Storage::clearRecords() {
    if (!mounted) return;
    
    SPIFFS.remove("/records.txt");
    Serial.println("记录已清除");
}

String Storage::getStatistics() {
    String stats = "工具借用状态:\n";
    for (int i = 0; i < TOTAL_TOOLS; i++) {
        stats += String(i + 1) + ". " + g_tools[i].name + ": ";
        stats += g_tools[i].is_borrowed ? "已借用" : "可用";
        stats += "\n";
    }
    return stats;
}