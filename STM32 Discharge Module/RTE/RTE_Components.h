
/*
 * Auto generated Run-Time-Environment Component Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'STM32DischargeModule' 
 * Target:  'Target 1' 
 */

#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H


/*
 * Define the Device Header File: 
 */
#define CMSIS_device_header "stm32f4xx.h"

#define RTE_CMSIS_RTOS                  /* CMSIS-RTOS */
        #define RTE_CMSIS_RTOS_RTX              /* CMSIS-RTOS Keil RTX */
#define RTE_Compiler_EventMessaging
        #define RTE_Compiler_EventMessaging_DAP
#define RTE_Compiler_IO_File            /* Compiler I/O: File */
        #define RTE_Compiler_IO_File_FS         /* Compiler I/O: File (File System) */
#define RTE_DEVICE_FRAMEWORK_CLASSIC
#define RTE_DEVICE_HAL_ADC
#define RTE_DEVICE_HAL_COMMON
#define RTE_DEVICE_HAL_CORTEX
#define RTE_DEVICE_HAL_CRC
#define RTE_DEVICE_HAL_DAC
#define RTE_DEVICE_HAL_DMA
#define RTE_DEVICE_HAL_GPIO
#define RTE_DEVICE_HAL_IWDG
#define RTE_DEVICE_HAL_PCCARD
#define RTE_DEVICE_HAL_PWR
#define RTE_DEVICE_HAL_RCC
#define RTE_DEVICE_HAL_RNG
#define RTE_DEVICE_HAL_RTC
#define RTE_DEVICE_HAL_SPI
#define RTE_DEVICE_HAL_TIM
#define RTE_DEVICE_HAL_USART
#define RTE_DEVICE_STARTUP_STM32F4XX    /* Device Startup for STM32F4 */
#define RTE_Drivers_ETH_MAC0            /* Driver ETH_MAC0 */
#define RTE_Drivers_MCI0                /* Driver MCI0 */
#define RTE_Drivers_PHY_LAN8720         /* Driver PHY LAN8720 */
#define RTE_Drivers_USBD0               /* Driver USBD0 */
#define RTE_FileSystem_Core             /* File System Core */
          #define RTE_FileSystem_SFN              /* File System without Long Filename support */
#define RTE_FileSystem_Drive_MC_0       /* File System Memory Card Drive 0 */
#define RTE_Network_Core                /* Network Core */
          #define RTE_Network_IPv4                /* Network IPv4 Stack */
          #define RTE_Network_IPv6                /* Network IPv6 Stack */
          #define RTE_Network_Release             /* Network Release Version */
#define RTE_Network_Interface_ETH_0     /* Network Interface ETH 0 */
#define RTE_Network_Socket_TCP          /* Network Socket TCP */
#define RTE_Network_Web_Server_FS       /* Network Web Server with Web Resources on File System */
#define RTE_USB_Core                    /* USB Core */
#define RTE_USB_Device_0                /* USB Device 0 */
#define RTE_USB_Device_CDC_0            /* USB Device CDC instance 0 */

#endif /* RTE_COMPONENTS_H */
