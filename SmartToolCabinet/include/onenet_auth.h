// onenet_auth.h
#ifndef ONENET_AUTH_H
#define ONENET_AUTH_H

#include <Arduino.h>
#include <time.h>
#include <MD5Builder.h> // ✅ 使用 Arduino-ESP32 自带的 MD5 工具

class OneNetAuth {
private:
    String product_id;
    String device_id;
    String device_secret;

    // 辅助函数：Base64 编码
    String base64Encode(const uint8_t* input, size_t length) {
        static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        String output;
        int i = 0, j = 0;
        uint8_t char_array_3[3], char_array_4[4];

        while (length--) {
            char_array_3[i++] = *(input++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; i < 4; i++)
                    output += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; j < i + 1; j++)
                output += base64_chars[char_array_4[j]];

            while (i++ < 3)
                output += '=';
        }
        return output;
    }

    // 辅助函数：URL 编码
    String urlEncode(const String& str) {
        String encoded;
        for (int i = 0; i < str.length(); i++) {
            char c = str.charAt(i);
            if (c == '/') {
                encoded += "%2F";
            } else if (c == ' ') {
                encoded += "%20";
            } else {
                encoded += c;
            }
        }
        return encoded;
    }

public:
    OneNetAuth(const String& pid, const String& did, const String& dkey)
        : product_id(pid), device_id(did), device_secret(dkey) {}

    String getUsername() {
        return product_id;
    }

    String generatePassword() {
        // 获取当前 Unix 时间戳（秒）
        time_t now = time(nullptr);
        if (now < 1000000000) { // 简单判断时间是否已同步（>2001年）
            Serial.println("⚠️ 警告：系统时间未同步！");
            // 可以选择返回空或尝试其他策略，但最好确保NTP已工作
            // 这里为了不崩溃，用一个远期时间（仅用于测试！）
            now = 2000000000; 
        }
        unsigned long expire_time = now + 3600; // 1小时后过期

        // 构造资源字符串
        String res = "products/" + product_id + "/devices/" + device_id;
        
        // 构造签名原文 (关键：末尾必须拼接 &password=DeviceSecret)
        String sign_str = "et=" + String(expire_time) + "&method=md5&res=" + res;
        sign_str += "&password=" + device_secret;

        Serial.println("\n[密码生成]");
        Serial.println("  et: " + String(expire_time));
        Serial.println("  res: " + res);
        Serial.println("  sign_str: " + sign_str);

        // === 使用 Arduino 的 MD5Builder 计算 MD5 ===
        MD5Builder md5;
        md5.begin();
        md5.add(sign_str);
        md5.calculate();
        uint8_t digest[16];
        md5.getBytes(digest); 
        // === MD5 计算结束 ===

        // 将二进制摘要转为 Base64 字符串
        String signature = base64Encode(digest, 16);
        
        // URL 编码
        signature.replace("+", "%2B");
        signature.replace("/", "%2F");
        signature.replace("=", "%3D");

        Serial.println("  signature (Base64): " + signature);

        // 构造最终密码
        String password = "version=2018-10-31&res=" + 
                          urlEncode(res) + 
                          "&et=" + String(expire_time) + 
                          "&method=md5&sign=" + signature;

        Serial.println("  最终密码: " + password);
        return password;
    }
};

#endif // ONENET_AUTH_H