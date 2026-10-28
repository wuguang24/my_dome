/*
 * 模块名称: GPS(NMEA-0183) 解析 (GPS)
 * 功能概述:
 *   - 通过 UART4 接收 NMEA 字符串(GPRMC/GPGGA/GPVTG)
 *   - 解析经纬度/UTC 时间/速度/状态，缓存为结构体
 *   - 提供初始化/处理/获取/有效性判断接口
 */
#ifndef __GPS_H
#define __GPS_H	 
#include "sys.h"  
#include "uart4.h"

//卫星信息
__packed typedef struct  
{										    
 	uint8_t num;		//卫星编号
	uint8_t eledeg;	//卫星仰角
	uint16_t azideg;	//卫星方位角
	uint8_t sn;		//信噪比		   
}nmea_slmsg;  

//UTC时间信息
__packed typedef struct  
{										    
 	uint16_t year;	//年
	uint8_t month;	//月
	uint8_t date;	//日
	uint8_t hour; 	//小时
	uint8_t min; 	//分钟
	uint8_t sec; 	//秒
}nmea_utc_time;   

//NMEA 0183 协议解析后的数据存储结构体
__packed typedef struct  
{										    
 	uint8_t svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
	uint32_t latitude;				//纬度 放大了100000倍,实际要除以100000
	uint8_t nshemi;					//南纬/北纬,N:北纬;S:南纬				  
	uint32_t longitude;			    //经度 放大了100000倍,实际要除以100000
	uint8_t ewhemi;					//东经/西经,E:东经;W:西经
	uint8_t gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
 	uint8_t posslnum;				//参与定位的卫星数,0~12.
 	uint8_t possl[12];				//参与定位的卫星编号
	uint8_t fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
	uint16_t pdop;					//位置精度因子 0~500,对应实际值0~50.0
	uint16_t hdop;					//水平精度因子 0~500,对应实际值0~50.0
	uint16_t vdop;					//垂直精度因子 0~500,对应实际值0~50.0 

	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	uint16_t speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时	 
}nmea_msg; 

// GPS函数声明
void GPS_Init(void);                  // 初始化 UART4 与内部缓存
void GPS_Frame_Check(void);           // 帧超时检测，需在1ms定时器中调用
void GPS_Process_Data(void);          // 轮询处理：从 UART4_RX_BUF 读取并解析
nmea_msg* GPS_Get_Data(void);         // 获取解析后的数据指针
uint8_t GPS_Is_Valid(void);           // 当前数据是否有效(已定位)

#endif

