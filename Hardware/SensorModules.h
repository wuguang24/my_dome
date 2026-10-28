/**
 * @file    SensorModules.h
 * @brief   传感器模块数据结构定义头文件
 * @details 定义传感器数据结构体和阈值结构体
 */

#ifndef	__SENSORMODULES_H_
#define __SENSORMODULES_H_

#include "stm32f10x.h"                  // STM32F10x标准外设库头文件

/* ==================== 传感器数据结构体 ==================== */
/**
 * @brief  传感器数据结构体
 * @note   存储所有传感器的实时采集数据
 */
typedef struct
{
	uint8_t humi;        // 湿度值：DHT11采集，单位：%
	uint8_t temp;        // 温度值：DHT11采集，单位：℃
	uint8_t waterLevel;  // 水位状态：0=缺水，1=有水
	uint8_t distance;    // 距离值：预留（超声波测距）
	uint8_t people;      // 人体检测：预留（PIR传感器）
	uint16_t lux;        // 光照强度：LDR采集，单位：lux
}SensorModules;

/* ==================== 传感器阈值结构体 ==================== */
/**
 * @brief  传感器阈值结构体
 * @note   存储各传感器的阈值设定值，用于自动控制判断
 */
typedef struct
{
	uint16_t Distance_threshold;      // 距离阈值：预留
	uint16_t Illumination_threshold;  // 光照阈值：用于自动亮度调节，单位：lux
}SensorThresholdValue;

/* ==================== 全局变量声明 ==================== */
// 传感器数据结构体变量（存储实时采集数据）
extern SensorModules sensorData;
// 传感器阈值结构体变量（存储阈值设定值）
extern SensorThresholdValue Sensorthreshold;

#endif
