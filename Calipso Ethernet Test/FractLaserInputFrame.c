#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"
#include "Erb_Energy_Table.h"
#include "Fract_Energy_Table.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);
														 
float32_t fract_programI = 0.0f;
float32_t fract_chargingVoltage = 0.0f;
														 
uint16_t SetLaserSettingsFract(uint16_t energy_index, uint16_t mode)
{
	uint16_t index = energy_index;
	uint16_t energy = 0;
	uint16_t voltageClb = 0;
	uint16_t duration = 0;
	
	if (slot1_id == LASER_ID_FRACTLASER)
	{
		if (index > (FRACT1440NM_NUM_ENERGY-1)) index = FRACT1440NM_NUM_ENERGY-1;
		
		energy = 0.6 * modeFract1440nmEnergyTable[index + mode * FRACT1440NM_NUM_ENERGY];
		voltageClb = modeFract1440nmVoltageTable[index + mode * FRACT1440NM_NUM_ENERGY];// + (FlushesGlobalSS / 100000);
		if (voltageClb >= 449)	voltageClb = 449;
		duration = modeFract1440nmDurationTable[index + mode * FRACT1440NM_NUM_ENERGY];
	}
	if (slot1_id == LASER_ID_1340NM)
	{
		if (index > (FRACT1340NM_NUM_ENERGY-1)) index = FRACT1340NM_NUM_ENERGY-1;
		
		energy = modeFract1340nmEnergyTable[index + mode * FRACT1340NM_NUM_ENERGY];
		voltageClb = modeFract1340nmVoltageTable[index + mode * FRACT1340NM_NUM_ENERGY];// + (FlushesGlobalSS / 100000);
		if (voltageClb >= 449)	voltageClb = 449;
		duration = modeFract1440nmDurationTable[index + mode * FRACT1340NM_NUM_ENERGY];
	}
	
	if (slot1_id == LASER_ID_2940NM)
	{
		if (index > (ERB_VOLTAGES_NUM-1)) index = ERB_VOLTAGES_NUM-1;
		
		energy = global_Erb_Energy_Table[index + mode * ERB_VOLTAGES_NUM];
		voltageClb = global_Erb_Voltage_Table[index + mode * ERB_VOLTAGES_NUM];// + (FlushesGlobalSS / 100000);
		if (voltageClb >= 449)	voltageClb = 449;
		duration = global_Erb_Duration_Table[index + mode * ERB_VOLTAGES_NUM];
	}
	
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
	frameData_FractLaser.PulseCounter = GetSolidStateGlobalPulse(slot1_id);//FlushesGlobalSS;
	frameData_FractLaser.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);//FlushesSessionSS;
	
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
	SolidStateLaserPulseReset(slot1_id);
	
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
		if ((slot1_id == LASER_ID_FRACTLASER) && (frameData_FractLaser.laserprofile.EnergyCnt > (FRACT1440NM_NUM_ENERGY-1))) 
			frameData_FractLaser.laserprofile.EnergyCnt = FRACT1440NM_NUM_ENERGY-1;
		if ((slot1_id == LASER_ID_1340NM) && (frameData_FractLaser.laserprofile.EnergyCnt > (FRACT1340NM_NUM_ENERGY-1))) 
			frameData_FractLaser.laserprofile.EnergyCnt = FRACT1340NM_NUM_ENERGY-1;
		if ((slot1_id == LASER_ID_2940NM) && (frameData_FractLaser.laserprofile.EnergyCnt > (ERB_VOLTAGES_NUM-1))) 
			frameData_FractLaser.laserprofile.EnergyCnt = ERB_VOLTAGES_NUM-1;
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
	
	if (frameData_FractLaser.PulseCounter != GetSolidStateGlobalPulse(slot1_id))
	{
		frameData_FractLaser.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
		frameData_FractLaser.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);
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
