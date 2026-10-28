#ifndef __ALIYUN_CONF_H
#define __ALIYUN_CONF_H

// WiFi
#define ALIYUN_WIFI_SSID        "1406"
#define ALIYUN_WIFI_PASS        "pxzn1406"

// Aliyun IoT
#define ALIYUN_PRODUCT_KEY      "a1XCKQxne7v"
#define ALIYUN_DEVICE_NAME      "ceshi2026"
#define ALIYUN_REGION           "cn-shanghai"

// MQTT server host: <ProductKey>.iot-as-mqtt.<Region>.aliyuncs.com
#define ALIYUN_MQTT_HOST        ALIYUN_PRODUCT_KEY ".iot-as-mqtt." ALIYUN_REGION ".aliyuncs.com"
#define ALIYUN_MQTT_PORT        1883

// MQTT username/password/clientId (directly copied from reference '3-实物程序')
// 用户名
#define ALIYUN_MQTT_USER        ALIYUN_DEVICE_NAME "&" ALIYUN_PRODUCT_KEY
// 密码/鉴权信息
#define ALIYUN_MQTT_PASS        "CD60F7A5FE256D8D04B17F57376685F7970BDEB3"
// clientId
#define ALIYUN_MQTT_CLIENTID    ALIYUN_DEVICE_NAME "|securemode=3,signmethod=hmacsha1,timestamp=1234|"

#endif
