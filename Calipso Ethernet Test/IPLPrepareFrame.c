#include <string.h>
#include "Driver_USART.h"

#include "DGUS.h"
#include "SDCard.h"
#include "LaserMisc.h"
#include "SolidStateLaser.h"

// Calypso System Global Configuration and Variables
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void ProcessIPLSimmerSensor(void);
bool IgnitionIPLState(void);
bool ChargingIPLState(void);

static uint32_t ipl_simmer_sensor_filter = 0;
static uint32_t ipl_charging_sensor_filter = 0;
static uint16_t stacked_pic_id = FRAME_PICID_IPL_COOLING_TIMER;

void IPLPrepare_Process(uint16_t pic_id)
{
	volatile bool update = false;
	uint16_t new_pic_id = pic_id;
	
	uint16_t timer_min = frameData_LaserDiode.timer.timer_minutes;
	uint16_t timer_sec = frameData_LaserDiode.timer.timer_seconds;
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(DGUS_LASERDIODE));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	// Prepare states
	switch (pic_id)
	{
		case FRAME_PICID_IPL_BATTERY_CHARGING:
			if (ChargingIPLState())
			{
#ifdef LDPREPARETIMER_ENABLE
				new_pic_id = FRAME_PICID_IPL_COOLING_TIMER;
#else
				new_pic_id = FRAME_PICID_IPL_START;
#endif
				update = true;
			}
			break;
		case FRAME_PICID_IPL_COOLING_TIMER:
			CoolSet((frameData_LaserDiode.cooling + 1) * 17);
			frameData_LaserDiode.timer.timer_minutes = m_wMinutes;
			frameData_LaserDiode.timer.timer_seconds = m_wSeconds;
			if (!prepare)
			{
				frameData_LaserDiode.state = 1;
				new_pic_id = FRAME_PICID_IPL_START;
				update = true;
			}
			break;
	}
	if (timer_min != frameData_LaserDiode.timer.timer_minutes ||
			timer_sec != frameData_LaserDiode.timer.timer_seconds)
		update = true;
	
	// Error states
	switch (pic_id)
	{
		case FRAME_PICID_IPL_FLOWERROR:
			frameData_LaserDiode.timer.timer_minutes = (uint16_t)flow2;
			frameData_LaserDiode.timer.timer_seconds = (uint16_t)(flow2 * 10.0f) % 10;
			update = true;	
			break;
		case FRAME_PICID_IPL_OVERHEATING:
			frameData_LaserDiode.timer.timer_minutes = (uint16_t)temperature;
			frameData_LaserDiode.timer.timer_seconds = (uint16_t)(temperature * 10.0f) % 10;
			update = true;
			break;
		default:
			if (!IgnitionIPLState())
			{
				new_pic_id = FRAME_PICID_IPL_IGNITION;
				update = true;
			}
	}
	
	if (frameData_LaserDiode.buttons.onCancelBtn != 0 || frameData_LaserDiode.buttons.onRestartBtn != 0)
	{
		// On Stop Pressed
		frameData_LaserDiode.buttons.onCancelBtn = 0;
		frameData_LaserDiode.buttons.onRestartBtn = 0;
		new_pic_id = FRAME_PICID_IPL_IGNITION;
		update = true;
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		
		__SOLIDSTATELASER_SIMMEROFF();
		__SOLIDSTATELASER_HVOFF();
		__SOLIDSTATELASER_DISCHARGEON();
		SetDACValue(0.0f);
	
		StoreGlobalVariables();
	}
	
	// Update IPL counter
	if (frameData_LaserDiode.PulseCounter != GetSolidStateGlobalPulse(slot1_id))
	{
		frameData_LaserDiode.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
		frameData_LaserDiode.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);
		update = true;
	}
	
	// Update IPL interface
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	// Update IPL menu
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}

void _IPLOff()
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
	CoolOff();
}

void IPLErrorCheck_Process(uint16_t pic_id)
{
	// Check for errors of IPL
	if ((pic_id >= FRAME_PICID_IPL_IGNITION) && (pic_id <= FRAME_PICID_IPL_WORK))
	{		
		ProcessIPLSimmerSensor();
		
		if (slot1_id == LASER_ID_IPL)
			MenuID = MENU_ID_IPL;
		
		// Check temperature
		if (temperature > temperature_overheat_solidstate)
		{
			_IPLOff();
			stacked_pic_id = pic_id;
			SetPicId(FRAME_PICID_IPL_OVERHEATING, g_wDGUSTimeout);
		}
		
#ifdef FLOW_CHECK
		// Check flow
		if (flow1 < flow_low)
		{
			_IPLOff();
			stacked_pic_id = pic_id;
			SetPicId(FRAME_PICID_IPL_FLOWERROR, g_wDGUSTimeout);
		}
#endif
		
		// Fault check
		if (__MISC_GETCHARGEMODULEFAULTSTATE())
		{
			_IPLOff();
			stacked_pic_id = pic_id;
			SetPicId(FRAME_PICID_IPL_PCA10_FAULT, g_wDGUSTimeout);
		}
	}
}

void IPLStopIfWork(uint16_t pic_id)
{
	if (((pic_id >= FRAME_PICID_IPL_IGNITION) && (pic_id <= FRAME_PICID_IPL_PDD_FAULT)))
	{
		frameData_LaserDiode.buttons.onCancelBtn = 0x00;
		frameData_LaserDiode.buttons.onIgnitionBtn = 0x00;
		frameData_LaserDiode.buttons.onInputBtn = 0x00;
		frameData_LaserDiode.buttons.onReadyBtn = 0x00;
		frameData_LaserDiode.buttons.onRestartBtn = 0x00;
		frameData_LaserDiode.buttons.onStartBtn = 0x00;
		frameData_LaserDiode.buttons.onStopBtn = 0x00;
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		
		CoolOff();
		_IPLOff();
	}
}

bool IgnitionIPLState(void)
{
	return (ipl_simmer_sensor_filter > 5);
}

bool ChargingIPLState(void)
{
	return __MISC_GETCHARGEMODULEREADYSTATE();
}

void ProcessIPLSimmerSensor(void)
{
	if (__MISC_GETSIMMERSENSOR())
	{
		if (ipl_simmer_sensor_filter < 10)
			ipl_simmer_sensor_filter++;
	}
	else
	{
		if (ipl_simmer_sensor_filter > 0)
			ipl_simmer_sensor_filter--;
	}
}
