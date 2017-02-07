#include "cmsis_os.h"                                           // CMSIS RTOS header file

// Local headers
#include "GlobalVariables.h"
#include "Driver_USART.h"
#include "SDCardBase.h"
#include "DGUS.h"

// Standart library
#include <string.h>
#include <stdlib.h>

 /* USART Driver */
#ifdef USE_DGUS_DRIVER
extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART* DGUS_USART_Driver;
#endif

void MainThread (void const *argument);                             // thread function
osThreadId tid_MainThread;                                          // thread id
osThreadDef (MainThread, osPriorityNormal, 1, 0);                   // thread object

int Init_Main_Thread (void) {

  tid_MainThread = osThreadCreate (osThread(MainThread), NULL);
  if (!tid_MainThread) return(-1);
	
#ifdef USE_DGUS_DRIVER
	/*Initialize the USART driver */
  Driver_USART1.Initialize(DWIN_USART_callback);
  /*Power up the USART peripheral */
  Driver_USART1.PowerControl(ARM_POWER_FULL);
	
  /*Configure the USART to 115200 Bits/sec */
  Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
												ARM_USART_DATA_BITS_8 |
												ARM_USART_PARITY_NONE |
												ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_NONE, DGUS_BAUDRATE);
	
	/* Enable Receiver and Transmitter lines */
  Driver_USART1.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);
	
	/* Set DGUS driver */
	DGUS_USART_Driver = &Driver_USART1;
#endif
	
  return(0);
}

extern volatile uint16_t firmware_update_progress;
extern volatile char FirmwareVersion[16];

void MainThread (void const *argument) {	
	uint16_t last_id = 0;
	GetDateTime(g_wDGUSTimeout, &datetime);

  while (1) {
    ; // Insert thread code here...
		
		last_id = pic_id;
		pic_id = GetPicId(g_wDGUSTimeout, pic_id);		
		GetDateTime(g_wDGUSTimeout, &datetime);
		
		switch (pic_id)
		{
			case FRAME_PICID_BOOTMENU:
			case FRAME_PICID_BOOTFIRMWAREUPDATE:
				WriteVariableConvert16(FRAMEDATA_FIRMWAREUPDATE, (void*)&firmware_update_progress, sizeof(uint16_t));
				osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
			
				WriteVariable(FRAMEDATA_FIRMWAREVERSION, (void*)FirmwareVersion, 16);
				osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
				break;
			case FRAME_PICID_LOGO:
				SetPicId(FRAME_PICID_BOOTMENU, g_wDGUSTimeout);
				break;
			default:
				SetPicId(FRAME_PICID_BOOTMENU, g_wDGUSTimeout);
				break;
		}
		
		osThreadYield ();                                           		// suspend thread
	}
}
