// camera_capture.h
#pragma once
#include "esp_camera.h"
#include "config.h"

class CameraCapture {
private:
    bool available;
    
public:
    CameraCapture();
    void init();
    bool capturePhoto();
    bool isAvailable() { return available; }
    void test();
};

extern CameraCapture camera;