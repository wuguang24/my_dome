#ifndef __GT1151_H
#define __GT1151_H	
#include "sys.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F407ZGT6开发板
//电容触摸屏-GT1151 驱动代码	   
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 


//IO��������	 
#define GT_RST    		PBout(14)	  //GT1151��λ����
#define GT_INT    		PBin(13)		//GT1151������	
   	
 
//I2C��д����	
#define GT_CMD_WR 		0X28    	//д����
#define GT_CMD_RD 		0X29		//������
  
//GT1151 ���ּĴ������� 
#define GT_CTRL_REG 	0X8040   	//GT1151���ƼĴ���
//#define GT_CFGS_REG 	0X8050   	//GT1151��ַ�Ĵ���
//#define GT_CHECK_REG 	0X813C   	//GT1151��ͼĴ���
#define GT_PID_REG 		0X8140   	//GT1151��ƷID�Ĵ���

#define GT_GSTID_REG 	0X814E   	//GT1151ǰ��⵽�Ĵ������
#define GT_TP1_REG 		0X8150  	//��һ�����������ݵ�ַ
#define GT_TP2_REG 		0X8158		//�ڶ������������ݵ�ַ
#define GT_TP3_REG 		0X8160		//���������������ݵ�ַ
#define GT_TP4_REG 		0X8168		//���ĸ����������ݵ�ַ
#define GT_TP5_REG 		0X8170		//��������������ݵ�ַ  
 
 
u8 GT1151_Send_Cfg(u8 mode);
u8 GT1151_WR_Reg(u16 reg,u8 *buf,u8 len);
void GT1151_RD_Reg(u16 reg,u8 *buf,u8 len); 
u8 GT1151_Init(void);
u8 GT1151_Scan(u8 mode); 
#endif













