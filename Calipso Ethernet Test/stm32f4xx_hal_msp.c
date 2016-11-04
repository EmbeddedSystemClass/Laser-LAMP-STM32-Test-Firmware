/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.2.4
  * @date    06-May-2016
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
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
#include "stm32f4xx_hal_gpio.h"
#include "SolidStateLaser.h"
#include "LaserMisc.h"

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
	
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	__GPIOF_CLK_ENABLE();
	__GPIOG_CLK_ENABLE();
	__USART6_CLK_ENABLE();
	
	//  ************************* FLOW   ******************************
	
	GPIO_InitTypeDef flow1 = {0};
	
	flow1.Pin   = GPIO_PIN_0;
	flow1.Mode  = GPIO_MODE_AF_PP;
	flow1.Pull  = GPIO_NOPULL;
	flow1.Speed = GPIO_SPEED_FREQ_LOW;
	flow1.Alternate = GPIO_AF3_TIM8;
	
	HAL_GPIO_Init(GPIOA, &flow1);
	
	GPIO_InitTypeDef flow2 = {0};
	
	flow2.Pin   = GPIO_PIN_0;
	flow2.Mode  = GPIO_MODE_AF_PP;
	flow2.Pull  = GPIO_NOPULL;
	flow2.Speed = GPIO_SPEED_FREQ_LOW;
	flow2.Alternate = GPIO_AF2_TIM4;
	
	HAL_GPIO_Init(GPIOE, &flow2);
	
	//  ************************* UART 6 ******************************
	GPIO_InitTypeDef uart = {0};
	
	uart.Pin   = GPIO_PIN_6;
	uart.Mode  = GPIO_MODE_AF_PP;
	uart.Pull  = GPIO_NOPULL;
	uart.Speed = GPIO_SPEED_FREQ_LOW;
	uart.Alternate = GPIO_AF8_USART6;
	
	HAL_GPIO_Init(GPIOC, &uart);
	
	uart.Pin   = GPIO_PIN_9;
	uart.Mode  = GPIO_MODE_AF_PP;
	uart.Pull  = GPIO_PULLUP;
	uart.Speed = GPIO_SPEED_FREQ_LOW;
	uart.Alternate = GPIO_AF8_USART6;
	
	HAL_GPIO_Init(GPIOG, &uart);
	
	//  ************************* UART 1 ******************************
	GPIO_InitTypeDef uart1 = {0};
	
	uart1.Pin   = GPIO_PIN_10;
	uart1.Mode  = GPIO_MODE_AF_PP;
	uart1.Pull  = GPIO_NOPULL;
	uart1.Speed = GPIO_SPEED_FREQ_LOW;
	uart1.Alternate = GPIO_AF7_USART1;
	
	HAL_GPIO_Init(GPIOA, &uart1);
	
	uart1.Pin   = GPIO_PIN_9;
	uart1.Mode  = GPIO_MODE_AF_PP;
	uart1.Pull  = GPIO_PULLUP;
	uart1.Speed = GPIO_SPEED_FREQ_LOW;
	uart1.Alternate = GPIO_AF7_USART1;
	
	HAL_GPIO_Init(GPIOA, &uart1);
	
	//  ************************* GPIO, SPI ********************************
	GPIO_InitTypeDef relays_gpio = {0};
	relays_gpio.Pin   = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	relays_gpio.Mode  = GPIO_MODE_OUTPUT_PP;
	relays_gpio.Pull  = GPIO_NOPULL;
	relays_gpio.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOB, &relays_gpio);
	
	GPIO_InitTypeDef gpio = {0};
	gpio.Pin   = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_VoltageMonitor_nCS | GPIO_PIN_CurrentMonitor_nCS;
	gpio.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio.Pull  = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOG, &gpio);
	
	GPIO_InitTypeDef gpio_E = {0};
	gpio_E.Pin   = GPIO_PIN_7 | GPIO_PIN_LaserDiode_Pulse;
	gpio_E.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio_E.Pull  = GPIO_NOPULL;
	gpio_E.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOE, &gpio_E);
	
	gpio_E.Pin   = GPIO_PIN_Laser_ID0 | GPIO_PIN_Laser_ID1;
	gpio_E.Mode  = GPIO_MODE_INPUT;
	gpio_E.Pull  = GPIO_NOPULL;
	gpio_E.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOE, &gpio_E);
	
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_CurrentMonitor_nCS | GPIO_PIN_CurrentMonitor_nCS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_CurrentProgram_nCS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_LaserDiode_Pulse, GPIO_PIN_SET);
	
	//  ************************* Lamp control ********************************
	
	GPIO_InitTypeDef gpio_F = {0};
	gpio_F.Pin   = GPIO_PIN_SimmerSensor | GPIO_PIN_ChargeModuleOn | GPIO_PIN_ChargeModuleFault | GPIO_PIN_ChargeModuleOvervoltage;
	gpio_F.Mode  = GPIO_MODE_INPUT;
	gpio_F.Pull  = GPIO_NOPULL;
	gpio_F.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOF, &gpio_F);
	
	GPIO_InitTypeDef gpio_C = {0};
	gpio_C.Pin   = GPIO_PIN_ChargeModuleOverheating | GPIO_PIN_ChargeModuleReady;
	gpio_C.Mode  = GPIO_MODE_INPUT;
	gpio_C.Pull  = GPIO_NOPULL;
	gpio_C.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOC, &gpio_C);
	
	//  ************************* Diode control ******************************
	gpio_F.Pin   = GPIO_PIN_LaserDiodeEnable;
	gpio_F.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio_F.Pull  = GPIO_NOPULL;
	gpio_F.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOF, &gpio_F);
	
	GPIO_InitTypeDef gpio_A = {0};
	gpio_A.Pin   = GPIO_PIN_LaserLED;
	gpio_A.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio_A.Pull  = GPIO_NOPULL;
	gpio_A.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOA, &gpio_A);
	
	//  ************************* Solid state laser LED **********************
	GPIO_InitTypeDef gpio_B = {0};
	gpio_B.Pin   = GPIO_PIN_LaserLED2;
	gpio_B.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio_B.Pull  = GPIO_NOPULL;
	gpio_B.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOB, &gpio_B);
	
	LampControlInit();
	FlowInit();
	CoolInit();
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
	
	HAL_GPIO_DeInit(GPIOG, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOG, GPIO_PIN_CurrentMonitor_nCS | GPIO_PIN_CurrentMonitor_nCS);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_CurrentProgram_nCS | GPIO_PIN_Laser_ID0 | GPIO_PIN_Laser_ID0);
	
	__GPIOA_CLK_DISABLE();
	__GPIOB_CLK_DISABLE();
	__GPIOC_CLK_DISABLE();
	__GPIOE_CLK_DISABLE();
	__GPIOF_CLK_DISABLE();
	__GPIOG_CLK_DISABLE();
	__USART6_CLK_DISABLE();
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
