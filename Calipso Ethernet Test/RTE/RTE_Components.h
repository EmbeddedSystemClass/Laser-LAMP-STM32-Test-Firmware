
/*
 * Auto generated Run-Time-Environment Component Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'CalipsoEthernetTest' 
 * Target:  'EthernetTest' 
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
#define RTE_Compiler_IO_STDERR          /* Compiler I/O: STDERR */
        #define RTE_Compiler_IO_STDERR_ITM      /* Compiler I/O: STDERR ITM */
#define RTE_Compiler_IO_STDIN           /* Compiler I/O: STDIN */
        #define RTE_Compiler_IO_STDIN_ITM       /* Compiler I/O: STDIN ITM */
#define RTE_Compiler_IO_STDOUT          /* Compiler I/O: STDOUT */
        #define RTE_Compiler_IO_STDOUT_ITM      /* Compiler I/O: STDOUT ITM */
#define RTE_Compiler_IO_TTY             /* Compiler I/O: TTY */
        #define RTE_Compiler_IO_TTY_BKPT        /* Compiler I/O: TTY Breakpoint */
#define RTE_DEVICE_FRAMEWORK_CLASSIC
#define RTE_DEVICE_HAL_COMMON
#define RTE_DEVICE_HAL_CORTEX
#define RTE_DEVICE_HAL_DMA
#define RTE_DEVICE_HAL_FLASH
#define RTE_DEVICE_HAL_GPIO
#define RTE_DEVICE_HAL_PWR
#define RTE_DEVICE_HAL_RCC
#define RTE_DEVICE_HAL_SPI
#define RTE_DEVICE_HAL_TIM
#define RTE_DEVICE_HAL_UART
#define RTE_DEVICE_HAL_USART
#define RTE_DEVICE_STARTUP_STM32F4XX    /* Device Startup for STM32F4 */
#define RTE_Drivers_ETH_MAC0            /* Driver ETH_MAC0 */
#define RTE_Drivers_MCI0                /* Driver MCI0 */
#define RTE_Drivers_PHY_LAN8720         /* Driver PHY LAN8720 */
#define RTE_Drivers_SPI1                /* Driver SPI1 */
        #define RTE_Drivers_SPI2                /* Driver SPI2 */
        #define RTE_Drivers_SPI3                /* Driver SPI3 */
        #define RTE_Drivers_SPI4                /* Driver SPI4 */
        #define RTE_Drivers_SPI5                /* Driver SPI5 */
        #define RTE_Drivers_SPI6                /* Driver SPI6 */
#define RTE_Drivers_USART1              /* Driver USART1 */
        #define RTE_Drivers_USART2              /* Driver USART2 */
        #define RTE_Drivers_USART3              /* Driver USART3 */
        #define RTE_Drivers_USART4              /* Driver USART4 */
        #define RTE_Drivers_USART5              /* Driver USART5 */
        #define RTE_Drivers_USART6              /* Driver USART6 */
        #define RTE_Drivers_USART7              /* Driver USART7 */
        #define RTE_Drivers_USART8              /* Driver USART8 */
#define RTE_FileSystem_Core             /* File System Core */
          #define RTE_FileSystem_LFN              /* File System with Long Filename support */
#define RTE_FileSystem_Drive_MC_0       /* File System Memory Card Drive 0 */

#endif /* RTE_COMPONENTS_H */
