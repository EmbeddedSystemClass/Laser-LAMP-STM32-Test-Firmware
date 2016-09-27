#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"

#include <math.h>
#include "arm_math.h"

uint16_t durationTable[5] = {200, 300, 400, 500};
//uint16_t voltageTable[5] = {410 , 420 , 430 , 440 , 450 };
uint16_t modeTable[3] = {0, 1, 3};

uint16_t energyTable[25] = {230 , 235 , 240 , 420 , 425 ,
														460 , 670 , 680 , 900 , 920 ,
														850 , 900 , 1100, 1144, 1235,
														1300, 1325, 1350, 1600, 1700}; 

uint16_t voltageTableClb[25] = 
														{410, 420, 430, 440, 450,
														 410, 420, 430, 440, 450,
														 410, 420, 430, 440, 450,
														 410, 420, 430, 440, 450}; 

extern void SetDACValue(float32_t value);
														 
float32_t programI = 0.0f;
float32_t chargingVoltage = 0.0f;
														 
uint16_t SetLaserSettings(uint16_t mode, uint16_t energy_index)
{
	uint16_t index = energy_index;
	if (index > 4) index = 4;
	
	uint16_t energy = energyTable[modeTable[mode] * 5 + index];
	uint16_t voltageClb = voltageTableClb[modeTable[mode] * 5 + index];
	uint16_t duration = durationTable[modeTable[mode]];
	
	SetPulseDuration_us(duration);
	
	chargingVoltage = (float32_t)(voltageClb);
	programI = ((float32_t)(voltageClb) / 450.0f) * 10.0f;
	
	return energy;
}

void SolidStateLaserInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	uint16_t energyCnt = frameData_SolidStateLaser.laserprofile.EnergyCnt;
	uint16_t mode = frameData_SolidStateLaser.mode;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_SolidStateLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata_ss(&frameData_SolidStateLaser, value);
	else 
		return;
	
	uint16_t state = frameData_SolidStateLaser.state;
	
	uint16_t energy = SetLaserSettings(frameData_SolidStateLaser.mode, frameData_SolidStateLaser.laserprofile.EnergyCnt);
	SetPulseFrequency(frameData_SolidStateLaser.laserprofile.Frequency);
	
	FlushesCount = 1000000;
	subFlushesCount = 1;
	
	if (mode != frameData_SolidStateLaser.mode)
	{
		frameData_SolidStateLaser.lasersettings.EnergyInt = energy / 1000;
		frameData_SolidStateLaser.lasersettings.Energy = (energy / 10) % 100;
		update = true;
	}
	
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
			frameData_SolidStateLaser.state = 1; // Ready
			
			// On Input Pressed
			frameData_SolidStateLaser.buttons.onInputBtn = 0;
		
			new_pic_id = FRAME_PICID_SOLIDSTATE_SIMMERSTART;
		
			SetDACValue(programI);
		}
		update = true;
	}
	
	if (state != frameData_SolidStateLaser.state)
		update = true;
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
