#ifndef __LASERDIODEGUIPRESET_H
#define __LASERDIODEGUIPRESET_H

#include <stdbool.h>
#include <stdint.h>
#include "DGUS.h"
#include "GlobalVariables.h"
#include "stm32f4xx_hal.h"

typedef struct GUI_PRESET_STRUCT
{
	// Limits
	uint16_t m_wMaxEnergy;
	uint16_t m_wMinEnergy;
	uint16_t m_wMaxDuration;
	uint16_t m_wMinDuration;
	uint16_t m_wMaxFreq;
	uint16_t m_wMinFreq;

	// Helper control
	int16_t m_wEnergyStep;
	int16_t m_wEnergyNumSteps;
	uint16_t m_wDurationStep;
	uint16_t m_wDurationNumSteps;	
	
	// Helper status
	bool updateDuration;
	bool updateEnergy;
} GUI_PRESET;

typedef enum APP_PROFILE_ENUM
{
	PROFILE_FAST		= 3,
	PROFILE_MEDIUM	= 2,
	PROFILE_SLOW		= 1,
	PROFILE_SINGLE	= 0,
} APP_PROFILE, *PAPP_PROFILE;

// Laser Diode Global Variables
extern APP_PROFILE Profile;
extern GUI_PRESET pstGUI[5];
extern volatile DGUS_LASERPROFILE	m_structLaserProfile [5];
extern volatile DGUS_LASERSETTINGS	m_structLaserSettings[5];

void NormalizeStep(uint16_t *min, uint16_t *max, uint16_t *step, uint16_t threshold_numsteps, uint16_t step_tbl[]);

// Old laser code for laser diode
	// Work with all limits
void UpdateLimits(uint16_t  freq, uint16_t  duration, uint16_t  energy, APP_PROFILE mode);
bool CheckLimits (uint16_t *freq, uint16_t *duration, uint16_t *energy, APP_PROFILE mode);
	// Calculate steps
void CalculateDurationSteps(uint16_t *freq, uint16_t *duration, APP_PROFILE Profile);
void CalculateEnergySteps  (uint16_t *freq, uint16_t *energy,   APP_PROFILE Profile);
void CalculateAllSteps     (uint16_t *freq, uint16_t *duration, APP_PROFILE mode);
	// Extended modes
bool FreqLimits(uint16_t *freq, APP_PROFILE mode);
bool CheckLimitsFastMode(uint16_t *freq, uint16_t *duration, uint16_t *energy);

void LaserPreset(uint16_t *freq, uint16_t *duration, uint16_t *energy, APP_PROFILE mode);

void MelaninPreset(uint16_t melanin);
void PhototypePreset(uint16_t phototype);

#endif
