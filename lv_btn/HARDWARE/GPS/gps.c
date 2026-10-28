/*
 * 模块名称: GPS NMEA 解析实现 (gps.c)
 * 功能说明:
 *   - 从 UART4 接收 NMEA-0183 字符串，解析 GPRMC/GPGGA/GPVTG
 *   - 提取经纬度、UTC 时间、定位状态、地速等，写入 gps_data
 *   - 提供 GPS_Is_Valid() 判断当前是否有效定位
 */
#include "gps.h"
#include "string.h"
#include "uart4.h"
#include <stdio.h>

// GPS 数据结构缓存
static nmea_msg gps_data; 
// 是否有效定位标志
static uint8_t gps_valid = 0; 

// 查找第 cx 个逗号的位置(从 buf 起始处计数)
uint8_t NMEA_Comma_Pos(uint8_t *buf, uint8_t cx)
{ 		    
    uint8_t *p = buf;
    while(cx)
    { 		 
        if(*buf == '*' || *buf < ' ' || *buf > 'z') return 0XFF; // 遇到'*'或非法字符 -> 不存在第 cx 个逗号
        if(*buf == ',') cx--;
        buf++;
    }
    return buf - p; 	 
}

// m^n 函数(无浮点)
uint32_t NMEA_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1; 	 
    while(n--) result *= m;    
    return result;
}

// 将以 ',' 或 '*' 结束的数字串转为整数(返回值放大 10^flen)
int NMEA_Str2num(uint8_t *buf, uint8_t *dx)
{
    uint8_t *p = buf;
    uint32_t ires = 0, fres = 0;
    uint8_t ilen = 0, flen = 0, i;
    uint8_t mask = 0;
    int res;
    while(1) // 扫描整数/小数长度和符号
    {
        if(*p == '-') { mask |= 0X02; p++; } // 负号
        if(*p == ',' || (*p == '*')) break; // 结束符
        if(*p == '.') { mask |= 0X01; p++; } // 小数点
        else if(*p > '9' || (*p < '0'))	// 非数字
        { 
            ilen = 0;
            flen = 0;
            break;
        } 
        if(mask & 0X01) flen++;
        else ilen++;
        p++;
    }
    if(mask & 0X02) buf++; // 去掉负号
    for(i = 0; i < ilen; i++) // 整数部分
    {  
        ires += NMEA_Pow(10, ilen - 1 - i) * (buf[i] - '0');
    }
    if(flen > 5) flen = 5; // 最多取 5 位小数
    *dx = flen; 		// 输出小数位数
    for(i = 0; i < flen; i++) // 小数部分
    {  
        fres += NMEA_Pow(10, flen - 1 - i) * (buf[ilen + 1 + i] - '0');
    } 
    res = ires * NMEA_Pow(10, flen) + fres;
    if(mask & 0X02) res = -res; 	   
    return res;
}

// 解析 GPRMC 信息: 时间/经纬度/东西南北/日期
void NMEA_GPRMC_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
    uint8_t *p1, dx; 		 
    uint8_t posx;     
    uint32_t temp;    
    float rs;  
    
    p1 = (uint8_t*)strstr((const char *)buf, "GPRMC"); // 查找语句头
    if(p1 == NULL) return;
    
    posx = NMEA_Comma_Pos(p1, 1); // UTC 时间 HHMMSS
    if(posx != 0XFF)
    {
        temp = NMEA_Str2num(p1 + posx, &dx) / NMEA_Pow(10, dx); 	// 去掉毫秒
        gpsx->utc.hour = temp / 10000;
        gpsx->utc.min = (temp / 100) % 100;
        gpsx->utc.sec = temp % 100; 	 
    } 
    posx = NMEA_Comma_Pos(p1, 3); // 纬度 ddmm.mmmm
    if(posx != 0XFF)
    {
        temp = NMEA_Str2num(p1 + posx, &dx); 	 
        gpsx->latitude = temp / NMEA_Pow(10, dx + 2); // 整数部分->度
        rs = temp % NMEA_Pow(10, dx + 2); // 余数->分
        gpsx->latitude = gpsx->latitude * NMEA_Pow(10, 5) + (rs * NMEA_Pow(10, 5 - dx)) / 60; // 转换为度(放大1e5)
    }
    posx = NMEA_Comma_Pos(p1, 4); // N/S
    if(posx != 0XFF) gpsx->nshemi = *(p1 + posx); 		 
    posx = NMEA_Comma_Pos(p1, 5); // 经度 dddmm.mmmm
    if(posx != 0XFF)
    { 						  
        temp = NMEA_Str2num(p1 + posx, &dx); 	 
        gpsx->longitude = temp / NMEA_Pow(10, dx + 2); // 整数部分->度
        rs = temp % NMEA_Pow(10, dx + 2); // 余数->分 	 
        gpsx->longitude = gpsx->longitude * NMEA_Pow(10, 5) + (rs * NMEA_Pow(10, 5 - dx)) / 60; // 转换为度(放大1e5)
    }
    posx = NMEA_Comma_Pos(p1, 6); // E/W
    if(posx != 0XFF) gpsx->ewhemi = *(p1 + posx); 	 
    posx = NMEA_Comma_Pos(p1, 9); // UTC 日期 DDMMYY
    if(posx != 0XFF)
    {
        temp = NMEA_Str2num(p1 + posx, &dx); 				// 解析日期
        gpsx->utc.date = temp / 10000;
        gpsx->utc.month = (temp / 100) % 100;
        gpsx->utc.year = 2000 + temp % 100; 	 
    } 
}

// 解析 GPGGA 信息: 定位状态/参与定位卫星数/海拔等
void NMEA_GPGGA_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
    uint8_t *p1, dx; 		 
    uint8_t posx;    
    p1 = (uint8_t*)strstr((const char *)buf, "$GPGGA"); // 查找语句头
    if(p1 == NULL) return;
    posx = NMEA_Comma_Pos(p1, 6); // GPS 状态
    if(posx != 0XFF) gpsx->gpssta = NMEA_Str2num(p1 + posx, &dx); 
    posx = NMEA_Comma_Pos(p1, 7); // 参与定位卫星数
    if(posx != 0XFF) gpsx->posslnum = NMEA_Str2num(p1 + posx, &dx); 
    posx = NMEA_Comma_Pos(p1, 9); // 海拔高度(0.1m)
    if(posx != 0XFF) gpsx->altitude = NMEA_Str2num(p1 + posx, &dx);  
}

// 解析 GPVTG 信息: 地面速率(km/h*1000)
void NMEA_GPVTG_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
    uint8_t *p1, dx; 		 
    uint8_t posx;    
    p1 = (uint8_t*)strstr((const char *)buf, "$GPVTG"); 				 // 查找语句头
    posx = NMEA_Comma_Pos(p1, 7); // 地面速率 km/h
    if(posx != 0XFF)
    {
        gpsx->speed = NMEA_Str2num(p1 + posx, &dx);
        if(dx < 3) gpsx->speed *= NMEA_Pow(10, 3 - dx); 	 	 	// 统一为 *1000
    }
}

// 读取并解析 NMEA-0183 信息: 顺序解析 GPGGA/GPRMC/GPVTG
void GPS_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
    NMEA_GPGGA_Analysis(gpsx, buf); // GPGGA解析定位状态
    NMEA_GPRMC_Analysis(gpsx, buf); // GPRMC解析经纬度，时间日期
    NMEA_GPVTG_Analysis(gpsx, buf); // GPVTG解析速率
}

// GPS帧接收状态
static uint16_t gps_last_rx_len = 0;    // 上次接收长度
static uint8_t gps_rx_timer = 0;        // 接收超时计数器
static uint8_t gps_frame_ready = 0;     // 帧接收完成标志

// GPS 初始化: 配置 UART4 9600/8N1 并清空缓存
void GPS_Init(void)
{
    uart4_init(9600); // GPS默认9600波特率
    memset(&gps_data, 0, sizeof(nmea_msg));
    gps_valid = 0;
    gps_last_rx_len = 0;
    gps_rx_timer = 0;
    gps_frame_ready = 0;
    printf("[GPS] init OK, UART4@9600\r\n");
}

// GPS帧超时检测，需要在1ms定时器中调用
// 连续50ms没有新数据到达，则认为一帧接收完成
void GPS_Frame_Check(void)
{
    uint16_t current_len = UART4_RX_STA & 0x3FFF;
    
    if(current_len > 0)
    {
        if(current_len > gps_last_rx_len)
        {
            // 有新数据到达，重置超时计数器
            gps_last_rx_len = current_len;
            gps_rx_timer = 0;
        }
        else
        {
            // 没有新数据，累加超时计数
            gps_rx_timer++;
            if(gps_rx_timer >= 50) // 50ms超时，帧接收完成
            {
                gps_frame_ready = 1;
                gps_rx_timer = 0;
            }
        }
    }
}

// 处理 GPS 数据: 使用超时机制判断帧是否完整
void GPS_Process_Data(void)
{
    uint16_t rx_len;
    
    // 检查是否有完整帧
    if(gps_frame_ready)
    {
        gps_frame_ready = 0;
        rx_len = UART4_RX_STA & 0x3FFF;
        
        if(rx_len > 0 && rx_len < UART4_REC_LEN)
        {
            // 禁用UART4中断，防止数据竞争
            HAL_NVIC_DisableIRQ(UART4_IRQn);
            
            UART4_RX_BUF[rx_len] = 0; // 添加结束符，便于 strstr
            GPS_Analysis(&gps_data, UART4_RX_BUF);
            
            // 清空接收状态
            UART4_RX_STA = 0;
            gps_last_rx_len = 0;
            
            // 重新使能UART4中断
            HAL_NVIC_EnableIRQ(UART4_IRQn);
            
            // 判断 GPS 数据是否有效（有定位 且 gpssta>0）
            if(gps_data.gpssta > 0)
            {
                gps_valid = 1;
                printf("[GPS] valid: lat=%ld lon=%ld sta=%d sv=%d\r\n", 
                       gps_data.latitude, gps_data.longitude, gps_data.gpssta, gps_data.posslnum);
            }
            else
            {
                gps_valid = 0;
                printf("[GPS] no fix (sta=%d)\r\n", gps_data.gpssta);
            }
        }
        else
        {
            // 缓冲区溢出或无数据，清空
            HAL_NVIC_DisableIRQ(UART4_IRQn);
            UART4_RX_STA = 0;
            gps_last_rx_len = 0;
            HAL_NVIC_EnableIRQ(UART4_IRQn);
            printf("[GPS] frame error, len=%u\r\n", rx_len);
        }
    }
}

// 获取 GPS 数据指针(供 UI/上报使用)
nmea_msg* GPS_Get_Data(void)
{
    return &gps_data;
}

// 检查 GPS 数据是否有效(是否定位且维持 2D/3D)
uint8_t GPS_Is_Valid(void)
{
    return gps_valid;
}
