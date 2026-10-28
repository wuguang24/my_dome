#include "encoder.h"
#include "led.h"
#include "MPU6050.h"
#include "hcsr04.h"
uint32_t gpio_interrup1,gpio_interrup2;
int Get_Encoder_countA,Get_Encoder_countB;
/*******************************************************
函数功能：外部中断模拟编码器信号
入口函数：无
返回  值：无
***********************************************************/
static void Encoder_GpioIsrHandler(void)
{
	//获取中断信号
    gpio_interrup1 = DL_GPIO_getEnabledInterruptStatus(ENCODERA_PORT,ENCODERA_E1A_PIN|ENCODERA_E1B_PIN);
    gpio_interrup2 = DL_GPIO_getEnabledInterruptStatus(ENCODERB_PORT,ENCODERB_E2A_PIN|ENCODERB_E2B_PIN);
	uint32_t mpu_intp = DL_GPIO_getEnabledInterruptStatus(MPU6050_PORT, MPU6050_INT_PIN_PIN);
	uint32_t hcsr04_intp = DL_GPIO_getEnabledInterruptStatus(HCSR04_PORT, HCSR04_Echo_PIN);
    
    
	//encoderB
	if((gpio_interrup1 & ENCODERA_E1A_PIN)==ENCODERA_E1A_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERA_PORT,ENCODERA_E1B_PIN))
		{
			Get_Encoder_countA--;
		}
		else
		{
			Get_Encoder_countA++;
		}
	}
	else if((gpio_interrup1 & ENCODERA_E1B_PIN)==ENCODERA_E1B_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERA_PORT,ENCODERA_E1A_PIN))
		{
			Get_Encoder_countA++;
		}
		else
		{
			Get_Encoder_countA--;
		}
	}
	
	//encoderA
	if((gpio_interrup2 & ENCODERB_E2A_PIN)==ENCODERB_E2A_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERB_PORT,ENCODERB_E2B_PIN))
		{
			Get_Encoder_countB--;
		}
		else
		{
			Get_Encoder_countB++;
		}
	}
	else if((gpio_interrup2 & ENCODERB_E2B_PIN)==ENCODERB_E2B_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERB_PORT,ENCODERB_E2A_PIN))
		{
			Get_Encoder_countB++;
		}                 
		else              
		{                 
			Get_Encoder_countB--;
		}
	}

	if( (mpu_intp & MPU6050_INT_PIN_PIN) == MPU6050_INT_PIN_PIN )
	{
		Read_DMP();
		mpu6050.pitch = Pitch;
		mpu6050.roll = Roll;
		mpu6050.yaw = Yaw;
		mpu6050.gyro.x=gyro[0];
		mpu6050.gyro.y=gyro[1];
		mpu6050.gyro.z=gyro[2];
		mpu6050.accel.x=accel[0];
		mpu6050.accel.y=accel[1];
		mpu6050.accel.z=accel[2];
		LED_PulseOn(2);
		DL_GPIO_clearInterruptStatus(MPU6050_PORT,MPU6050_INT_PIN_PIN);
	}
	if ((hcsr04_intp & HCSR04_Echo_PIN) == HCSR04_Echo_PIN)
	{
		uint8_t level_high = (DL_GPIO_readPins(HCSR04_PORT, HCSR04_Echo_PIN) != 0U) ? 1U : 0U;
		HCSR04_AsyncOnEchoEdge(level_high, Systick_getTick());
		DL_GPIO_clearInterruptStatus(HCSR04_PORT, HCSR04_Echo_PIN);
	}
	DL_GPIO_clearInterruptStatus(ENCODERA_PORT,ENCODERA_E1A_PIN|ENCODERA_E1B_PIN);
	DL_GPIO_clearInterruptStatus(ENCODERB_PORT,ENCODERB_E2A_PIN|ENCODERB_E2B_PIN);
}

/*
 * NOTE:
 * Different startup files/toolchains use different IRQ handler symbol names.
 * - Some use GROUP1_IRQHandler (device interrupt group dispatcher)
 * - Some (notably Keil startup variants) use GPIOA_IRQHandler/GPIOB_IRQHandler
 *
 * Provide wrappers to ensure encoder/MPU/HCSR04 Echo GPIO interrupts are always serviced.
 */
void GROUP1_IRQHandler(void)
{
	Encoder_GpioIsrHandler();
}

void GPIOA_IRQHandler(void)
{
	Encoder_GpioIsrHandler();
}

void GPIOB_IRQHandler(void)
{
	Encoder_GpioIsrHandler();
}

