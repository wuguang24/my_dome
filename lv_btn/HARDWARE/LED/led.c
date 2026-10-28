/*
 * 模块名称: LED 驱动实现 (led.c)
 * 功能说明:
 *   - 初始化LED0(PA1, TIM2_CH2 PWM)和LED1(PA2, GPIO)
 *   - LED0支持PWM调光(0..500)，LED1支持开关控制
 */
#include "led.h"
#include "stm32f4xx_hal_tim.h"

static TIM_HandleTypeDef htim2;  // TIM2句柄(用于LED0 PWM)
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F407ZGT6开发板
//LED驱动代码	   
//版本：V1.0				  
//////////////////////////////////////////////////////////////////////////////////	

// 初始化LED0(PWM)和LED1(GPIO)
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOA_CLK_ENABLE();           // 开启GPIOA时钟
    __HAL_RCC_TIM2_CLK_ENABLE();            // 开启TIM2时钟

    /* LED0: PA1配置为TIM2_CH2 PWM输出 */
    GPIO_Initure.Pin = GPIO_PIN_1;          // PA1
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;    // 复用推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;        // 上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;   // 高速
    GPIO_Initure.Alternate = GPIO_AF1_TIM2; // PA1复用为TIM2_CH2
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);

    /* LED1: PA2配置为普通GPIO输出 */
    GPIO_Initure.Pin = GPIO_PIN_2;          // PA2
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;// 推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;        // 上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;   // 高速
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET); // LED1默认熄灭

    /* 配置TIM2为PWM模式 */
    // 84MHz / (84) / (500) = 2kHz PWM频率
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 84 - 1;          // 预分频: 84MHz -> 1MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 500 - 1;            // 自动重装载: 1MHz / 500 = 2kHz
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim2);

    /* 配置TIM2通道2 (PA1) */
    TIM_OC_InitTypeDef sConfigOC;
    sConfigOC.OCMode = TIM_OCMODE_PWM1;     // PWM1模式
    sConfigOC.Pulse = 0;                    // 初始占空比=0 (灭)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW; // 低电平有效(因为LED是低电平点亮)
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);

    /* 启动PWM输出 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

// LED0 PWM调光: 0(灭)..500(最亮), 低电平有效
void LED0_SetPWM(u16 duty)
{
    if(duty > 500) duty = 500;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

// LED0快捷开关: on=1点亮, on=0熄灭
void LED0_Set(u8 on)
{
    LED0_SetPWM(on ? 500 : 0);  // 全亮或全灭
}

// LED1(绿灯)控制: 低电平点亮
void LED1_Set(u8 on)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
