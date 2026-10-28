/**
 * @file    Key.c
 * @brief   按键驱动程序
 * @details 实现按键的初始化、扫描、消抖、长按、连击检测功能
 *          硬件连接：KEY1-PB8, KEY2-PB7, KEY3-PB6, KEY4-PB5
 *          按键按下为低电平（上拉输入）
 */

#include "key.h"

/* ==================== 按键状态标志变量 ==================== */
// 长按标志：1=检测到长按
u8 key1_long_flag,key2_long_flag,key3_long_flag,key4_long_flag;
// 自锁标志：1=按键已按下且未释放，防止重复触发
u8 key1_lock_flag,key2_lock_flag,key3_lock_flag,key4_lock_flag;
// 按键计数器：用于消抖和长按计时
u16 key1_cnt,key2_cnt,key3_cnt,key4_cnt;
// 连击计数器：用于连击功能计时
u16 key1_cnt2,key2_cnt2,key3_cnt2,key4_cnt2;

// KEY1短按标志：1=检测到短按
u8 key1_short_flag = 0;

/* ==================== 全局变量定义 ==================== */
// 当前按键键值：0=无按键按下
u8 KeyNum = 0;

/**
  * @brief  按键初始化函数
  * @param  无
  * @retval 无
  * @note   配置PB5-PB8为上拉输入模式，按键按下为低电平
  */
void Key_Init(void)
{
	// 使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 配置GPIO结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	// 上拉输入模式：按键按下为低电平，释放为高电平
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	// 按键引脚：KEY1-PB8, KEY2-PB7, KEY3-PB6, KEY4-PB5
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN | KEY2_GPIO_PIN | KEY3_GPIO_PIN | KEY4_GPIO_PIN;
	// 输出速度（输入模式下无效，但需要设置）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 初始化GPIO
	GPIO_Init(KEY_PROT, &GPIO_InitStructure);
}

/**
  * @brief  按键扫描函数
  * @param  无
  * @retval 无
  * @note   需要在定时器中断中周期性调用（如2ms一次）
  *         实现功能：消抖、短按、长按、连击检测
  */
void Key_scan(void)
{
	/* ==================== KEY1扫描 ==================== */
	if(KEY1)  // 如果KEY1没有按下（高电平）
	{
		key1_lock_flag = 0;  // 清零自锁标志，允许下次触发
		key1_cnt = 0;        // 清零计数器
		if(key1_long_flag)   // 如果之前有长按标志
		{
			key1_long_flag = 0;  // 清零长按标志
			KeyNum = 11;         // 赋键值编码：KEY1长按
		}
		if(key1_short_flag)  // 如果之前有短按标志
		{
			key1_short_flag = 0;  // 清零短按标志
			KeyNum = 1;           // 赋键值编码：KEY1短按
		}
	}
	else if(!key1_lock_flag)  // KEY1按下且未自锁
	{
		key1_cnt++;  // 累计消抖延时次数
		if(key1_cnt > KEY_DELAY_TIME)  // 达到消抖时间
		{
			key1_cnt = 0;           // 清零计数器
			key1_short_flag = 1;    // 置短按标志
			key1_lock_flag = 1;     // 自锁标志置1，防止多次触发
		}
	}
	else  // KEY1按下且已自锁（继续按住）
	{
		key1_cnt++;  // 累计长按计时

		if(key1_cnt > KEY1_LONG_TIME)  // 达到长按时间
		{
			key1_short_flag = 0;    // 清零短按标志
			key1_long_flag = 1;     // 置长按标志
		}
	}
	
	/* ==================== KEY2扫描 ==================== */
	if(KEY2)  // 如果KEY2没有按下（高电平）
	{
		key2_lock_flag = 0;  // 清零自锁标志
		key2_cnt = 0;        // 清零计数器
		if(key2_long_flag)   // 如果之前有长按标志
		{
			key2_long_flag = 0;  // 清零长按标志
			KeyNum = 22;         // 赋键值编码：KEY2长按
		}
	}
	else if(!key2_lock_flag)  // KEY2按下且未自锁
	{
		key2_cnt++;  // 累计消抖延时次数
		if(key2_cnt > KEY_DELAY_TIME)  // 达到消抖时间
		{
			key2_cnt = 0;       // 清零计数器
			KeyNum = 2;         // 赋键值编码：KEY2短按
			key2_lock_flag = 1; // 自锁标志置1，防止多次触发
		}
	}
	else if(key2_cnt < KEY_Continue_TIME)  // 未达到连击触发时间
	{
		key2_cnt++;  // 累计连击触发时间
	}
	else  // 达到连击触发时间，开始连击
	{
		key2_cnt++;   // 累计长按计时
		key2_cnt2++;  // 累计连击延迟时间
		if(key2_cnt2 > KEY_Continue_Trigger_TIME)  // 达到连击触发间隔
		{
			key2_cnt2 = 0;  // 清零连击延迟时间
			KeyNum = 2;     // 赋连击编码键值
		}
		if(key2_cnt > KEY_LONG_TIME)  // 达到长按时间
		{
			key2_long_flag = 1;  // 置长按标志
		}
	}
	
	/* ==================== KEY3扫描 ==================== */
	if(KEY3)  // 如果KEY3没有按下（高电平）
	{
		key3_lock_flag = 0;  // 清零自锁标志
		key3_cnt = 0;        // 清零计数器
		if(key3_long_flag)   // 如果之前有长按标志
		{
			key3_long_flag = 0;  // 清零长按标志
			KeyNum = 33;         // 赋键值编码：KEY3长按
		}
	}
	else if(!key3_lock_flag)  // KEY3按下且未自锁
	{
		key3_cnt++;  // 累计消抖延时次数
		if(key3_cnt > KEY_DELAY_TIME)  // 达到消抖时间
		{
			key3_cnt = 0;       // 清零计数器
			KeyNum = 3;         // 赋键值编码：KEY3短按
			key3_lock_flag = 1; // 自锁标志置1，防止多次触发
		}
	}
	else if(key3_cnt < KEY_Continue_TIME)  // 未达到连击触发时间
	{
		key3_cnt++;  // 累计连击触发时间
	}
	else  // 达到连击触发时间，开始连击
	{
		key3_cnt++;   // 累计长按计时
		key3_cnt2++;  // 累计连击延迟时间
		if(key3_cnt2 > KEY_Continue_Trigger_TIME)  // 达到连击触发间隔
		{
			key3_cnt2 = 0;  // 清零连击延迟时间
			KeyNum = 3;     // 赋连击编码键值
		}
		if(key3_cnt > KEY_LONG_TIME)  // 达到长按时间
		{
			key3_long_flag = 1;  // 置长按标志
		}
	}
	
	/* ==================== KEY4扫描 ==================== */
	if(KEY4)  // 如果KEY4没有按下（高电平）
	{
		key4_lock_flag = 0;  // 清零自锁标志
		key4_cnt = 0;        // 清零计数器
		if(key4_long_flag)   // 如果之前有长按标志
		{
			key4_long_flag = 0;  // 清零长按标志
			KeyNum = 44;         // 赋键值编码：KEY4长按
		}
	}
	else if(!key4_lock_flag)  // KEY4按下且未自锁
	{
		key4_cnt++;  // 累计消抖延时次数
		if(key4_cnt > KEY_DELAY_TIME)  // 达到消抖时间
		{
			key4_cnt = 0;       // 清零计数器
			KeyNum = 4;         // 赋键值编码：KEY4短按
			key4_lock_flag = 1; // 自锁标志置1，防止多次触发
		}
	}
	else if(key4_cnt < KEY_Continue_TIME)  // 未达到连击触发时间
	{
		key4_cnt++;  // 累计连击触发时间
	}
	else  // 达到连击触发时间，开始连击
	{
		key4_cnt++;   // 累计长按计时
		key4_cnt2++;  // 累计连击延迟时间
		if(key4_cnt2 > KEY_Continue_Trigger_TIME)  // 达到连击触发间隔
		{
			key4_cnt2 = 0;  // 清零连击延迟时间
			KeyNum = 4;     // 赋连击编码键值
		}
		if(key4_cnt > KEY_LONG_TIME)  // 达到长按时间
		{
			key4_long_flag = 1;  // 置长按标志
		}
	}
}
