#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"

#include <math.h>
#include "arm_math.h"
														
uint16_t modeFractDurationTable[30] = { 500,  500,  500,  500,  500,  500,  500,  500,  500,  500,
																			 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
																			 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000};
uint16_t modeFractVoltageTable [30] = { 260,  280,  300,  320,  340,  360,  380,  400,  420,  440,
																				260,  280,  300,  320,  340,  360,  380,  400,  420,  440,
																				260,  280,  300,  320,  340,  360,  380,  400,  420,  440};
uint16_t modeFractEnergyTable  [30] = { 120,	201,	504,	637,	920, 1243, 1590, 2138, 2325, 2707,
																				512,	965, 1474, 2053, 2661, 3392, 4048, 4800, 5392, 6160,
																			 1088, 1923, 2387, 3920, 4464, 5568, 6672, 7792, 8896, 9984};

extern void SetDACValue(float32_t value);
														 
float32_t fract_programI = 0.0f;
float32_t fract_chargingVoltage = 0.0f;
														 
uint16_t SetLaserSettingsFract(uint16_t energy_index, uint16_t mode)
{
	uint16_t index = energy_index;
	if (index > 9) index = 9;
	
	uint16_t energy = modeFractEnergyTable[index + mode * 10];
	uint16_t voltageClb = modeFractVoltageTable[index + mode * 10];// + (FlushesGlobalSS / 100000);
	if (voltageClb >= 449)	voltageClb = 449;
	uint16_t duration = modeFractDurationTable[index + mode * 10];
	
	duration_publish = duration * 0.001;
	energy_publish = energy * 0.001;
	
	SetPulseDuration_us(duration);
	
	fract_chargingVoltage = (float32_t)(voltageClb);
	fract_programI = ((float32_t)(voltageClb) / 450.0f) * 10.0f;
	
	return energy;
}

void FractLaserInput_Init(uint16_t pic_id)
{
	uint16_t energy = SetLaserSettingsFract(0, 0);
	frameData_FractLaser.laserprofile.Frequency = 1;
	frameData_FractLaser.laserprofile.EnergyCnt = 0;
	frameData_FractLaser.lasersettings.EnergyInt = energy / 1000;
	frameData_FractLaser.lasersettings.Energy = energy % 1000 + 1000;
	frameData_FractLaser.mode = 0;
	frameData_FractLaser.state = 0;
	frameData_FractLaser.connector = 0;
	frameData_FractLaser.PulseCounter = GetSolidStateGlobalPulse(LaserID);//FlushesGlobalSS;
	frameData_FractLaser.SessionPulseCounter = GetSolidStateSessionPulse(LaserID);//FlushesSessionSS;
	
	// Reset button states
	frameData_FractLaser.buttons.onInputBtn = 0;
	frameData_FractLaser.buttons.onSimmerBtn = 0;
	frameData_FractLaser.buttons.onStartBtn = 0;
	frameData_FractLaser.buttons.onStopBtn = 0;
	
	WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_FractLaser);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}

void FractLaserInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	// Reset session flushes
	SolidStateLaserPulseReset(LaserID);
	
	//uint16_t frequency = frameData_SolidStateLaser.laserprofile.Frequency;
	uint16_t energyCnt = frameData_FractLaser.laserprofile.EnergyCnt;
	uint16_t mode = frameData_FractLaser.mode;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_FractLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata_ss(&frameData_FractLaser, value);
	else 
		return;
	
	uint16_t state = frameData_FractLaser.state;
	
	FlushesCount = 1000000;
	subFlushesCount = 1;
	
	if (mode != frameData_FractLaser.mode)		update = true;
	
	uint16_t energy = SetLaserSettingsFract(frameData_FractLaser.laserprofile.EnergyCnt, frameData_FractLaser.mode);
	
	if (frameData_FractLaser.mode == 0) 
	{
		if (frameData_FractLaser.laserprofile.Frequency > config_FractLaser.mode_freq_max[0])
		{
			frameData_FractLaser.laserprofile.Frequency = config_FractLaser.mode_freq_max[0];
			update = true;
		}
	}
	
	if (frameData_FractLaser.mode == 1) 
	{
		if (frameData_FractLaser.laserprofile.Frequency > config_FractLaser.mode_freq_max[1])
		{
			frameData_FractLaser.laserprofile.Frequency = config_FractLaser.mode_freq_max[1];
			update = true;
		}
	}
	
	if (frameData_FractLaser.mode == 2) 
	{
		if (frameData_FractLaser.laserprofile.Frequency > config_FractLaser.mode_freq_max[2])
		{
			frameData_FractLaser.laserprofile.Frequency = config_FractLaser.mode_freq_max[2];
			update = true;
		}
	}
	
	SetPulseFrequency(frameData_FractLaser.laserprofile.Frequency);
	frequency_publish = frameData_FractLaser.laserprofile.Frequency;
	
	//if (frameData_SolidStateLaser.mode != 0) energy = (uint16_t)((float32_t)(energy) * 1.15f);
	
	frameData_FractLaser.lasersettings.EnergyInt = energy / 1000;
	frameData_FractLaser.lasersettings.Energy = energy % 1000 + 1000;
	
	if (energyCnt != frameData_FractLaser.laserprofile.EnergyCnt)
	{
		//frameData_FractLaser.lasersettings.EnergyInt = energy / 1000;
		//frameData_FractLaser.lasersettings.Energy = (energy / 10) % 100;
		update = true;
	}

	__SOLIDSTATELASER_DISCHARGEON();
	
	frameData_FractLaser.state = 0;
	
	CoolOn();
	
	if (frameData_FractLaser.buttons.onInputBtn != 0)
	{
		__SOLIDSTATELASER_DISCHARGEOFF();
		__SOLIDSTATELASER_HVON();
		
		frameData_FractLaser.state = 1; // Battery charging
		
#ifdef DEBUG_SOLID_STATE_LASER
		if (__MISC_GETCHARGEMODULEREADYSTATE())
#endif
		{
			frameData_FractLaser.state = 3; // Ready
			
			// On Input Pressed
			frameData_FractLaser.buttons.onInputBtn = 0;
		
			new_pic_id = FRAME_PICID_FRACTLASER_SIMMERSTART;
		
			SetDACValue(fract_programI);
		}
		update = true;
	}
	
	if (frameData_FractLaser.buttons.onCancelBtn != 0)
	{
		// On Input Pressed
		frameData_FractLaser.buttons.onCancelBtn = 0;
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		frameData_FractLaser.state = 0;
		new_pic_id = FRAME_PICID_FRACTLASER_INPUT;
		update = true;
		StoreGlobalVariables();
	}
	
	if (state != frameData_FractLaser.state)
		update = true;
	
	if (frameData_FractLaser.PulseCounter != GetSolidStateGlobalPulse(LaserID))
	{
		frameData_FractLaser.PulseCounter = GetSolidStateGlobalPulse(LaserID);
		frameData_FractLaser.SessionPulseCounter = GetSolidStateSessionPulse(LaserID);
		update = true;
	}
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_FractLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
