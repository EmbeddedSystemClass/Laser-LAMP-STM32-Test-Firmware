#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"
#include "LP_Energy_Table.h"

#include <math.h>
#include "arm_math.h"

#define SHORT_DURATIONS_NUM	10
#define STDRD_DURATIONS_NUM	9
#define LONGP_DURATIONS_NUM	10
#define VOLTAGES_SNUM 8
#define VOLTAGES_NUM 16//8

extern void SetDACValue(float32_t value);

float32_t _programI;

uint16_t duration;
uint16_t voltage;

void LongPulseLaserInput_Init(uint16_t pic_id)
{
	frameData_SolidStateLaser.laserprofile.DurationCnt = 0;
	frameData_SolidStateLaser.laserprofile.Frequency = 10;
	frameData_SolidStateLaser.laserprofile.EnergyCnt = 0;
	frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
	frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_SNUM];
	frameData_SolidStateLaser.mode = 0;
	frameData_SolidStateLaser.state = 0;
	frameData_SolidStateLaser.connector = 0;
	frameData_SolidStateLaser.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
	frameData_SolidStateLaser.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);;
	
	// Reset button states
	frameData_SolidStateLaser.buttons.onInputBtn = 0;
	frameData_SolidStateLaser.buttons.onSimmerBtn = 0;
	frameData_SolidStateLaser.buttons.onStartBtn = 0;
	frameData_SolidStateLaser.buttons.onStopBtn = 0;
	
	WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}

void LongPulseLaserInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	// Reset session flushes
	SolidStateLaserPulseReset(slot1_id);
	
	//uint16_t frequency = frameData_SolidStateLaser.laserprofile.Frequency;
	uint16_t durationCnt = frameData_SolidStateLaser.laserprofile.DurationCnt;
	uint16_t energyCnt = frameData_SolidStateLaser.laserprofile.EnergyCnt;
	uint16_t connector = frameData_SolidStateLaser.connector;
	uint16_t mode = frameData_SolidStateLaser.mode;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_SolidStateLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata_ss(&frameData_SolidStateLaser, value);
	else 
		return;
	
	uint16_t state = frameData_SolidStateLaser.state;
	
	FlushesCount = 1000000;
	subFlushesCount = 1;
	
	if (frameData_SolidStateLaser.mode == 0)
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > VOLTAGES_SNUM-1) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = VOLTAGES_SNUM-1;
			update = true;
		}
	}
	else
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > VOLTAGES_NUM-1) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = VOLTAGES_NUM-1;
			update = true;
		}
	}
	
	switch (frameData_SolidStateLaser.mode)
	{
		case 0:
			if (frameData_SolidStateLaser.laserprofile.DurationCnt > SHORT_DURATIONS_NUM-1)
			{
				frameData_SolidStateLaser.laserprofile.DurationCnt = SHORT_DURATIONS_NUM-1;
				update = true;
			}
			break;
		case 1:
			if (frameData_SolidStateLaser.laserprofile.DurationCnt > STDRD_DURATIONS_NUM-1)
			{
				frameData_SolidStateLaser.laserprofile.DurationCnt = STDRD_DURATIONS_NUM-1;
				update = true;
			}
			break;
		case 2:
			if (frameData_SolidStateLaser.laserprofile.DurationCnt > LONGP_DURATIONS_NUM-1)
			{
				frameData_SolidStateLaser.laserprofile.DurationCnt = LONGP_DURATIONS_NUM-1;
				update = true;
			}
			break;
	}
	
	if (frameData_SolidStateLaser.mode == 0)
		frameData_SolidStateLaser.connector = 1;
	else
		frameData_SolidStateLaser.connector = 0;
	
	if (connector != frameData_SolidStateLaser.connector) update = true;
	
	if (frameData_SolidStateLaser.mode == 0) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 100)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 100;
			update = true;
		}
	}
	else
	if (frameData_SolidStateLaser.mode == 1) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 10)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 10;
			update = true;
		}
	}
	else
	if (frameData_SolidStateLaser.mode == 2) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 4)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 4;
			update = true;
		}
	}
	
	SetPulseFrequency_(frameData_SolidStateLaser.laserprofile.Frequency);
	frequency_publish = frameData_SolidStateLaser.laserprofile.Frequency * 0.1f;
	
	bool update_val = false;
	if (mode != frameData_SolidStateLaser.mode)		{ update = true; update_val = true; };
	switch (frameData_SolidStateLaser.mode)
	{
		case 0:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_SNUM];
		
			voltage = voltage_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_SNUM];
			//voltage_short[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
			
		case 1:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_stdrt[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*VOLTAGES_SNUM];
		
			//voltage = voltage_stdrt[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			voltage = voltage_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*VOLTAGES_SNUM];
			break;
			
		case 2:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_long[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*(VOLTAGES_NUM + VOLTAGES_SNUM)];
		
			//voltage = voltage_long[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			voltage = voltage_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*(VOLTAGES_NUM + VOLTAGES_SNUM)];
			break;
	}
	
	energy_publish = frameData_SolidStateLaser.lasersettings.Energy * 0.1f;
	
	if ((durationCnt != frameData_SolidStateLaser.laserprofile.DurationCnt) || update_val)
	{
			update = true;
	}
			
	if ((energyCnt != frameData_SolidStateLaser.laserprofile.EnergyCnt) || update_val)
	{
			update = true;
	}
	
	// ******************************************* Start working with module ************************************************
	
	_programI = ((float32_t)(voltage) / 450.0f) * 10.0f;
	
	if (frameData_SolidStateLaser.mode == 0)
	{
		SetPulseDuration_us(duration * 10);
		duration_publish = duration * 10.0f * 0.000001f;
	}
	else
	{
		SetPulseDuration_ms(duration, duration * 2);
		duration_publish = duration * 0.001f;
	}

	__SOLIDSTATELASER_DISCHARGEON();
	
	frameData_SolidStateLaser.state = 0;
	
	CoolOn();
	
	if (frameData_SolidStateLaser.buttons.onInputBtn != 0)
	{
		__SOLIDSTATELASER_DISCHARGEOFF();
		__SOLIDSTATELASER_HVON();
		
		frameData_SolidStateLaser.state = 1; // Battery charging
		
#ifdef DEBUG_SOLID_STATE_LASER
		if (__MISC_GETCHARGEMODULEREADYSTATE())
#endif
		{
			frameData_SolidStateLaser.state = 3; // Ready
			
			// On Input Pressed
			frameData_SolidStateLaser.buttons.onInputBtn = 0;
		
			new_pic_id = FRAME_PICID_LONGPULSE_START;
		
			SetDACValue(_programI);
		}
#ifdef DEBUG_SOLID_STATE_LASER
		else
		{
			frameData_SolidStateLaser.mode = mode;
		}
#endif
		update = true;
	}
	
	if (frameData_SolidStateLaser.buttons.onCancelBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onCancelBtn = 0;
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		//__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		frameData_SolidStateLaser.state = 0;
		new_pic_id = FRAME_PICID_LONGPULSE_INPUT;
		update = true;
		StoreGlobalVariables();
	}
	
	static int16_t simmer_off_cnt = 0;
#ifdef DEBUG_SOLID_STATE_LASER
	if (!(__MISC_GETSIMMERSENSOR()))
	{
		simmer_off_cnt++;
		if (simmer_off_cnt > 10) simmer_off_cnt = 10;
	}
	else
	{
		simmer_off_cnt = 0;
	}
	if ((simmer_off_cnt > 5) && pic_id != 55 && pic_id != 53) 
	{
		new_pic_id = FRAME_PICID_LONGPULSE_SIMMERSTART;
		
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
		
		update = true;
	}
#endif
	
	if (state != frameData_SolidStateLaser.state)
		update = true;
	
	if (frameData_SolidStateLaser.PulseCounter != GetSolidStateGlobalPulse(slot1_id))
	{
		frameData_SolidStateLaser.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
		frameData_SolidStateLaser.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);;
		update = true;
	}
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
