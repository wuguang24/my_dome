/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "board.h"
#include "control.h"

#define FW_DEBUG_LOG 1  /* 启用详细调试信息 */
#define APP_SHOW_ENABLE 0

u8 Car_Mode=Diff_Car;
int Motor_Left,Motor_Right;                 //电机PWM变量
u8 PID_Send;            //延时和调参相关变量
float RC_Velocity=200,RC_Turn_Velocity,Move_X,Move_Y,Move_Z,PS2_ON_Flag;               //遥控控制的速度
float Velocity_Left,Velocity_Right; //车轮速度(mm/s)
u16 test_num,show_cnt;
float Voltage=0;
u32 Distance=0;
u32 Distance_UpdateMs=0;

int main(void)
{
    // 系统初始化
    SYSCFG_DL_init();  // 初始化系统配置
	DL_SYSTICK_enableInterrupt();
	/* UART0 默认被 SysConfig 生成代码打开了 loopback，这会影响外部串口调试 */
	DL_UART_Main_disableLoopbackMode(UART_0_INST);

    // 清除所有外设的中断挂起状态
    NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);    // 编码器A/MPU6050 GPIOA组中断
    NVIC_ClearPendingIRQ(ENCODERB_INT_IRQN);    // 编码器B中断
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);  // UART0串口中断
    NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);  // UART1串口中断
    // 使能各外设的中断
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);    // 开启编码器A/MPU6050 GPIOA组中断
    NVIC_EnableIRQ(ENCODERB_INT_IRQN);    // 开启编码器B中断
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN); // 开启UART0中断
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN); // 开启UART1中断
    // 定时器和ADC相关中断配置

    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);  // 清除定时器0中断挂起
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);        // 开启定时器0中断
    NVIC_EnableIRQ(ADC12_VOLTAGE_INST_INT_IRQN);
	/* Servo PWM timer (TIMG1) interrupt init */
	Servo_TimerInit();
	NVIC_ClearPendingIRQ(TIMA0_INT_IRQn);
	NVIC_EnableIRQ(TIMA0_INT_IRQn);
	/* 蓝牙 UART1 DMA 接收初始化（BTBufferHandler 依赖 DMA 计数） */
	BT_DAMConfig();
	
	printf("\r\n========================================\r\n");
	printf("[INIT] System boot started...\r\n");
	printf("[INIT] Build: %s %s\r\n", __DATE__, __TIME__);
	
	User_sIICDev.init();
	printf("[INIT] I2C initialized\r\n");
	
	MPU6050_initialize();
	printf("[INIT] MPU6050 initialized\r\n");
	
	DMP_Init();
	printf("[INIT] DMP initialized\r\n");
	
	OLED_Init();  // 初始化OLED显示屏
	printf("[INIT] OLED initialized\r\n");
	
	printf("[FW] WHEELTEC_C07A_IRF_CAR ready\r\n");
	printf("========================================\r\n");
	
	// 延迟初始化舵机和超声波，确保所有基础模块先完成初始化
	{
		uint32_t t0;
		Servo_SetAngleDeg(servo_angle_offset(90));
		Servo_Enable(1U);
		t0 = (uint32_t)tick_ms;
		while ((uint32_t)(tick_ms - t0) < 300U) {
			BTBufferHandler();
		}
		/* Keep servo PWM enabled so it can hold center position */
		Servo_SetAngleDeg(servo_angle_offset(90));
	}
	
	// 初始化超声波相关变量
	Distance = 9999U;  // 初始设为安全远距离
	Distance_UpdateMs = 0U;
	
    // 主循环
    uint32_t last_hcsr04_ms = 0;
	uint32_t last_dbg_ms = 0;
	uint32_t last_dist_ms = 0;
	uint32_t last_mpu_ms = 0;
	uint32_t last_system_ms = 0;
	uint32_t last_oled_ms = 0;
	float hcsr04_last_raw = 0.0f;
	uint32_t hcsr04_ok_cnt = 0;
	uint32_t hcsr04_fail_cnt = 0;
	
	printf("[MAIN] Entering main loop...\r\n");
	
    while (1) 
    {
        BTBufferHandler();    // 处理蓝牙数据缓冲区
        Voltage = Get_battery_volt();//采样小车当前电压

        /* 超声波测量 - 100ms间隔 */
        if ((uint32_t)(tick_ms - last_hcsr04_ms) >= 100U) {
            last_hcsr04_ms = (uint32_t)tick_ms;

            float d = HCSR04_ReadDistanceCm();
            hcsr04_last_raw = d;

            if (d > 0 && d < 400.0f) {
                Distance = (u32)(d * 10.0f);
                Distance_UpdateMs = (u32)tick_ms;
                hcsr04_ok_cnt++;
            } else {
                hcsr04_fail_cnt++;
                if (Distance_UpdateMs == 0U) {
                    Distance = 9999U;
                    Distance_UpdateMs = (u32)tick_ms;
                }
            }

            /* BT模式近距障碍物检测：距离 < 10cm 或测距失败但上次较近 */
            if (Run_Mode == 0) {
                uint8_t near = 0;
                if (Distance != 0 && Distance < 100U) {  /* 10cm */
                    near = 1;
                }
                /* 测距失败(盲区/已碰墙)且上一次较近时，保守视为近距 */
                if (d <= 0.0f && Distance != 0 && Distance < 150U && Distance_UpdateMs != 0) {
                    near = 1;
                }
                BT_Obstacle_Near = near;
            } else {
                BT_Obstacle_Near = 0;
            }
        }
        
        /* 距离调试输出 - 500ms间隔 */
        if ((uint32_t)(tick_ms - last_dist_ms) >= 500U) {
            last_dist_ms = (uint32_t)tick_ms;
            uint32_t age_ms = (Distance_UpdateMs != 0) ? (tick_ms - Distance_UpdateMs) : 0xFFFFFFFF;
            printf("[DIST] raw=%.1fcm, Dist=%lu(cm*10), age=%lums, OK=%lu, FAIL=%lu\r\n",
                   (double)hcsr04_last_raw,
                   (unsigned long)Distance,
                   (unsigned long)age_ms,
                   (unsigned long)hcsr04_ok_cnt,
                   (unsigned long)hcsr04_fail_cnt);
        }

        /* MPU6050调试输出 - 500ms间隔 */
        if ((uint32_t)(tick_ms - last_mpu_ms) >= 500U) {
            last_mpu_ms = (uint32_t)tick_ms;
            printf("[MPU] pitch=%.2f roll=%.2f yaw=%.2f\r\n",
                   mpu6050.pitch,
                   mpu6050.roll,
                   mpu6050.yaw);
        }
        
        /* 系统状态调试输出 - 1000ms间隔 */
        if ((uint32_t)(tick_ms - last_system_ms) >= 1000U) {
            last_system_ms = (uint32_t)tick_ms;
            printf("[SYS] t=%lums, stop=%u, mode=%d, V=%.2fV, Flag_Dir=%d\r\n",
                   (unsigned long)tick_ms,
                   (unsigned int)Flag_Stop,
                   Run_Mode,
                   Voltage,
                   Flag_Direction);
            printf("[MOTOR] Move_X=%.3f, Move_Z=%.3f\r\n",
                   Move_X, Move_Z);
            printf("[ENC] Tgt(L/R)=%.3f/%.3f, Cur(L/R)=%.3f/%.3f\r\n",
                   MotorA.Target_Encoder,
                   MotorB.Target_Encoder,
                   MotorA.Current_Encoder,
                   MotorB.Current_Encoder);
            printf("[PWM] PWM(L/R)=%.0f/%.0f\r\n",
                   MotorA.Motor_Pwm,
                   MotorB.Motor_Pwm);
        }
        
        // OLED显示更新 - 50ms间隔
		if ((uint32_t)((uint32_t)tick_ms - last_oled_ms) >= 50U) {
			last_oled_ms = (uint32_t)tick_ms;
			oled_show();
		}

        /* 模式切换处理 */
        {
            static uint8_t last_run_mode = 0xFF;
            if (Run_Mode != last_run_mode) {
                printf("[MODE] Changed from %d to %d (0=BT, 1=Line)\r\n",
                       last_run_mode, Run_Mode);
                Servo_SetAngleDeg(servo_angle_offset(90));
                if (Run_Mode == 0 || Run_Mode == 1) {
                    Flag_Stop = 0;
                    printf("[MODE] Auto start (Flag_Stop=0)\r\n");
                }
                last_run_mode = Run_Mode;
            }
        }

        /* 保持舵机PWM使能 */
        Servo_Enable(1U);
        Servo_Task20ms();
    }
}
