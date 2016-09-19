
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "DGUS.h"
#include "Driver_USART.h"
#include "LaserMisc.h"

#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 /* USART Driver */
extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART* DGUS_USART_Driver;
 
void MainThread (void const *argument);                             // thread function
osThreadId tid_MainThread;                                          // thread id
osThreadDef (MainThread, osPriorityNormal, 1, 0);                   // thread object

//----------------------------- GUI FRAMES ----------------------------------
extern void     PasswordFrame_Process(uint16_t pic_id);
extern void      ServiceFrame_Process(uint16_t pic_id);
extern void   LaserDiodeInput_Process(uint16_t pic_id);
extern void LaserDiodePrepare_Process(uint16_t pic_id);

int Init_Main_Thread (void) {

  tid_MainThread = osThreadCreate (osThread(MainThread), NULL);
  if (!tid_MainThread) return(-1);
	
	/*Initialize the USART driver */
  Driver_USART1.Initialize(DWIN_USART_callback);
  /*Power up the USART peripheral */
  Driver_USART1.PowerControl(ARM_POWER_FULL);
	
  /*Configure the USART to 115200 Bits/sec */
  Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
												ARM_USART_DATA_BITS_8 |
												ARM_USART_PARITY_NONE |
												ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_NONE, 115200);
	
	/* Enable Receiver and Transmitter lines */
  Driver_USART1.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);
	
	/* Set DGUS driver */
	DGUS_USART_Driver = &Driver_USART1;
	
  return(0);
}

void LaserSwitch(uint16_t pic_id)
{
	// Switch to Solid State Laser
	if (pic_id == FRAME_PICID_SERVICE_SOLIDSTATELASER)
		__MISC_RELAY2_ON();
	else
		__MISC_RELAY2_OFF();
	
	// Switch to Laser Diode
	if ((pic_id == FRAME_PICID_SERVICE_LASERDIODE) || 
			((pic_id >= 19) && (pic_id <= 31)))
		__MISC_RELAY3_ON();
	else
		__MISC_RELAY3_OFF();
}

DGUS_LASERDIODE_STATE laser_state;
extern volatile float32_t temperature;

void UpdateLaserStatus()
{
	laser_state.temperature = (uint16_t)(temperature * 10.0f);
	laser_state.coolIcon = 1;
	laser_state.flow = 10;
	
	WriteVariableConvert16(FRAMEDATA_LASERSTATE_BASE, &laser_state, sizeof(laser_state));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
}

void MainThread (void const *argument) {
	static uint16_t last_pic_id = 0;
	uint16_t pic_id;

  while (1) {
    ; // Insert thread code here...
		pic_id = GetPicId(100);		
		LaserSwitch(pic_id);
		
		// GUI Frames process
		switch (pic_id)
		{
			case FRAME_PICID_LOGO:
				SetPicId(FRAME_PICID_MAINMENU, 100);
				break;
			
			// Frames process
			case FRAME_PICID_PASSWORD:									
				PasswordFrame_Process(pic_id);	
				break;
			case FRAME_PICID_SERVICE_SOLIDSTATELASER:		
				ServiceFrame_Process(pic_id);	
				break;
			case FRAME_PICID_SERVICE_LASERDIODE:				break; // Blank picture, for future release
			case FRAME_PICID_SERVICE_BASICSETTINGS:			break; // Blank picture, for future release
			
			// Laser Diode control
			case FRAME_PICID_LASERDIODE_INPUT:
				LaserDiodeInput_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LASERDIODE_TEMPERATUREOUT:
			case FRAME_PICID_LASERDIODE_PREPARETIMER:
				LaserDiodePrepare_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LASERDIODE_READY:
			case FRAME_PICID_LASERDIODE_START:
			case FRAME_PICID_LASERDIODE_STARTED:
			case FRAME_PICID_LASERDIODE_PHOTOTYPE:
				UpdateLaserStatus();
				break;
			
			// WiFi features
			case FRAME_PICID_SERVICE_WIFISCANNING:			break; // Blank picture, for future release
			case FRAME_PICID_SERVICE_WIFIAUTHENTICATION:break; // Blank picture, for future release
			
			// App idle user state
			case FRAME_PICID_MAINMENU:
			case FRAME_PICID_SERVICE:
			case FRAME_PICID_LANGMENU:
				// nothing to do
				break;
			
			default:
				SetPicId(FRAME_PICID_MAINMENU, 100);
				break;
		}
		
		last_pic_id = pic_id;
		
    osThreadYield ();                                           // suspend thread
  }
}
