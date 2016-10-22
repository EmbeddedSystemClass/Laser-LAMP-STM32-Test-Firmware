#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

float32_t _programI;

uint16_t duration;
uint16_t voltage;

uint16_t duration_short[6] = {20, 40, 60, 80, 100, 120};
uint16_t duration_stdrt[6] = {5, 10, 15, 20, 25, 30};
uint16_t duration_long[6]	=	{40, 50, 60, 70, 80, 90};

/*uint16_t energy_short[5] = {1,	2,	3,	4,	5};
uint16_t energy_stdrt[5] = {10,	15,	20,	25,	30};
uint16_t energy_long [5] = {50,	60,	70,	80,	90};*/

uint16_t energy_tbl[5*18] = 
	{	1,	17, 34, 51,	68,
		12,	14, 16,	18,	20,
		22,	24,	25,	27,	28,
		25,	28,	30,	33,	35,
		31,	35,	38,	42,	45,
		40,	43,	45,	48,	50,
		
		210,	225,	240,	255,	260,
		350,	375,	400, 	425,	450,
		470,	485,	500,	510,	520,
		500,	525,	550,	875,	600,
		550,	575,	600,	625,	650,
		600,	620, 	640, 	660,	680,
		
		600,	630,	660,	690,	720,
		700,	740,	780,	815,	850,
		700,	745,	790,	830,	870,
		700,	750,	800,	850,	900,
		700,	750,	800,	850,	900,
		700,	750,	800,	850,	900};
		

uint16_t voltage_short[5] = {380, 390, 400, 410, 420};
uint16_t voltage_other[5] = {360, 370, 380, 390, 400};

void LongPulseLaserInput_Init(uint16_t pic_id)
{
	frameData_SolidStateLaser.laserprofile.Frequency = 10;
	frameData_SolidStateLaser.laserprofile.EnergyCnt = 0;
	frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
	frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*5];
	frameData_SolidStateLaser.mode = 0;
	frameData_SolidStateLaser.state = 0;
	frameData_SolidStateLaser.connector = 0;
	frameData_SolidStateLaser.PulseCounter = 0;
	frameData_SolidStateLaser.SessionPulseCounter = 0;
	
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
	FlushesSessionSS = 0;
	
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
	
	/*if (frameData_SolidStateLaser.mode == 0)
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > 4) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = 4;
			update = true;
		}
	}
	else
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > 7) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = 7;
			update = true;
		}
	}*/
	
	if (frameData_SolidStateLaser.laserprofile.EnergyCnt > 4) 
	{
		frameData_SolidStateLaser.laserprofile.EnergyCnt = 4;
		update = true;
	}
	
	if (frameData_SolidStateLaser.laserprofile.DurationCnt > 5)
	{
		frameData_SolidStateLaser.laserprofile.DurationCnt = 5;
		update = true;
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
	
	//if (frameData_SolidStateLaser.mode != 0) energy = (uint16_t)((float32_t)(energy) * 1.15f);
	
	/*frameData_SolidStateLaser.lasersettings.EnergyInt = energy / 1000;
	frameData_SolidStateLaser.lasersettings.Energy = (energy / 10) % 100;*/
	
	bool update_val = false;
	if (mode != frameData_SolidStateLaser.mode)		{ update = true; update_val = true; };
	switch (frameData_SolidStateLaser.mode)
	{
		case 0:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*5];
		
			voltage = voltage_short[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
			
		case 1:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_stdrt[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*5 + 30];
		
			voltage = voltage_other[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
			
		case 2:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_long[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
		frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*5 + 60];
		
			voltage = voltage_other[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
	}
	
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
		SetPulseDuration_us(duration * 10);
	else
		SetPulseDuration_ms(duration, duration * 2);

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
	
#ifdef DEBUG_SOLID_STATE_LASER
	if (!(__MISC_GETSIMMERSENSOR()) && pic_id != 55 && pic_id != 53) 
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
	
	if (frameData_SolidStateLaser.PulseCounter != FlushesGlobalSS)
	{
		frameData_SolidStateLaser.PulseCounter = FlushesGlobalSS;
		frameData_SolidStateLaser.SessionPulseCounter = FlushesSessionSS;
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
