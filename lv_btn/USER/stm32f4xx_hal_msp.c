/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.2.5
  * @date    17-February-2017
  * @brief   HAL MSP module.
  *         
  @verbatim
 ===============================================================================
                     ##### How to use this driver #####
 ===============================================================================
    [..]
    This file is generated automatically by STM32CubeMX and eventually modified 
    by the user

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "usart3.h"
#include "uart4.h"
#include "esp32_uart.h"
#include <string.h>  // for memmove

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

/**
  * @brief  Initializes the Global MSP.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
  /* NOTE : This function is generated automatically by STM32CubeMX and eventually  
            modified by the user
   */ 
}

/**
  * @brief  DeInitializes the Global MSP.
  * @param  None  
  * @retval None
  */
void HAL_MspDeInit(void)
{
  /* NOTE : This function is generated automatically by STM32CubeMX and eventually  
            modified by the user
   */
}

/**
  * @brief  UART MSP Initialization
  * @param  huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  if(huart->Instance==USART1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#if EN_USART1_RX
    /* USART1 interrupt Init - 优先级必须>=5才能调用FreeRTOS API */
    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
#endif
  }
  else if(huart->Instance==USART3)
  {
    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#if EN_USART3_RX
    /* USART3 interrupt Init - 优先级必须>=5才能调用FreeRTOS API */
    HAL_NVIC_SetPriority(USART3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
#endif
  }
  else if(huart->Instance==UART4)
  {
    /* Peripheral clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /**UART4 GPIO Configuration
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

#if EN_UART4_RX
    /* UART4 interrupt Init - 优先级必须>=5才能调用FreeRTOS API */
    HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
#endif
  }
  else if(huart->Instance==USART6)
  {
    /* Peripheral clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

#if EN_ESP32_UART_RX
    /* USART6 interrupt Init - 优先级必须>=5才能调用FreeRTOS API */
    HAL_NVIC_SetPriority(USART6_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
#endif
  }
}

/**
  * @brief  UART Rx Complete Callback
  * @param  huart: UART handle pointer
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if EN_USART1_RX
  extern u8 aRxBuffer[RXBUFFERSIZE];
  extern u16 USART_RX_STA;
  extern u8 USART_RX_BUF[USART_REC_LEN];
  
  if(huart->Instance==USART1)
  {
    if((USART_RX_STA&0x8000)==0)                //接收未完成
    {
      if(USART_RX_STA&0x4000)                   //接收到了0x0d
      {
        if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;   //接收错误,重新开始
        else USART_RX_STA|=0x8000;              //接收完成了 
      }
      else //还没收到0X0D
      {	
        if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
        else
        {
          USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0];
          USART_RX_STA++;
          if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收
        }		 
      }
    }
  }
#endif

#if EN_USART3_RX
  extern u8 aRxBuffer3[USART3_RXBUFFERSIZE];
  extern u16 USART3_RX_STA;
  extern u8 USART3_RX_BUF[USART3_REC_LEN];
  
  if(huart->Instance==USART3)
  {
    // 雷达数据直接放入缓冲区，不进行0x0d 0x0a判断
    if (USART3_RX_STA < (USART3_REC_LEN - 1))
    {
      USART3_RX_BUF[USART3_RX_STA & 0X3FFF] = aRxBuffer3[0];
      USART3_RX_STA++;
    }
    else
    {
      USART3_RX_STA = 0; // 接收数据错误,重新开始接收
    }
  }
#endif

#if EN_UART4_RX
  extern u8 aRxBuffer4[UART4_RXBUFFERSIZE];
  extern u16 UART4_RX_STA;
  extern u8 UART4_RX_BUF[UART4_REC_LEN];
  
  if(huart->Instance==UART4)
  {
    // GPS数据直接放入缓冲区，不进行0x0d 0x0a判断
    if (UART4_RX_STA < (UART4_REC_LEN - 1))
    {
      UART4_RX_BUF[UART4_RX_STA & 0X3FFF] = aRxBuffer4[0];
      UART4_RX_STA++;
    }
    else
    {
      UART4_RX_STA = 0; // 接收数据错误,重新开始接收
    }
  }
#endif

#if EN_ESP32_UART_RX
  extern u8 aRxBufferESP32[ESP32_UART_RXBUFFERSIZE];
  extern u16 ESP32_UART_RX_STA;
  extern u8 ESP32_UART_RX_BUF[ESP32_UART_REC_LEN];
  
  if(huart->Instance==USART6)
  {
    // ESP32数据直接放入缓冲区，不进行0x0d 0x0a判断
    // 使用掩码确保不会越界访问（ESP32_UART_REC_LEN=512，掩码0x1FF）
    if (ESP32_UART_RX_STA < (ESP32_UART_REC_LEN - 1))
    {
      ESP32_UART_RX_BUF[ESP32_UART_RX_STA] = aRxBufferESP32[0];
      ESP32_UART_RX_STA++;
    }
    else
    {
      // 缓冲区满，丢弃最旧的数据，保留最新数据（环形缓冲区思想）
      // 简单处理：重置到一半位置，保留后半部分数据
      u16 half = ESP32_UART_REC_LEN / 2;
      memmove(ESP32_UART_RX_BUF, &ESP32_UART_RX_BUF[half], half);
      ESP32_UART_RX_STA = half;
      ESP32_UART_RX_BUF[ESP32_UART_RX_STA++] = aRxBufferESP32[0];
    }
  }
#endif
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
