#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"

#include <math.h>
#include "arm_math.h"
//C27
														
uint16_t modeDurationTable[10] = {200, 200, 200, 200, 300, 300, 400, 500, 500, 500};
uint16_t modeVoltageTable[10] = {410, 415, 420, 430, 415, 435, 420, 415, 425, 440};
//uint16_t modeEnergyTable[10] = {180, 230, 270, 520, 715, 910, 1100, 1267, 1430, 1625}; //true values

//uint16_t modeEnergyTable[10] = {180, 230, 270, 598, 822, 1046, 1265, 1457, 1644, 1868}; //updated
uint16_t modeEnergyTable[10] = {180, 230, 270, 598, 822, 1100, 1265, 1457, 1644, 1868};

extern void SetDACValue(float32_t value);
extern float SSVoltageTrim(float voltage, uint32_t counter);
														 
float32_t programI = 0.0f;
float32_t chargingVoltage = 0.0f;
														 
uint16_t SetLaserSettings(uint16_t energy_index)
{
	uint16_t index = energy_index;
	if (index > 9) index = 9;
	
	uint16_t energy = modeEnergyTable[index];
	uint16_t voltageClb;
	if (slot1_id == LASER_ID_SOLIDSTATE)
		voltageClb = (uint16_t)SSVoltageTrim((float)modeVoltageTable[index], FlushesGlobalSS);
	if (voltageClb >= 449)	voltageClb = 449;
	uint16_t duration = modeDurationTable[index];
	
	SetPulseDuration_us(duration);
	
	duration_publish = duration * 0.001;
	energy_publish = energy * 0.001;
	
	chargingVoltage = (float32_t)(voltageClb);
	programI = ((float32_t)(voltageClb) / 450.0f) * 10.0f;
	
	if (slot1_id == LASER_ID_SOLIDSTATE)
		return energy;
	
	if (slot1_id == LASER_ID_SOLIDSTATE2)
		return (energy * 25) / 10;
	
	return energy;
}

void SolidStateLaserInput_Init(uint16_t pic_id)
{
	uint16_t energy = SetLaserSettings(frameData_SolidStateLaser.laserprofile.EnergyCnt);
	frameData_SolidStateLaser.laserprofile.Frequency = 1;
	frameData_SolidStateLaser.laserprofile.EnergyCnt = 0;
	frameData_SolidStateLaser.lasersettings.EnergyInt = energy / 1000;
	frameData_SolidStateLaser.lasersettings.Energy = (energy / 10) % 100;
	frameData_SolidStateLaser.mode = 0;
	frameData_SolidStateLaser.state = 0;
	frameData_SolidStateLaser.connector = 0;
	frameData_SolidStateLaser.PulseCounter = GetSolidStateGlobalPulse(slot1_id);//FlushesGlobalSS;
	frameData_SolidStateLaser.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);//FlushesSessionSS;
	
	// Reset button states
	frameData_SolidStateLaser.buttons.onInputBtn = 0;
	frameData_SolidStateLaser.buttons.onSimmerBtn = 0;
	frameData_SolidStateLaser.buttons.onStartBtn = 0;
	frameData_SolidStateLaser.buttons.onStopBtn = 0;
	
	WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}

void SolidStateLaserInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	// Reset session flushes
	SolidStateLaserPulseReset(slot1_id);
	
	//uint16_t frequency = frameData_SolidStateLaser.laserprofile.Frequency;
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
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > 2) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = 2;
			update = true;
		}
	}
	else
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt < 3) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = 3;
			update = true;
		}
	}
	
	if (mode != frameData_SolidStateLaser.mode)
	{
		if (frameData_SolidStateLaser.mode == 1)
		{
			if (frameData_SolidStateLaser.laserprofile.EnergyCnt > 6)
			{
				frameData_SolidStateLaser.laserprofile.EnergyCnt = 6;
				update = true;
			}
		}
		if (frameData_SolidStateLaser.mode == 2)
		{
			if (frameData_SolidStateLaser.laserprofile.EnergyCnt < 7)
			{
				frameData_SolidStateLaser.laserprofile.EnergyCnt = 7;
				update = true;
			}
		}
		
		update = true;
	}
	
	if (/*(frameData_SolidStateLaser.laserprofile.EnergyCnt >= 0) && */(frameData_SolidStateLaser.laserprofile.EnergyCnt <= 2))
		frameData_SolidStateLaser.mode = 0;		
	
	if ((frameData_SolidStateLaser.laserprofile.EnergyCnt >= 3) && (frameData_SolidStateLaser.laserprofile.EnergyCnt <= 6))
		frameData_SolidStateLaser.mode = 1;
	
	if ((frameData_SolidStateLaser.laserprofile.EnergyCnt >= 7) && (frameData_SolidStateLaser.laserprofile.EnergyCnt <= 9))
		frameData_SolidStateLaser.mode = 2;
	
	if (mode != frameData_SolidStateLaser.mode)		update = true;
	if (connector != frameData_SolidStateLaser.connector) update = true;
	
	uint16_t energy = SetLaserSettings(frameData_SolidStateLaser.laserprofile.EnergyCnt);
	
	if (frameData_SolidStateLaser.connector == 1)	energy = (uint16_t)((float32_t)(energy) * 0.35f);
	
	if (frameData_SolidStateLaser.mode == 2) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 6)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 6;
			update = true;
		}
	}
	else
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 10)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 10;
			update = true;
		}
	}
	
	SetPulseFrequency(frameData_SolidStateLaser.laserprofile.Frequency);
	frequency_publish = frameData_SolidStateLaser.laserprofile.Frequency;
	
	//if (frameData_SolidStateLaser.mode != 0) energy = (uint16_t)((float32_t)(energy) * 1.15f);
	
	frameData_SolidStateLaser.lasersettings.EnergyInt = energy / 1000;
	frameData_SolidStateLaser.lasersettings.Energy = (energy / 10) % 100;
	
	if (energyCnt != frameData_SolidStateLaser.laserprofile.EnergyCnt)
	{
		frameData_SolidStateLaser.lasersettings.EnergyInt = energy / 1000;
		frameData_SolidStateLaser.lasersettings.Energy = (energy / 10) % 100;
		update = true;
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
		
			new_pic_id = FRAME_PICID_SOLIDSTATE_SIMMERSTART;
		
			SetDACValue(programI);
		}
		update = true;
	}
	
	if (frameData_SolidStateLaser.buttons.onCancelBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onCancelBtn = 0;
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		frameData_SolidStateLaser.state = 0;
		new_pic_id = FRAME_PICID_SOLIDSTATE_INPUT;
		update = true;
		StoreGlobalVariables();
	}
	
	if (state != frameData_SolidStateLaser.state)
		update = true;
	
	if (frameData_SolidStateLaser.PulseCounter != GetSolidStateGlobalPulse(slot1_id))
	{
		frameData_SolidStateLaser.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
		frameData_SolidStateLaser.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);
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
