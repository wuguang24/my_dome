#include "motor.h"

void Set_PWM(int pwmL,int pwmR)
{
	static int last_pwmL = 0;
	static int last_pwmR = 0;
	static uint32_t last_pwm_dbg_ms = 0;
	uint32_t now_ms = (uint32_t)tick_ms;
	if ((uint32_t)(now_ms - last_pwm_dbg_ms) >= 200U) {
		int dL = pwmL - last_pwmL;
		int dR = pwmR - last_pwmR;
		uint8_t changed = 0;
		if ((pwmL == 0 && last_pwmL != 0) || (pwmL != 0 && last_pwmL == 0)) changed = 1;
		if ((pwmR == 0 && last_pwmR != 0) || (pwmR != 0 && last_pwmR == 0)) changed = 1;
		if (dL > 800 || dL < -800 || dR > 800 || dR < -800) changed = 1;
		if (changed) {
			last_pwm_dbg_ms = now_ms;
			printf("[PWM] L=%d R=%d\r\n", pwmL, pwmR);
		}
	}
	last_pwmL = pwmL;
	last_pwmR = pwmR;

	 if(pwmL>0)
    {
        DL_GPIO_setPins(AIN_PORT,AIN_AIN2_PIN);
        DL_GPIO_clearPins(AIN_PORT,AIN_AIN1_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,ABS(pwmL),GPIO_PWM_0_C0_IDX);
    }
    else
    {
        DL_GPIO_setPins(AIN_PORT,AIN_AIN1_PIN);
        DL_GPIO_clearPins(AIN_PORT,AIN_AIN2_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,ABS(pwmL),GPIO_PWM_0_C0_IDX);
    }
    if(pwmR>0)
    {
		DL_GPIO_setPins(BIN_PORT,BIN_BIN2_PIN);
        DL_GPIO_clearPins(BIN_PORT,BIN_BIN1_PIN);
        DL_Timer_setCaptureCompareValue(PWM_0_INST,ABS(pwmR),GPIO_PWM_0_C1_IDX);
    }
    else
    {
		DL_GPIO_setPins(BIN_PORT,BIN_BIN1_PIN);
        DL_GPIO_clearPins(BIN_PORT,BIN_BIN2_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,ABS(pwmR),GPIO_PWM_0_C1_IDX);
    }
}



