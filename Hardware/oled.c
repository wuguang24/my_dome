#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 

#include "board.h"
#include "stdio.h"

uint8_t OLED_GRAM[128][8];	 
/**************************************************************************
Function: Refresh the OLED screen
Input   : none
Output  : none
函数功能：刷新OLED屏幕
入口参数：无
返回  值：无
**************************************************************************/
void OLED_Refresh_Gram(void)
{
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //Set page address (0~7)
		OLED_WR_Byte (0x00,OLED_CMD);      //Set the display location - column low address
		OLED_WR_Byte (0x10,OLED_CMD);      //Set the display location - column height address   
		for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA); 
	}   
}
/**************************************************************************
Function: Write a byte to OLED
Input   : Dat: data/command to write, CMD: data/command flag 0, represents the command;1, represents data
Output  : none
函数功能：向OLED写入一个字节
入口参数：dat:要写入的数据/命令，cmd:数据/命令标志 0,表示命令;1,表示数据
返回  值：无
**************************************************************************/  
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{	
	uint8_t i;			  
	if(cmd)
	  OLED_RS_Set();
	else 
	  OLED_RS_Clr();		  
	for(i=0;i<8;i++)
	{			  
		OLED_SCLK_Clr();
		if(dat&0x80)
		   OLED_SDIN_Set();
		else 
		   OLED_SDIN_Clr();
		OLED_SCLK_Set();
		dat<<=1;   
	}				 		  
	OLED_RS_Set();   	  
} 
/**************************************************************************
Function: Turn on the OLED display
Input   : none
Output  : none
函数功能：开启OLED显示 
入口参数：无
返回  值：无
**************************************************************************/  
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC command
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
/**************************************************************************
Function: Turn off the OLED display
Input   : none
Output  : none
函数功能：关闭OLED显示 
入口参数：无			  
返回  值：无
**************************************************************************/  
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC command
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}	
/**************************************************************************
Function: Screen clear function, clear the screen, the entire screen is black
Input   : none
Output  : none
函数功能：清屏函数,清完屏,整个屏幕是黑色的，和没点亮一样
入口参数：无		  
返回  值：无
**************************************************************************/  
void OLED_Clear(void)  
{  
	uint8_t i,n;  
	for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[n][i]=0X00;  
	OLED_Refresh_Gram();
}
/**************************************************************************
Function: Draw point
Input   : x,y: starting coordinate;T :1, fill,0, empty
Output  : none
函数功能：画点 
入口参数：x,y :起点坐标; t:1,填充,0,清空			  
返回  值：无
**************************************************************************/ 
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t pos,bx,temp=0;
	if(x>127||y>63)return;
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;	    
}
/**************************************************************************
Function: Displays a character at the specified position
Input   : x,y: starting coordinate;chr: character;size: font size;mode:0, anti-white,1, normal
Output  : none
函数功能：在指定位置显示一个字符
入口参数：x,y :起点坐标; chr:字符; size:字体大小; mode:0,反白显示,1,正常显示	   
返回  值：无
**************************************************************************/
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode)
{      			    
	uint8_t temp,t,t1;
	uint8_t y0=y;
	chr=chr-' ';				   
    for(t=0;t<size;t++)
    {   
		if(size==12)temp=oled_asc2_1206[chr][t];
		else temp=oled_asc2_1608[chr][t];	                          
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }          
}
/**************************************************************************
Function: Find m to the NTH power
Input   : m: base number, n: power number
Output  : none
函数功能：求m的n次方的函数
入口参数：m：底数，n：次方数
返回  值：无
**************************************************************************/
uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}

/**************************************************************************
Function: Displays 2 numbers
Input   : x,y: starting coordinate;num: value;len: number of digits;size: font size
Output  : none
函数功能：显示2个数字
入口参数：x,y :起点坐标; num:数值; len:数字的位数; size:字体大小;	 
返回  值：无
**************************************************************************/
void OLED_ShowNumber(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,1);
				continue;
			}else enshow=1; 
			 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1); 
	}
} 
/**************************************************************************
Function: Display string
Input   : x,y: starting coordinate;*p: starting address of the string
Output  : none
函数功能：显示字符串
入口参数：x,y :起点坐标; *p:字符串起始地址 
返回  值：无
**************************************************************************/
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p)
{
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58          
    while(*p!='\0')
    {       
        if(x>MAX_CHAR_POSX){x=0;y+=16;}
        if(y>MAX_CHAR_POSY){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,12,1);	 
        x+=8;
        p++;
    }  
}	 
/**************************************************************************
Function: Initialize the OLED
Input   : none
Output  : none
函数功能：初始化OLED	
入口参数: 无 
返回  值：无
**************************************************************************/	    
void OLED_Init(void)
{ 	
	OLED_RST_Clr();
	delay_ms(120);

	OLED_RST_Set(); 
				  
	OLED_WR_Byte(0xAE,OLED_CMD); //Close display
	OLED_WR_Byte(0xD5,OLED_CMD); //Set clock divide ratio
	OLED_WR_Byte(80,OLED_CMD);   
	OLED_WR_Byte(0xA8,OLED_CMD); //Set multiplex ratio
	OLED_WR_Byte(0X3F,OLED_CMD); 
	OLED_WR_Byte(0xD3,OLED_CMD); //Set display offset
	OLED_WR_Byte(0X00,OLED_CMD); 

	OLED_WR_Byte(0x40,OLED_CMD); //Set start line address
													
	OLED_WR_Byte(0x8D,OLED_CMD); //Charge pump setting
	OLED_WR_Byte(0x14,OLED_CMD); 
	OLED_WR_Byte(0x20,OLED_CMD); //Set memory address mode
	OLED_WR_Byte(0x02,OLED_CMD); //Page addressing mode
	OLED_WR_Byte(0xA1,OLED_CMD); //Segment remap
	OLED_WR_Byte(0xC0,OLED_CMD); //Set COM scan direction
	OLED_WR_Byte(0xDA,OLED_CMD); //Set COM hardware pin config
	OLED_WR_Byte(0x12,OLED_CMD); 
	 
	OLED_WR_Byte(0x81,OLED_CMD); //Contrast setting
	OLED_WR_Byte(0xEF,OLED_CMD); 
	OLED_WR_Byte(0xD9,OLED_CMD); //Set pre-charge period
	OLED_WR_Byte(0xf1,OLED_CMD); 
	OLED_WR_Byte(0xDB,OLED_CMD); //Set VCOMH voltage
	OLED_WR_Byte(0x30,OLED_CMD); 

	OLED_WR_Byte(0xA4,OLED_CMD); //Global display on
	OLED_WR_Byte(0xA6,OLED_CMD); //Set normal display mode	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //Open display	 
	OLED_Clear();
}  

/**************************************************************************
Function: Display Chinese character
Input   : x,y: coordinate;no: index in Hzk16;font_width,font_height: size
Output  : none
函数功能：显示汉字	
入口参数: x,y:坐标;no:汉字在Hzk16中的索引;font_width,font_height:字体尺寸
返回  值：无
**************************************************************************/	    
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no,uint8_t font_width,uint8_t font_height)
{     			    
	 uint8_t t, i;
   for(i=0;i<(font_height/8);i++)
	 {
			OLED_Set_Pos(x,y+i);	
			for(t=0;t<font_width;t++)		
			{	
					OLED_WR_Byte(Hzk16[(font_height/8)*no+i][t],OLED_DATA);
			}		
	 }
}	 
/**************************************************************************
Function: Set the display position
Input   : x, y: starting point coordinates
Output  : none
函数功能：设置显示坐标
入口参数: x,y :起点坐标
返回  值：无
**************************************************************************/	  
void OLED_Set_Pos(unsigned char x, unsigned char y)
{ 	
	 OLED_WR_Byte(0xb0+y,OLED_CMD);
	 OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	 OLED_WR_Byte((x&0x0f),OLED_CMD); 
} 


void OLED_RST_Clr(void)
{
	DL_GPIO_clearPins(OLED_RST_PORT,OLED_RST_PIN_RST_PIN);
} 	
void OLED_RST_Set(void)
{
	DL_GPIO_setPins(OLED_RST_PORT,OLED_RST_PIN_RST_PIN);
}  

void OLED_RS_Clr(void)
{
	DL_GPIO_clearPins(OLED_DC_PORT,OLED_DC_PIN_DC_PIN);
}   
void OLED_RS_Set(void) 
{
	DL_GPIO_setPins(OLED_DC_PORT,OLED_DC_PIN_DC_PIN);
} 	

void OLED_SCLK_Clr(void)
{
	DL_GPIO_clearPins(OLED_SCL_PORT,OLED_SCL_PIN_SCL_PIN);
}   
void OLED_SCLK_Set(void)
{
	DL_GPIO_setPins(OLED_SCL_PORT,OLED_SCL_PIN_SCL_PIN);
}   
void OLED_SDIN_Clr(void)
{
	DL_GPIO_clearPins(OLED_SDA_PORT,OLED_SDA_PIN_SDA_PIN);
}   
void OLED_SDIN_Set(void) 
{
	DL_GPIO_setPins(OLED_SDA_PORT,OLED_SDA_PIN_SDA_PIN);
} 	
