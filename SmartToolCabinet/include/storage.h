// storage.h - 单工具柜版本
#pragma once
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config.h"

// 用户账号结构
struct UserAccount {
    char username[17];  // 最大16字符 + 结束符
    char password[17];  // 最大16字符 + 结束符
    bool valid;         // 账号是否有效
};

#define MAX_USERS 10  // 最多存储10个用户

class Storage {
private:
    bool mounted;
    UserAccount users[MAX_USERS];  // 用户账号数组
    uint8_t user_count;            // 当前用户数量
    
public:
    Storage();
    void init();
    void load();
    void save();
    void addRecord(const String& operation, const String& tool_name = "", const String& user = "local");
    void clearRecords();
    String getStatistics();
    
    // 工具状态管理函数
    void saveToolStatus(uint8_t tool_index, bool borrowed);
    bool loadToolStatus(uint8_t tool_index);
    void saveAllToolsStatus();
    void loadAllToolsStatus();
    
    // 用户账号管理函数
    bool loadUsers();                                           // 从SPIFFS加载用户
    bool saveUsers();                                           // 保存用户到SPIFFS
    bool addUser(const String& username, const String& password);  // 添加用户
    bool removeUser(const String& username);                    // 删除用户
    bool updatePassword(const String& username, const String& newPassword);  // 修改密码
    bool verifyUser(const String& username, const String& password);  // 验证用户
    bool userExists(const String& username);                    // 检查用户是否存在
    uint8_t getUserCount() { return user_count; }               // 获取用户数量
    String getUsersJson();                                      // 获取所有用户JSON（用于上报云平台）
    void syncUsersFromCloud(const String& accountStr, const String& passwordStr);  // 从云平台同步用户
};

extern Storage storage;