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
#include "SDCardBase.h"
#include "stm32f4xx_hal_flash.h"

#include "stdlib.h"
#include "ctype.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#include "Driver_MCI.h"
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

uint32_t flashSectorSizeTable[12] = {0x4000, 0x4000, 0x4000, 0x4000, 0x10000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000};
uint32_t flashSectorAddrTable[13] = {0x08000000, 0x8004000, 0x08008000, 0x0800c000, 0x08010000, 0x08020000, 0x08040000, 0x08060000, 0x08080000, 0x080A0000, 0x080C0000, 0x080E0000, 0x08100000};
bool     flashSectorIsCleared[12] = {0};
extern osThreadId tid_MainThread;
extern ARM_DRIVER_MCI Driver_MCI0;

char FirmwareFileName[256];
volatile char FirmwareVersion[16] = "V1.0";
volatile uint16_t firmware_update_progress = 0;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define ApplicationAddress 0x08010000

/* Private variables ---------------------------------------------------------*/
// Local variables
uint16_t currentSector = 0;
uint32_t baseAddr = 0x0000;
uint32_t offsAddr = 0x0000;
char buffer[256];
char packet[16];
bool end_of_file = false;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
extern int  Init_Main_Thread (void);

uint32_t hextoint(char* str, int num)
{
	uint32_t result = 0;
	uint32_t p = 1;
	int i = 0;
	for (i = 0; i < num; i++)
	{
		int j = num - 1 - i;
		if (str[j] >= '0' && str[j] <= '9')
			result += (str[j] - '0') * p;
		if (str[j] >= 'A' && str[j] <= 'F')
			result += (10 + str[j] - 'A') * p;
		if (str[j] >= 'a' && str[j] <= 'f')
			result += (10 + str[j] - 'A') * p;
		p *= 16;
	}	
	return result;
}

uint32_t EraseSector(uint32_t sector)
{
	FLASH_EraseInitTypeDef flash_erase = {0};
	
	flash_erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	flash_erase.Banks = FLASH_BANK_1;
	flash_erase.Sector = sector;
	flash_erase.NbSectors = 1;
	flash_erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	
	uint32_t sector_error = 0;
	
	while (HAL_FLASH_Unlock() != HAL_OK);
	HAL_FLASHEx_Erase(&flash_erase, &sector_error);
	
	FLASH_WaitForLastOperation((uint32_t)50000U);
	HAL_FLASH_Lock();
	
	return sector_error;
}

int GetSectorByAddr(uint32_t addr)
{
	int i = 0;
	for (i = 0; i < 12; i++)
		if (addr >= flashSectorAddrTable[i] && addr < flashSectorAddrTable[i+1])
			return i;
	return -1;
}

void JumpToApp()
{
	typedef  void (*pFunction)(void);
	pFunction Jump_To_Application;
	uint32_t JumpAddress;
	
	// Deinitialize periph
	HAL_DeInit();
	Driver_MCI0.Uninitialize();
	osThreadTerminate(tid_MainThread); // Terminate main thread
	
	// Set interrupt vector table offset
	__disable_irq();
	SCB->VTOR = ApplicationAddress;
	__enable_irq();
	
	// Jump to main application
	if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
	{ 
		JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		__set_CONTROL(0); // Change PSP to MSP
		__set_MSP(*(__IO uint32_t*) ApplicationAddress);
		Jump_To_Application();
	}
}

#define PACK_SIZE 64

void UpdateFirmwareFromBin()
{
	FILE* fp = fopen(FirmwareFileName, "r");
		
	if (fp != 0) {
	
		fseek(fp, 0, SEEK_END);
		uint32_t firmware_len = ftell(fp);
		uint32_t current_size = 0;
		uint32_t needtoread = firmware_len;
		uint32_t currentAddr = ApplicationAddress;
		fclose(fp);
		
		fp = fopen(FirmwareFileName, "r");
		
		slog_out(datetime, "Firmware update starting\r\n");
		printf("Firmware update starting\r\n");
		
		end_of_file = false;
		
		while (!end_of_file)
		{
			uint16_t sector = GetSectorByAddr(currentAddr);
			
			// Clear sector if needed
			if (!flashSectorIsCleared[sector])
			{
				EraseSector(sector);
				flashSectorIsCleared[sector] = true;
			}
			
			if (needtoread > PACK_SIZE)
			{
				fread(buffer, 1, PACK_SIZE, fp);
				
				while (HAL_FLASH_Unlock() != HAL_OK);
				fmemcpy((void*)currentAddr, (void*)buffer, PACK_SIZE);
				HAL_FLASH_Lock();
				
				currentAddr += PACK_SIZE;
				needtoread -= PACK_SIZE;
				current_size += PACK_SIZE;
			}
			else
			{
				fread(buffer, 1, needtoread, fp);
				
				while (HAL_FLASH_Unlock() != HAL_OK);
				fmemcpy((void*)currentAddr, (void*)buffer, needtoread);
				HAL_FLASH_Lock();
				
				currentAddr += needtoread;
				current_size += needtoread;
				end_of_file = true;
			}
			
			firmware_update_progress = (current_size * 100) / firmware_len;
		}
		
		fclose(fp);
		
		HAL_Delay(100);
			
		slog_out(datetime, "Firmware update complete\r\n");
		printf("Firmware update complete\r\n");
	}
}

void UpdateFirmwareFromHex()
{
	FILE* fp = fopen(FirmwareFileName, "r");
		
	if (fp != 0) {
	
		fseek(fp, 0, SEEK_END);
		uint32_t firmware_len = ftell(fp);
		uint32_t current_size = 0;
		fclose(fp);
		
		fp = fopen(FirmwareFileName, "r");
		
		slog_out(datetime, "Firmware update starting\r\n");
		printf("Firmware update starting\r\n");
		
		// Read from hex file
		while (!end_of_file)
		{
			uint32_t frame_size = 0;
			
			while (frame_size < flashSectorSizeTable[currentSector])
			{
				char* str = fgets(buffer, 256, fp);
				
				/*
				slog_out(datetime, str);
				printf(str);*/
				
				uint16_t sector = 0;
				uint32_t addr = 0;
				uint16_t tt = hextoint(&str[6+1], 2);
				uint16_t size = 0;
				uint16_t i = 0;
				
				switch (tt)
				{
					case 0x04:
						baseAddr	= hextoint(&str[8+1], 4) << 16;
						break;
					case 0x00:
						size			= hextoint(&str[0+1], 2);
						offsAddr	= hextoint(&str[2+1], 4);
						addr			= baseAddr | offsAddr;
						sector 		= GetSectorByAddr(addr);
						currentSector = sector;
					
						// Firmware address alias boot sections
						if (addr < 0x08010000)
						{
							fclose(fp);
							slog_out(datetime, "Firmware offset error\r\n");
							printf("Firmware offset error\r\n");
							JumpToApp();
						}
					
						// Clear sector if needed
						if (!flashSectorIsCleared[sector])
						{
							EraseSector(sector);
							flashSectorIsCleared[sector] = true;
						}
					
						// read data from string
						for (i = 0; i < size; i++)
							packet[i] = hextoint(&str[8+1+i*2], 2);
						
						while (HAL_FLASH_Unlock() != HAL_OK);
						fmemcpy((void*)addr, (void*)packet, size);
						HAL_FLASH_Lock();
						
						frame_size += size;
						current_size += strlen(str);
						firmware_update_progress = (current_size * 100) / firmware_len;
						break;
					case 0x05:
						break;
					case 0x01:
						end_of_file = true;
						break;
				};
				
				if (end_of_file) break;
			}
			currentSector++;
		}
		fclose(fp);
		
		HAL_Delay(100);
			
		slog_out(datetime, "Firmware update complete\r\n");
		printf("Firmware update complete\r\n");
	}
}

void UpdateFirmware()
{
	char filename[256];
	int i = 0;
	
	for (i = 0; i < strlen(FirmwareFileName); i++)
		filename[i] = toupper(FirmwareFileName[i]);
	filename[i] = 0;
	
	//memcpy((void*)filename, (void*)FirmwareFileName, strlen(FirmwareFileName));
	char* name = strtok(filename, ".");
	char* extn = strtok(NULL, ".");
	
	if (sdcard_ready)
	{
		if (strcmp(extn, "HEX") == 0)
			UpdateFirmwareFromHex();
		
		if (strcmp(extn, "BIN") == 0)
			UpdateFirmwareFromBin();
		
		HAL_Delay(1000);
		
		JumpToApp();
	}
	else
		JumpToApp();
}

/* Private functions ---------------------------------------------------------*/
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

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
	SystemCoreClockUpdate();
	
	/* Init File System of SD */
	init_filesystem();
	
#ifndef USE_DGUS_DRIVER
	Initialize_DGUS();
#endif

  /* Add your application code here
     */
	uint32_t timer_start = HAL_GetTick();
	
	HAL_Delay(1000);

#ifdef RTE_CMSIS_RTOS                   // when using CMSIS RTOS
  osKernelStart();                      // start thread execution 
	Init_Main_Thread();
#endif

	// Check for firmware update
	if (sdcard_ready)
	{
		FILE* fp = fopen("FirmwareUpdate.txt", "r");
		if (fp != 0)
		{
			fgets((char*)FirmwareVersion, 16, fp);
			fgets((char*)FirmwareFileName, 256, fp);
			fclose(fp);
		}
		else {
			slog_out(datetime, "Firmware update is not available\r\n");
			printf("Firmware update is not available\r\n");
			JumpToApp();
		}
	}

	// Check for SD card enabled
	if (!sdcard_ready)
	{
		slog_out(datetime, "Firmware update is missing, start application\r\n");
		printf("Firmware update is missing, start application\r\n");
		JumpToApp();
	}
	
  /* Infinite loop */
  while (1)
  {
		if ((HAL_GetTick() - timer_start) > 3000)
			JumpToApp();
		
		if (pic_id == FRAME_PICID_BOOTFIRMWAREUPDATE)
		{
			UpdateFirmware();
			
			// Could not start main application
			while (1);
		}
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
  *            PLL_N                          = 336
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
