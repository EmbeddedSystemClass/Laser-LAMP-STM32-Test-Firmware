
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "DGUS.h"
#include "Driver_USART.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"
#include "SolidStateLaser.h"
#include "SDCard.h"

#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 /* USART Driver */
#ifdef USE_DGUS_DRIVER
extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART* DGUS_USART_Driver;
#endif
 
void MainThread (void const *argument);                             // thread function
osThreadId tid_MainThread;                                          // thread id
osThreadDef (MainThread, osPriorityNormal, 1, 0);                   // thread object

//----------------------------- GUI FRAMES ----------------------------------
extern void     PasswordFrame_Process(uint16_t pic_id);
extern void      ServiceFrame_Process(uint16_t pic_id);
extern void ServiceDiodeFrame_Process(uint16_t pic_id);

extern void   LaserDiodeInput_Init   (uint16_t pic_id);
extern void   LaserDiodeInput_Process(uint16_t pic_id);
extern void LaserDiodePrepare_Process(uint16_t pic_id);
extern void    LaserDiodeWork_Process(uint16_t pic_id);

extern void   SolidStateLaserInput_Init   (uint16_t pic_id);
extern void   SolidStateLaserInput_Process(uint16_t pic_id);
extern void SolidStateLaserPrepare_Process(uint16_t pic_id);
extern void    SolidStateLaserWork_Process(uint16_t pic_id);

extern void    LongPulseLaserInput_Init   (uint16_t pic_id);
extern void    LongPulseLaserInput_Process(uint16_t pic_id);
extern void  LongPulseLaserPrepare_Process(uint16_t pic_id);
extern void     LongPulseLaserWork_Process(uint16_t pic_id);

extern void        FractLaserInput_Init   (uint16_t pic_id);
extern void        FractLaserInput_Process(uint16_t pic_id);
extern void      FractLaserPrepare_Process(uint16_t pic_id);
extern void    			FractLaserWork_Process(uint16_t pic_id);

extern void       WifiScanningFrame_Init   (uint16_t pic_id);
extern void  		  WifiScanningFrame_Process(uint16_t pic_id);
extern void WifiAuthenticationFrame_Process(uint16_t pic_id);
extern void WiFiLinkFrame_Process(void);

extern void CoolingServiceFrame_Process(uint16_t pic_id);

extern void LogFrame_Process(uint16_t pic_id);

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

void DiodeLaserOff()
{
	// Diode Laser Off
	footswitch_en = false;
	DiodeLaser_en = false;
	DiodeControlPulseStop();
	osDelay(100);
	__MISC_LASERDIODE_OFF();
	osDelay(100);
}

void SolidStateLaserOff()
{
	// Solid State Laser Off
	footswitch_en = false;
	SolidStateLaser_en = false;
	LampControlPulseStop();
	osDelay(100);
	__SOLIDSTATELASER_SIMMEROFF();
	osDelay(100);
	__SOLIDSTATELASER_HVOFF();
	osDelay(100);
	__SOLIDSTATELASER_DISCHARGEON();
}

void StopIfRunning(uint16_t pic_id)
{
	// If laser diode running
	if (((pic_id >= 19) && (pic_id <= 32)) || (pic_id == FRAME_PICID_SERVICE_LASERDIODE))
	{
		frameData_LaserDiode.buttons.onInputBtn = 0x00;
		frameData_LaserDiode.buttons.onReadyBtn = 0x00;
		frameData_LaserDiode.buttons.onStartBtn = 0x00;
		frameData_LaserDiode.buttons.onStopBtn  = 0x00;
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		
		DiodeLaserOff();
	}
	
	if (((pic_id >= 35) && (pic_id <= 43)) || (pic_id == FRAME_PICID_SERVICE_SOLIDSTATELASER) || (pic_id == FRAME_PICID_REMOTECONTROL))
	{
		frameData_SolidStateLaser.buttons.onInputBtn = 0x00;
		frameData_SolidStateLaser.buttons.onSimmerBtn = 0x00;
		frameData_SolidStateLaser.buttons.onStartBtn = 0x00;
		frameData_SolidStateLaser.buttons.onStopBtn = 0x00;
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		
		SolidStateLaserOff();
	}
	
	if (((pic_id >= 51) && (pic_id <= 62)))
	{
		frameData_SolidStateLaser.buttons.onInputBtn = 0x00;
		frameData_SolidStateLaser.buttons.onSimmerBtn = 0x00;
		frameData_SolidStateLaser.buttons.onStartBtn = 0x00;
		frameData_SolidStateLaser.buttons.onStopBtn = 0x00;
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		
		SolidStateLaserOff();
	}
	
	if (((pic_id >= 73) && (pic_id <= 83)))
	{
		frameData_SolidStateLaser.buttons.onInputBtn = 0x00;
		frameData_SolidStateLaser.buttons.onSimmerBtn = 0x00;
		frameData_SolidStateLaser.buttons.onStartBtn = 0x00;
		frameData_SolidStateLaser.buttons.onStopBtn = 0x00;
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		
		SolidStateLaserOff();
	}
}

void UpdateLaserState(uint16_t pic_id)
{
	// Enable footswitch when work
	if ((pic_id == FRAME_PICID_LASERDIODE_STARTED) || 
			(pic_id == FRAME_PICID_SOLIDSTATE_WORK) ||
			(pic_id == FRAME_PICID_LONGPULSE_WORK) ||
			(pic_id == FRAME_PICID_FRACTLASER_WORK) ||
			(pic_id == FRAME_PICID_REMOTECONTROL) ||
			(pic_id == FRAME_PICID_SERVICE_SOLIDSTATELASER) ||
			(pic_id == FRAME_PICID_SERVICE_LASERDIODE))
		footswitch_en = true;
	else
		footswitch_en = false;
	
	// Switch to Solid State Laser
#ifdef OLD_STYLE_LASER_SW
	if ((pic_id == FRAME_PICID_SERVICE_SOLIDSTATELASER) || 
			(pic_id == FRAME_PICID_REMOTECONTROL) ||
			((pic_id >= 35) && (pic_id <= 42)))
		__MISC_RELAY2_ON();
	else
		__MISC_RELAY2_OFF();
	
	// Switch to Laser Diode
	if ((pic_id == FRAME_PICID_SERVICE_LASERDIODE) || 
			((pic_id >= 19) && (pic_id <= 32)))
		__MISC_RELAY3_ON();
	else
	{
		__MISC_LASERDIODE_OFF();
		osDelay(100);
		__MISC_RELAY3_OFF();
	}
#else
	if (GetLaserID() == LASER_ID_DIODELASER)
		__MISC_RELAY3_ON();
	else
		__MISC_RELAY3_OFF();
	
	if (GetLaserID() == LASER_ID_SOLIDSTATE || GetLaserID() == LASER_ID_SOLIDSTATE2 || GetLaserID() == LASER_ID_LONGPULSE)
		__MISC_RELAY2_ON();
	else
		__MISC_RELAY2_OFF();
#endif
	
	MenuID = MENU_ID_MENU;
	
	// Check for error state of diode laser
	if ((pic_id >= 19) && (pic_id <= 32))
	{
		//MenuID = MENU_ID_DIODELASER;
		
		// Check temperature
		if (temperature > temperature_overheat)
		{
			DiodeLaserOff();
			SetPicId(FRAME_PICID_LASERDIODE_TEMPERATUREOUT, g_wDGUSTimeout);
		}
		
		// Check flow
		if (flow1 < flow_low)
		{
			DiodeLaserOff();
			SetPicId(FRAME_PICID_LASERDIODE_FLOWERROR, g_wDGUSTimeout);
		}
		
		// Check is working
		if ((pic_id == FRAME_PICID_LASERDIODE_INPUT) || 
			  (pic_id == FRAME_PICID_LASERDIODE_PHOTOTYPE))
		{
			// Peltier off
			CoolOff();
		}
	}
	else
	{
		// Peltier off
		CoolOff();
	}
	
	// Check for errors of solid state laser
	if ((pic_id >= FRAME_PICID_SOLIDSTATE_INPUT) && (pic_id <= FRAME_PICID_SOLIDSTATE_WORK))
	{
		if (GetLaserID() == LASER_ID_SOLIDSTATE)
			MenuID = MENU_ID_SOLIDSTATE;
		
		if (GetLaserID() == LASER_ID_SOLIDSTATE2)
			MenuID = MENU_ID_SOLIDSTATE2;
		
		// Check temperature
		if (temperature > temperature_overheat_solidstate)
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_SOLIDSTATE_OVERHEATING, g_wDGUSTimeout);
		}
		
		// Check flow
		if (flow1 < flow_low)
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_SOLIDSTATE_FLOWERROR, g_wDGUSTimeout);
		}
		
		// Fault check
		if (__MISC_GETCHARGEMODULEFAULTSTATE())
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_SOLIDSTATE_FAULT, g_wDGUSTimeout);
		}
	}
	
	// Check for errors of fractional laser
	if ((pic_id >= FRAME_PICID_FRACTLASER_INPUT) && (pic_id <= FRAME_PICID_FRACTLASER_WORK))
	{		
		if (GetLaserID() == LASER_ID_FRACTLASER)
			MenuID = MENU_ID_FRACTLASER;
		
		// Check temperature
		if (temperature > temperature_overheat_solidstate)
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_FRACTLASER_OVERHEATING, g_wDGUSTimeout);
		}
		
		// Check flow
		if (flow1 < flow_low)
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_FRACTLASER_FLOWERROR, g_wDGUSTimeout);
		}
		
		// Fault check
		if (__MISC_GETCHARGEMODULEFAULTSTATE())
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_FRACTLASER_FAULT, g_wDGUSTimeout);
		}
	}
	
	// Check for errors of solid state laser long pulse
	if ((pic_id >= 51) && (pic_id <= 58))
	{
		MenuID = MENU_ID_LONGPULSE;
		
		// Check temperature
		if (temperature > temperature_overheat_solidstate)
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_LONGPULSE_OVERHEATING, g_wDGUSTimeout);
		}
		
		// Check flow
		if (flow1 < flow_low)
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_LONGPULSE_FLOWERROR, g_wDGUSTimeout);
		}
		
		// Fault check
		if (__MISC_GETCHARGEMODULEFAULTSTATE())
		{
			SolidStateLaserOff();
			SetPicId(FRAME_PICID_LONGPULSE_FAULT, g_wDGUSTimeout);
		}
	}
}

DGUS_LASERDIODE_STATE laser_state;
extern volatile float32_t temperature;

void UpdateLaserStatus()
{
	laser_state.temperature = (uint16_t)(temperature * 10.0f);
	
	if (flow1 < flow_low)
		laser_state.coolIcon = 1;
	else
	if (flow1 < flow_normal)
		laser_state.coolIcon = 2;
	else
	//if (flow1 < 7)
		laser_state.coolIcon = 3;
	
	laser_state.flow = flow1 * 10;
	
	WriteVariableConvert16(FRAMEDATA_LASERSTATE_BASE, &laser_state, sizeof(laser_state));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}

void MainThread (void const *argument) {
	static uint16_t last_pic_id = 0;
	static MENU_ID last_menu_id = MENU_ID_MENU;
	
	LaserDiodeInput_Init(pic_id);
	SolidStateLaserInput_Init(pic_id);
	LongPulseLaserInput_Init(pic_id);
	FractLaserInput_Init(pic_id);
	
	GetDateTime(g_wDGUSTimeout, &datetime);

  while (1) {
    ; // Insert thread code here...
		pic_id = GetPicId(g_wDGUSTimeout, pic_id);		
		GetDateTime(g_wDGUSTimeout, &datetime);
		
		last_menu_id = MenuID;
		UpdateLaserState(pic_id);
		
		LaserID = GetLaserID();
		
		// GUI Frames process
		switch (pic_id)
		{
			case FRAME_PICID_LOGO:
				SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				break;
			
			// Frames process
			case FRAME_PICID_PASSWORD:									
				PasswordFrame_Process(pic_id);	
				break;
			case FRAME_PICID_SERVICE_SOLIDSTATELASER:		
				ServiceFrame_Process(pic_id);	
				break;
			case FRAME_PICID_SERVICE_LASERDIODE:				
				ServiceDiodeFrame_Process(pic_id);
				break;
			case FRAME_PICID_SERVICE_BASICSETTINGS:			break; // Blank picture, for future release
			case FRAME_PICID_SERVICECOOLING:
				CoolingServiceFrame_Process(pic_id);
				UpdateLaserStatus();
				break;
			
			// Laser Diode control
			case FRAME_PICID_LASERDIODE_INPUT:
#ifdef LASERIDCHECK_LASERDIODE
				if (GetLaserID() == LASER_ID_DIODELASER)
				{
					if (last_pic_id != pic_id && last_menu_id != MenuID)
						LaserDiodeInput_Init(pic_id);
					LaserDiodeInput_Process(pic_id);
				}
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
#else
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					LaserDiodeInput_Init(pic_id);
				LaserDiodeInput_Process(pic_id);
#endif
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LASERDIODE_TEMPERATUREOUT:
			case FRAME_PICID_LASERDIODE_PREPARETIMER:
			case FRAME_PICID_LASERDIODE_FLOWERROR:
				LaserDiodePrepare_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LASERDIODE_READY:
			case FRAME_PICID_LASERDIODE_START:
			case FRAME_PICID_LASERDIODE_STARTED:
				LaserDiodeWork_Process(pic_id);
			case FRAME_PICID_LASERDIODE_PHOTOTYPE:
				UpdateLaserStatus();
				break;
			
			// WiFi features
			case FRAME_PICID_SERVICE_WIFISCANNINGINIT:
				WifiScanningFrame_Init(pic_id);
				break;
			case FRAME_PICID_SERVICE_WIFISCANNING:			
				WifiScanningFrame_Process(pic_id);
				break;
			case FRAME_PICID_SERVICE_WIFIAUTHENTICATION:
				WifiAuthenticationFrame_Process(pic_id);
				break;
			case FRAME_PICID_WIFI_LINKING:
				WiFiLinkFrame_Process();
				break;
			
			// App idle user state
			case FRAME_PICID_MAINMENU:
				if (ip_addr_updated)
				{
					WriteVariable(FRAMEDATA_WIFIUP_IPADDR, ip_addr, 16);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
					ip_addr_updated = false;
				}
				StopIfRunning(last_pic_id);
			case FRAME_PICID_SERVICE:
			case FRAME_PICID_LANGMENU:
				// nothing to do
				break;
			
			// Solid State Laser
			case FRAME_PICID_SOLIDSTATE_INPUT:	
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					SolidStateLaserInput_Init(pic_id);
				if (GetLaserID() == LASER_ID_SOLIDSTATE || GetLaserID() == LASER_ID_SOLIDSTATE2)				
					SolidStateLaserInput_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_SOLIDSTATE_SIMMERSTART:
			case FRAME_PICID_SOLIDSTATE_SIMMER:
			case FRAME_PICID_SOLIDSTATE_START:
			case FRAME_PICID_SOLIDSTATE_WORK:
				SolidStateLaserWork_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_SOLIDSTATE_FLOWERROR:
			case FRAME_PICID_SOLIDSTATE_OVERHEATING:
			case FRAME_PICID_SOLIDSTATE_FAULT:
				SolidStateLaserPrepare_Process(pic_id);
				UpdateLaserStatus();
				break;
			
			// Long Pulse Laser
			case FRAME_PICID_LONGPULSE_INPUT:	
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					LongPulseLaserInput_Init(pic_id);
				if (GetLaserID() == LASER_ID_LONGPULSE)
					LongPulseLaserInput_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
				UpdateLaserStatus();
				/*
				if (GetLaserID() == LASER_ID_LONGPULSE)				
					LongPulseLaserInput_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);*/
				break;
			case FRAME_PICID_LONGPULSE_SIMMERSTART:
			case FRAME_PICID_LONGPULSE_SIMMER:
				if (GetLaserID() == LASER_ID_LONGPULSE)				
					LongPulseLaserWork_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LONGPULSE_START:
			case FRAME_PICID_LONGPULSE_WORK:
				LongPulseLaserWork_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LONGPULSE_FLOWERROR:
			case FRAME_PICID_LONGPULSE_OVERHEATING:
			case FRAME_PICID_LONGPULSE_FAULT:
				LongPulseLaserPrepare_Process(pic_id);
				UpdateLaserStatus();
				break;
			
			// Fractional Laser
			case FRAME_PICID_FRACTLASER_INPUT:	
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					FractLaserInput_Init(pic_id);
				if (GetLaserID() == LASER_ID_FRACTLASER)
					FractLaserInput_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_FRACTLASER_SIMMERSTART:
			case FRAME_PICID_FRACTLASER_SIMMER:
			case FRAME_PICID_FRACTLASER_START:
			case FRAME_PICID_FRACTLASER_WORK:
				FractLaserWork_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_FRACTLASER_FLOWERROR:
			case FRAME_PICID_FRACTLASER_OVERHEATING:
			case FRAME_PICID_FRACTLASER_FAULT:
				FractLaserPrepare_Process(pic_id);
				UpdateLaserStatus();
				break;
			
			case FRAME_PICID_BASICSETTINGS:
				UpdateLaserStatus();
				break;
				
			case FRAME_PICID_LOGVIEW:
				LogFrame_Process(pic_id);
				break;
			
			case FRAME_PICID_WRONG_EMMITER:
				osDelay(800);
				SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				break;
			
			case FRAME_PICID_REMOTECONTROL:
				break;
			
			default:
				SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				break;
		}
		
		// Remote control
		if (RemoteControl)
		{
			if (pic_id == FRAME_PICID_MAINMENU)
				SetPicId(FRAME_PICID_REMOTECONTROL, g_wDGUSTimeout);
		}
		else
			if (pic_id == FRAME_PICID_REMOTECONTROL)
			{
				SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				//pic_id = FRAME_PICID_MAINMENU;
			}
		
		last_pic_id = pic_id;
		
    osThreadYield ();                                           // suspend thread
  }
}
