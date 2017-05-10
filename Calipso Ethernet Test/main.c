/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.4
  * @date    06-May-2016
  * @brief   Main program body
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
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
#include "main.h"
#include <stdio.h>
#include "Driver_USART.h"
#include "DGUS.h"
#include "DS18B20.h"
#include "LaserMisc.h"
#include "GlobalVariables.h"
#include "SDCard.h"
#include "WiFiThread.h"
#include "CANBus.h"

#include <math.h>
#include "arm_math.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif
#ifdef RTE_CMSIS_RTOS                   // when RTE component CMSIS RTOS is used
#include "cmsis_os.h"                   // CMSIS RTOS header file
#endif

#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern osThreadId tid_WiFiThread;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
extern void resolve_host (void);
extern void SetDACValue(float32_t value);
extern int  Init_MainSPI_Thread (void);
extern int  Init_WiFi_Thread (void);
extern int  Init_Main_Thread (void);
extern void Init_Timers (void);

uint8_t rx_buffer[8];

float32_t absf(float32_t x)
{
	if (x < 0.0f) return -x;
	return x;
}

bool eneable_temp_sensor = true;

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

#ifdef RTE_CMSIS_RTOS                   // when using CMSIS RTOS
  osKernelInitialize();                 // initialize CMSIS-RTOS
#endif
	
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
	HAL_Init();
	
	HAL_Delay(10);
	
	if (GetLaserID() == LASER_ID_DIODELASER)
		__MISC_RELAY3_ON();
	else
		__MISC_RELAY3_OFF();
	
	if (GetLaserID() == LASER_ID_SOLIDSTATE)
		__MISC_RELAY2_ON();
	else
		__MISC_RELAY2_OFF();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
	SystemCoreClockUpdate();
	
#ifndef USE_DGUS_DRIVER
	Initialize_DGUS();
#endif

	SpeakerInit();	
	init_filesystem();
	
	// Load global variables from flash
	//ClearGlobalVariables();
	LoadGlobalVariables();
	
	start_log(datetime);
	
	//HAL_Init();
	//SystemClock_Config();
	//SystemCoreClockUpdate();

  /* Add your application code here
     */

#ifdef RTE_CMSIS_RTOS                   // when using CMSIS RTOS
	osKernelStart();                      // start thread execution 
	
	Init_MainSPI_Thread();
	Init_WiFi_Thread();
	Init_DS18B20();
	Init_Timers();
	
	if (DS18B20_Reset())
		DS18B20_StartConvertion();
		
	HAL_Delay(750); // delay 750 ms
		
	if (DS18B20_Reset())
	{
		uint16_t tt = DS18B20_ReadData();
		float32_t t = tt * 0.0625f;
		temperature = t;
	}
	
	SoundOn();
	HAL_Delay(500);
	SoundOff();
	//HAL_Delay(3000); 										// Wait for display initialization
	
	Init_Main_Thread();
	
	// Start logging
	HAL_Delay(100);
#endif

  /* Infinite loop */
  while (1)
  {
		if (eneable_temp_sensor)
		{
			if (DS18B20_Reset())
				DS18B20_StartConvertion();
		
			HAL_Delay(750); // delay 750 ms
		
			if (DS18B20_Reset())
			{
				uint16_t tt = DS18B20_ReadData();
				float32_t t = tt * 0.0625f;
				if ((absf(t - temperature) < 10) && (t < 100.0f) && (t > 0.0f))
					temperature = t;
			}
		}
		
		/*SoundOn();
		HAL_Delay(500);
		SoundOff();*/
		
		LOG_F(LOG_ID_TEMPERATURE, "Temperature:%.2f\r\n", temperature);
		
		if (temperature > temperature_cool_on)
		{
			LOG_I(LOG_ID_COOLINGFAN, "Cooling on\r\n", 1);
			__MISC_RELAY1_ON();
			g_cooling_en = true;
		}
		
		if (temperature < temperature_cool_off)
		{
			LOG_I(LOG_ID_COOLINGFAN, "Cooling off\r\n", 0);
			__MISC_RELAY1_OFF();
			g_cooling_en = false;
		}
		
		//osSignalSet(tid_WiFiThread, WIFI_EVENT_TEMPERATURE_UPDATE);
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 168
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4; //RCC_HCLK_DIV16; //RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
