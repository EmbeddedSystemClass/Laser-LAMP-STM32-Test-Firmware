#include "cmsis_os.h"                                           // CMSIS RTOS header file

// Local headers
#include "GlobalVariables.h"
#include "rtEventSystem.h"
#include "Driver_USART.h"
#include "SDCardBase.h"
#include "DGUSBase.h"

// Standart library
#include <string.h>
#include <stdlib.h>

void MainThread (void const *argument);                             // thread function
osThreadId tid_MainThread;                                          // thread id
osThreadDef (MainThread, osPriorityNormal, 1, 0);                   // thread object

int Init_Main_Thread (void) {

  tid_MainThread = osThreadCreate (osThread(MainThread), NULL);
  if (!tid_MainThread) return(-1);
	
  return(0);
}

extern volatile uint16_t firmware_update_progress;
extern volatile char FirmwareVersion[16];

void MainThread (void const *argument) {	
	GetDateTime(g_wDGUSTimeout, &datetime);

  while (1) {
    ; // Insert thread code here...
		pic_id = GetPicId(g_wDGUSTimeout, pic_id);		
		GetDateTime(g_wDGUSTimeout, &datetime);
		
		switch (pic_id)
		{
			case FRAME_PICID_BOOTMENU:
			case FRAME_PICID_BOOTFIRMWAREUPDATE:
				WriteVariableConvert16(FRAMEDATA_FIRMWAREUPDATE, (void*)&firmware_update_progress, sizeof(uint16_t));
				rtSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
			
				WriteVariable(FRAMEDATA_FIRMWAREVERSION, (void*)FirmwareVersion, 16);
				rtSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
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
