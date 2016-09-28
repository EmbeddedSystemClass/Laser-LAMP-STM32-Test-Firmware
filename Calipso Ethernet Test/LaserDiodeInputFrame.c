#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h" 
#include "LaserMisc.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void LaserDiodeInput_Init(uint16_t pic_id)
{
	uint16_t freq = 10;
	uint16_t duration = 37;
	uint16_t energy = 13;
	
	// Old presets. Now read from flash !!!
	// Fast profile
	LaserPreset(&freq, &duration, &energy, PROFILE_FAST);
	// Medium profile
	freq = 3; duration = 80; energy = 26;
	LaserPreset(&freq, &duration, &energy, PROFILE_MEDIUM);
	// Slow profile
	freq = 2; duration = 120; energy = 36;
	LaserPreset(&freq, &duration, &energy, PROFILE_SLOW);
	// Single profile
	freq = 1; duration = 100; energy = 32;
	LaserPreset(&freq, &duration, &energy, PROFILE_SINGLE);
	
	// Current profile
	Profile = PROFILE_FAST;
	
	// Set all laser settings
	//laserCounter = eeprom_read_dword((uint32_t*)LASER_CNT_EEPROMADDR);
	frameData_LaserDiode.mode = Profile;
	memcpy((void*)&frameData_LaserDiode.laserprofile, (void*)&m_structLaserProfile[PROFILE_FAST], sizeof(DGUS_LASERPROFILE));
	memcpy((void*)&frameData_LaserDiode.lasersettings, (void*)&m_structLaserSettings[PROFILE_FAST], sizeof(DGUS_LASERSETTINGS));
	frameData_LaserDiode.timer.timer_minutes = m_wSetMin;
	frameData_LaserDiode.timer.timer_seconds = m_wSetSec;
	frameData_LaserDiode.PulseCounter = FlushesGlobalLD;
	frameData_LaserDiode.melanin = 0;
	frameData_LaserDiode.phototype = 1;
	frameData_LaserDiode.temperature = temperature;
	frameData_LaserDiode.cooling = 3;
	frameData_LaserDiode.flow = 0;
	//frameData_LaserDiode.DatabasePageOffset = 0;
	//frameData_LaserDiode.DatabaseSelectionIndex = 13;
	frameData_LaserDiode.SessionPulseCounter = 0;
	
	// Old presets. Now read from flash !!!
	// Preset hardware to FAST mode
	freq = 10;
	duration = 40;
	energy = 14;
	LaserPreset(&freq, &duration, &energy, PROFILE_FAST);
	
	WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}

void LaserDiodeInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	float32_t power = 0.0f;
	
	// Reset session flushes
	FlushesSessionLD = 0;
	
	// Old code ***************************************************************************************** >
	uint16_t melanin      = frameData_LaserDiode.melanin;
	uint16_t phototype    = frameData_LaserDiode.phototype;
	uint16_t freq         = frameData_LaserDiode.laserprofile.Frequency;
	uint16_t durationCnt  = frameData_LaserDiode.laserprofile.DurationCnt;
	uint16_t energyCnt    = frameData_LaserDiode.laserprofile.EnergyCnt;
	uint16_t flushesLimit	= frameData_LaserDiode.lasersettings.FlushesLimit;
	// Old code ***************************************************************************************** >
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(frameData_LaserDiode));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	// Old code ***************************************************************************************** >
	uint16_t duration;
	uint16_t energy;
	if (flushesLimit != frameData_LaserDiode.lasersettings.FlushesLimit)
	{	
		if ((frameData_LaserDiode.lasersettings.FlushesLimit != 4) && (Profile != PROFILE_FAST))
		{
			frameData_LaserDiode.lasersettings.FlushesLimit = 4;
			update = true;
			goto update;
		}
		
		if (Profile == PROFILE_FAST)
		{
			if (frameData_LaserDiode.lasersettings.FlushesLimit == 3) frameData_LaserDiode.lasersettings.FlushesLimit = 0;
			update = true;
			goto update;
		}
	}
	
	if (Profile == PROFILE_FAST)
	{
		uint16_t laserLimitMode = frameData_LaserDiode.lasersettings.FlushesLimit;
		switch (laserLimitMode)
		{
			case 0:
				FlushesCount = 300;
				break;
			case 1:
				FlushesCount = 400;
				break;
			case 2:
				FlushesCount = 500;
				break;
		}
	}
	else
		FlushesCount = 1000000;
	
	if (phototype != frameData_LaserDiode.phototype)
	{
		PhototypePreset(frameData_LaserDiode.phototype);
		memcpy((void*)&frameData_LaserDiode.laserprofile , (void*)&m_structLaserProfile [Profile], sizeof(frameData_LaserDiode.laserprofile));
		memcpy((void*)&frameData_LaserDiode.lasersettings, (void*)&m_structLaserSettings[Profile], sizeof(frameData_LaserDiode.lasersettings));
		update = true;
		goto update;
	}
	else
	if (melanin != frameData_LaserDiode.melanin)
	{
		MelaninPreset(frameData_LaserDiode.melanin);
		memcpy((void*)&frameData_LaserDiode.laserprofile , (void*)&m_structLaserProfile [Profile], sizeof(frameData_LaserDiode.laserprofile));
		memcpy((void*)&frameData_LaserDiode.lasersettings, (void*)&m_structLaserSettings[Profile], sizeof(frameData_LaserDiode.lasersettings));
		update = true;
		goto update;
	}
	else
	if (Profile != (APP_PROFILE)frameData_LaserDiode.mode)
	{
		Profile = (APP_PROFILE)frameData_LaserDiode.mode;
		// Update profile
		memcpy((void*)&frameData_LaserDiode.laserprofile , (void*)&m_structLaserProfile [Profile], sizeof(frameData_LaserDiode.laserprofile));
		memcpy((void*)&frameData_LaserDiode.lasersettings, (void*)&m_structLaserSettings[Profile], sizeof(frameData_LaserDiode.lasersettings));
		
		update = true;
		goto update;
	}
	
	duration = durationCnt * pstGUI[Profile].m_wDurationStep;
	energy   = energyCnt   * pstGUI[Profile].m_wEnergyStep;
	
	if (freq != frameData_LaserDiode.laserprofile.Frequency)
	{
		freq = frameData_LaserDiode.laserprofile.Frequency;
		
		update = true;
	}
	
	UpdateLimits(freq, duration, energy, Profile);
	
	if (durationCnt != frameData_LaserDiode.laserprofile.DurationCnt)
	{					
		if (Profile == PROFILE_FAST)
			frameData_LaserDiode.laserprofile.DurationCnt = durationCnt; // Disable duration when FAST mode
		else
		{
			duration = frameData_LaserDiode.laserprofile.DurationCnt * pstGUI[Profile].m_wDurationStep;
			
			// Check limit
			if ((duration <= pstGUI[Profile].m_wMaxDuration) && (duration >= pstGUI[Profile].m_wMinDuration))
			{
				if (pstGUI[Profile].updateDuration)
					CalculateDurationSteps(&freq, &duration, Profile);
				frameData_LaserDiode.lasersettings.Duration = duration;
			}
			else
				frameData_LaserDiode.laserprofile.DurationCnt = durationCnt; // Cancel change if out
		}
		update = true;
	}
	if (energyCnt != frameData_LaserDiode.laserprofile.EnergyCnt)
	{
		energy = frameData_LaserDiode.laserprofile.EnergyCnt * pstGUI[Profile].m_wEnergyStep;
		
		if (Profile == PROFILE_FAST)
			CheckLimitsFastMode(&freq, &duration, &energy);
		else
		{
			// Check limit
			if ((energy <= pstGUI[Profile].m_wMaxEnergy) && (energy >= pstGUI[Profile].m_wMinEnergy))
			{
				if (pstGUI[Profile].updateEnergy)
					CalculateEnergySteps(&freq, &energy, Profile);
				frameData_LaserDiode.lasersettings.Energy = energy;
			}
			else
				frameData_LaserDiode.laserprofile.EnergyCnt = energyCnt; // Cancel change if out
		}
		update = true;
	}
	
	if (Profile != PROFILE_FAST)
	{
		if (CheckLimits(&freq, &duration, &energy, Profile))
			update = true;
	}
	else
		CheckLimitsFastMode(&freq, &duration, &energy);
	
	// Copy data to profiles
	if (update)
	{
		memcpy((void*)&m_structLaserProfile [Profile], (void*)&frameData_LaserDiode.laserprofile , sizeof(frameData_LaserDiode.laserprofile));
		memcpy((void*)&m_structLaserSettings[Profile], (void*)&frameData_LaserDiode.lasersettings, sizeof(frameData_LaserDiode.lasersettings));
	}	
	// Old code ***************************************************************************************** >
	
	// Set laser settings
	//DiodeControlPulseStop();
	if (Profile == PROFILE_MEDIUM)
	{
		if (freq > 2)
			SetPulseDuration_ms(duration/2, duration/2 + 15);
		else
			SetPulseDuration_ms(duration/2, duration/2 + 20);
		subFlushesCount = 2;
	}
	else
	{
		SetPulseDuration_ms(duration, duration * 2);
		subFlushesCount = 1;
	}
	
	SetPulseFrequency(freq);
	power = ((float32_t)energy * 1440.0f) / (float32_t)(duration);
	if (power > 500.0f) power = 500.0f;
	SetDACValue(power * 10.0f / 500.0f);
	
	// State - input data
	if (frameData_LaserDiode.state != 0)
	{
		frameData_LaserDiode.state = 0;
		update = true;
	}
	
	// Control peltier cooling
	CoolOn();
	CoolSet((frameData_LaserDiode.cooling + 1) * 17);
		
	if (frameData_LaserDiode.buttons.onInputBtn != 0)
	{
		// On Input Pressed
		frameData_LaserDiode.buttons.onInputBtn = 0;
		
		prepare = true; // Start prepare timer
		update = true;
		
		new_pic_id = FRAME_PICID_LASERDIODE_PREPARETIMER;
	}
	
update:
	if (frameData_LaserDiode.PulseCounter != FlushesGlobalLD)
	{
		frameData_LaserDiode.PulseCounter = FlushesGlobalLD;
		frameData_LaserDiode.SessionPulseCounter = FlushesSessionLD;
		update = true;
	}
	
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
