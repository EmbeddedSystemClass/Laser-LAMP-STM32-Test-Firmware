
#include "Driver_SPI.h"
#include "cmsis_os.h"                                           // CMSIS RTOS header file

#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#include <math.h>
#include "arm_math.h"

#include "GlobalVariables.h"
#include "CANBus.h"

#define GPIO_PIN_VoltageMonitor_nCS	GPIO_PIN_0 // PORT GPIOG
#define GPIO_PIN_CurrentMonitor_nCS GPIO_PIN_1 // PORT GPIOG
#define GPIO_PIN_CurrentProgram_nCS GPIO_PIN_7 // PORT GPIOE

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
void MainSPI_Thread (void const *argument);                     // thread function
osThreadId tid_MainSPI_Thread;                                  // thread id
osThreadDef (MainSPI_Thread, osPriorityNormal, 1, 0);           // thread object

/* SPI Driver */
extern ARM_DRIVER_SPI Driver_SPI2;

volatile bool wait = true;
volatile bool dac_cs = false;
volatile bool adc1_cs = false; 
volatile bool adc2_cs = false; 
 
void MainSPI_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        /* Success: Wakeup Thread */
				if ( dac_cs) HAL_GPIO_WritePin(GPIOE, GPIO_PIN_CurrentProgram_nCS, GPIO_PIN_RESET);
				if (adc1_cs) HAL_GPIO_WritePin(GPIOG, GPIO_PIN_CurrentMonitor_nCS, GPIO_PIN_RESET);
				if (adc2_cs) HAL_GPIO_WritePin(GPIOG, GPIO_PIN_VoltageMonitor_nCS, GPIO_PIN_RESET);
				wait = false;
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}

int Init_MainSPI_Thread (void) {

	#ifdef DEBUG
  ARM_DRIVER_VERSION   version;
  ARM_SPI_CAPABILITIES drv_capabilities;
 
  version = SPIdrv->GetVersion();
  if (version.api < 0x200) /* requires at minimum API version 2.00 or higher */
  {                        /* error handling                                 */
      return(-1);
  }
 
  drv_capabilities = SPIdrv->GetCapabilities();
  if (drv_capabilities.event_mode_fault == 0)
  {                        /* error handling */
      return(-1);
  }
#endif
 
  /* Initialize the SPI driver */
  Driver_SPI2.Initialize(MainSPI_callback);
  /* Power up the SPI peripheral */
  Driver_SPI2.PowerControl(ARM_POWER_FULL);
  /* Configure the SPI to Master, 8-bit mode @10000 kBits/sec */
  Driver_SPI2.Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED | ARM_SPI_DATA_BITS(16), 1000000);
	
  tid_MainSPI_Thread = osThreadCreate (osThread(MainSPI_Thread), NULL);
  if (!tid_MainSPI_Thread) return(-1);
  
  return(0);
}

void SetDACValue(float32_t value)
{
	uint16_t data = (uint16_t)((value / 10.0f) * 2048.0f); // calibration
	
	// Set DAC value
	dac_cs = true;
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_CurrentProgram_nCS, GPIO_PIN_SET);
	
	Driver_SPI2.Send((void*)&data, 1);
	
	while (wait);
	wait = true;
	dac_cs = false;
}

void MainSPI_Thread (void const *argument) {

	ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;
	
	volatile uint16_t datain;
	uint8_t len = 0;
		
  while (1) {
		
		uint16_t datainv;
		// Read ADC1 value
		adc1_cs = true;
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_CurrentMonitor_nCS, GPIO_PIN_SET);
		
		SPIdrv->Receive((void*)&datainv, 1);
		datain = datainv;
		
		VoltageMonitor = 10.0f * 1.0465f * (float32_t)datain / 8192.0f;
		
		while (wait);
		wait = true;
		adc1_cs = false;
		
		// Read ADC2 value
		adc2_cs = true;
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_VoltageMonitor_nCS, GPIO_PIN_SET);
		
		SPIdrv->Receive((void*)&datainv, 1);
		datain = datainv;
		
		CurrentMonitor = 10.0f * (float32_t)datain / 8192.0f;
		
		while (wait);
		wait = true;
		adc2_cs = false;
		
		len = 4;
		if (slot0_id > 0)
			CANReadRegister(SLOT_ID_0, CAN_MESSAGE_TYPE_REGISTER_TEMPERATURE, (uint8_t*)&temperature_slot0, &len);
		len = 4;
		if (slot1_id > 0)
			CANReadRegister(SLOT_ID_1, CAN_MESSAGE_TYPE_REGISTER_TEMPERATURE, (uint8_t*)&temperature_slot1, &len);
		
    osThreadYield ();
  }
}
