
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

void DiodeLaserOff()
{
	// Diode Laser Off
	footswitch_en = false;
	DiodeLaser_en = false;
	DiodeControlPulseStop();
	osDelay(100);
	__MISC_LASERDIODE_OFF();
	osDelay(100);
	CoolOff1();
}

void SolidStateLaserOff()
{
	// Solid State Laser Off
	footswitch_en = false;
	SolidStateLaser_en = false;
	LampControlPulseStop();
	osDelay(100);
	NBU1012_SimmerOff();
	osDelay(100);
	PCA10_HVOff();
	osDelay(100);
	NBU1012_DischargeOn();
}

//----------------------------- GUI FRAMES ----------------------------------
extern void           PasswordFrame_Process(uint16_t pic_id); // Service menu
extern void            ServiceFrame_Process(uint16_t pic_id);
extern void       ServiceDiodeFrame_Process(uint16_t pic_id);
            
extern void         LaserDiodeInput_Init   (uint16_t pic_id); // Diode laser
extern void         LaserDiodeInput_Process(uint16_t pic_id);
extern void       LaserDiodePrepare_Process(uint16_t pic_id);
extern void          LaserDiodeWork_Process(uint16_t pic_id);
extern void  	LaserDiodeErrorCheck_Process(uint16_t pic_id);
            
extern void    SolidStateLaserInput_Init   (uint16_t pic_id); // Nd YAG Qsw 1064nm
extern void    SolidStateLaserInput_Process(uint16_t pic_id);
extern void  SolidStateLaserPrepare_Process(uint16_t pic_id);
extern void     SolidStateLaserWork_Process(uint16_t pic_id);
extern void  	SolidStateErrorCheck_Process(uint16_t pic_id);
            
extern void     LongPulseLaserInput_Init   (uint16_t pic_id); // Long Pulse (Nd YAG 1064nm)
extern void     LongPulseLaserInput_Process(uint16_t pic_id);
extern void   LongPulseLaserPrepare_Process(uint16_t pic_id);
extern void      LongPulseLaserWork_Process(uint16_t pic_id);
extern void     LongPulseErrorCheck_Process(uint16_t pic_id);
            
extern void         FractLaserInput_Init   (uint16_t pic_id); // Nd YAG 1440
extern void         FractLaserInput_Process(uint16_t pic_id);
extern void       FractLaserPrepare_Process(uint16_t pic_id);
extern void     			FractLaserWork_Process(uint16_t pic_id);
extern void    FractLaserErrorCheck_Process(uint16_t pic_id);
            
extern void                   IPLInput_Init(uint16_t pic_id); // IPL menu
extern void                IPLInput_Process(uint16_t pic_id);
extern void              IPLPrepare_Process(uint16_t pic_id);
extern void                 IPLWork_Process(uint16_t pic_id);
extern void           IPLErrorCheck_Process(uint16_t pic_id);

extern void       WifiScanningFrame_Init   (uint16_t pic_id); // WiFi scanning menu
extern void  		  WifiScanningFrame_Process(uint16_t pic_id);
extern void WifiAuthenticationFrame_Process(uint16_t pic_id);
extern void WiFiLinkFrame_Process(void);

//----------------------------- GUI Preset Frames ---------------------------
extern void            LaserDiodeStopIfWork(uint16_t pic_id);
extern void             LongPulseStopIfWork(uint16_t pic_id);
extern void            SolidStateStopIfWork(uint16_t pic_id);
extern void            FractLaserStopIfWork(uint16_t pic_id);
extern void                   IPLStopIfWork(uint16_t pic_id);

//----------------------------- GUI Basic settings --------------------------
extern void CoolingServiceFrame_Process(uint16_t pic_id);
extern void LogFrame_Process(uint16_t pic_id);

/* USART Driver Initialization */
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

// Preset GUI frames when exit to main menu
void StopIfRunning(uint16_t pic_id)
{		
	LaserDiodeStopIfWork(pic_id);
	LongPulseStopIfWork(pic_id);
	SolidStateStopIfWork(pic_id);
	FractLaserStopIfWork(pic_id);
	IPLStopIfWork(pic_id);	
}

// Error checking process, enable or disable main laser control
void UpdateLaserState(uint16_t pic_id)
{
	// Enable footswitch when work
	if ((pic_id == FRAME_PICID_LASERDIODE_STARTED) || 
			(pic_id == FRAME_PICID_SOLIDSTATE_WORK) ||
			(pic_id == FRAME_PICID_LONGPULSE_WORK) ||
			(pic_id == FRAME_PICID_FRACTLASER_WORK) ||
			(pic_id == FRAME_PICID_REMOTECONTROL) ||
			(pic_id == FRAME_PICID_SERVICE_SOLIDSTATELASER) ||
			(pic_id == FRAME_PICID_SERVICE_LASERDIODE) ||
			(pic_id == FRAME_PICID_IPL_WORK))
		footswitch_en = true;  // Enable laser control
	else
		footswitch_en = false; // Disable lase control
	
	// Laser connector switch
#ifdef OLD_STYLE_LASER_SW
	// Switch to Solid State Laser
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
	
	if (GetLaserID() == LASER_ID_SOLIDSTATE || GetLaserID() == LASER_ID_SOLIDSTATE2 || GetLaserID() == LASER_ID_LONGPULSE || GetLaserID() == LASER_ID_IPL || GetLaserID() == LASER_ID_1340NM)
		__MISC_RELAY2_ON();
	else
		__MISC_RELAY2_OFF();
#endif
	
	MenuID = MENU_ID_MENU;
	
	//Error checking and update MenuID
	LaserDiodeErrorCheck_Process(pic_id);
	SolidStateErrorCheck_Process(pic_id);
	LongPulseErrorCheck_Process(pic_id);
	FractLaserErrorCheck_Process(pic_id);
	IPLErrorCheck_Process(pic_id);
}

DGUS_LASERDIODE_STATE laser_state;
extern volatile float32_t temperature;

void UpdateLaserStatus()
{
	laser_state.temperature = (uint16_t)(temperature * 10.0f);
	
	if ((pic_id >= FRAME_PICID_LASERDIODE_INPUT) && (pic_id <= FRAME_PICID_LASERDIODE_PHOTOTYPE))
	{
		if (flow2 < flow_low)
			laser_state.coolIcon = 1;
		else
		if (flow2 < flow_normal)
			laser_state.coolIcon = 2;
		else
		//if (flow1 < 7)
			laser_state.coolIcon = 3;
	}
	else
	{
		if (flow1 < flow_low)
			laser_state.coolIcon = 1;
		else
		if (flow1 < flow_normal)
			laser_state.coolIcon = 2;
		else
		//if (flow1 < 7)
			laser_state.coolIcon = 3;
	}
	
	//laser_state.flow = (flow1 + flow2) * 10;
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
	IPLInput_Init(pic_id);
	
	StopIfRunning(pic_id);
	CoolOff1();
	CoolOff2();
	
	GetDateTime(g_wDGUSTimeout, &datetime);

  while (1) {
    ; // Insert thread code here...
		pic_id = GetPicId(g_wDGUSTimeout, pic_id);		
		GetDateTime(g_wDGUSTimeout, &datetime);
		
		last_menu_id = MenuID;
		UpdateLaserState(pic_id);
		
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
				if (CheckEmmiter(LASER_ID_DIODELASER))
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
			case FRAME_PICID_LASERDIODE_PHOTOTYPE+1:
				SetPicId(FRAME_PICID_LASERDIODE_PHOTOTYPE, 1000);
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
			case FRAME_PICID_MAINMENU_IPL:
				if (ip_addr_updated)
				{
					WriteVariable(FRAMEDATA_WIFIUP_IPADDR, ip_addr, 16);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
					ip_addr_updated = false;
				}
#ifndef MAINMENU_IPL
				if ((LaserSet & LASER_ID_MASK_IPL) == 0)	SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
#endif
				StopIfRunning(last_pic_id);
				break;
			case FRAME_PICID_MAINMENU_QSW:
				if (ip_addr_updated)
				{
					WriteVariable(FRAMEDATA_WIFIUP_IPADDR, ip_addr, 16);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
					ip_addr_updated = false;
				}
				if ((LaserSet & LASER_ID_MASK_SOLIDSTATE) == 0)	SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				StopIfRunning(last_pic_id);
				break;
			case FRAME_PICID_MAINMENU_LONGPULSE:
				if (ip_addr_updated)
				{
					WriteVariable(FRAMEDATA_WIFIUP_IPADDR, ip_addr, 16);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
					ip_addr_updated = false;
				}
				if ((LaserSet & LASER_ID_MASK_LONGPULSE) == 0)	SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				StopIfRunning(last_pic_id);
				break;
			case FRAME_PICID_MAINMENU_FRACTLASER:
				if (ip_addr_updated)
				{
					WriteVariable(FRAMEDATA_WIFIUP_IPADDR, ip_addr, 16);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
					ip_addr_updated = false;
				}
				if (((LaserSet & LASER_ID_MASK_FRACTIONAL) == 0) && ((LaserSet & LASER_ID_MASK_1340NM) == 0) && ((LaserSet & LASER_ID_MASK_2940NM) == 0))
					SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
				StopIfRunning(last_pic_id);
				break;
			case FRAME_PICID_MAINMENU:
#ifdef MAINMENU_IPL
				SetPicId(FRAME_PICID_MAINMENU_IPL, g_wDGUSTimeout);
#else
				if (LaserSet & LASER_ID_MASK_SOLIDSTATE)	SetPicId(FRAME_PICID_MAINMENU_QSW, g_wDGUSTimeout);
				if (LaserSet & LASER_ID_MASK_SOLIDSTATE2)	SetPicId(FRAME_PICID_MAINMENU_QSW, g_wDGUSTimeout);
				if (LaserSet & LASER_ID_MASK_LONGPULSE)		SetPicId(FRAME_PICID_MAINMENU_LONGPULSE, g_wDGUSTimeout);
				if (LaserSet & LASER_ID_MASK_FRACTIONAL)	SetPicId(FRAME_PICID_MAINMENU_FRACTLASER, g_wDGUSTimeout);
				if (LaserSet & LASER_ID_MASK_1340NM)			SetPicId(FRAME_PICID_MAINMENU_FRACTLASER, g_wDGUSTimeout);
				if (LaserSet & LASER_ID_MASK_2940NM)			SetPicId(FRAME_PICID_MAINMENU_FRACTLASER, g_wDGUSTimeout);
				if (LaserSet & LASER_ID_MASK_IPL)					SetPicId(FRAME_PICID_MAINMENU_IPL, g_wDGUSTimeout);
#endif
				if (ip_addr_updated)
				{
					WriteVariable(FRAMEDATA_WIFIUP_IPADDR, ip_addr, 16);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
					ip_addr_updated = false;
				}
				StopIfRunning(last_pic_id);
				break;
			case FRAME_PICID_SERVICE:
			case FRAME_PICID_LANGMENU:
				// nothing to do
				break;
			
			// Solid State Laser
			case FRAME_PICID_SOLIDSTATE_INPUT:	
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					SolidStateLaserInput_Init(pic_id);
				if (CheckEmmiter(LASER_ID_SOLIDSTATE) || CheckEmmiter(LASER_ID_SOLIDSTATE2))
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
				if (CheckEmmiter(LASER_ID_LONGPULSE))
					LongPulseLaserInput_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_LONGPULSE_SIMMERSTART:
			case FRAME_PICID_LONGPULSE_SIMMER:
				if (CheckEmmiter(LASER_ID_LONGPULSE))
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
				if (CheckEmmiter(LASER_ID_FRACTLASER) || CheckEmmiter(LASER_ID_1340NM) || CheckEmmiter(LASER_ID_2940NM))
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
			
			// IPL menu
			case FRAME_PICID_IPL_IGNITION:
			case FRAME_PICID_IPL_IGNITION_PROCESS:
			case FRAME_PICID_IPL_INPUT:
#ifdef LASERIDCHECK_IPL
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					IPLInput_Init(pic_id);
				if (CheckEmmiter(LASER_ID_IPL))
					IPLInput_Process(pic_id);
				else
					SetPicId(FRAME_PICID_WRONG_EMMITER, g_wDGUSTimeout);
#else
				if (last_pic_id != pic_id && last_menu_id != MenuID)
					IPLInput_Init(pic_id);
				IPLInput_Process(pic_id);
#endif
				
				UpdateLaserStatus();
				break;
			case FRAME_PICID_IPL_BATTERY_CHARGING:
			case FRAME_PICID_IPL_COOLING_TIMER:
				IPLPrepare_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_IPL_START:
			case FRAME_PICID_IPL_WORK:
				IPLWork_Process(pic_id);
				UpdateLaserStatus();
				break;
			case FRAME_PICID_IPL_FLOWERROR:
			case FRAME_PICID_IPL_PCA10_FAULT:
			case FRAME_PICID_IPL_PDD_FAULT:
			case FRAME_PICID_IPL_OVERHEATING:
				IPLPrepare_Process(pic_id);
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
			
#ifdef PCA10_ADAPTER_SUPPORT
		uint8_t len = 2;
		CAN2ReadRegister(SLOT_ID_1, CAN_MESSAGE_TYPE_REGISTER_STATUS, (uint8_t*)&pca10_adapter_status, &len);
#endif
		
    osThreadYield ();                                           // suspend thread
  }
}
