// camera_capture.cpp
#include "camera_capture.h"

CameraCapture camera;

CameraCapture::CameraCapture() : available(false) {
}

void CameraCapture::init() {
    Serial.println("初始化摄像头...");
    
    camera_config_t config;
    
    config.ledc_channel = LEDC_CHANNEL_1;
    config.ledc_timer = LEDC_TIMER_1;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    

    config.xclk_freq_hz = 20000000;    
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("摄像头初始化失败: 0x%x\n", err);
        return;
    }
    
    available = true;
    Serial.println("摄像头初始化成功");
}

bool CameraCapture::capturePhoto() {
    if (!available) return false;
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("拍照失败");
        return false;
    }
    
    Serial.printf("拍照成功: %u 字节\n", fb->len);
    esp_camera_fb_return(fb);
    return true;
}

void CameraCapture::test() {
    if (!available) {
        Serial.println("摄像头不可用");
        return;
    }
    
    Serial.println("测试摄像头...");
    for (int i = 0; i < 3; i++) {
        if (capturePhoto()) {
            Serial.println("  拍照成功");
        } else {
            Serial.println("  拍照失败");
        }
        delay(1000);
    }
}