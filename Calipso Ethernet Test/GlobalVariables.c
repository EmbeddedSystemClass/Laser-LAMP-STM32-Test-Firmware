#include "GlobalVariables.h"
#include "stm32f4xx_hal_flash.h"
#include <string.h>

// Flash data
PFLASH_GLOBAL_DATA global_flash_data = (PFLASH_GLOBAL_DATA)FLASH_LASERDATA_BASE;

// DGUS Control variables
uint16_t g_wDGUSTimeout = 200;

// Timer global variables
int16_t m_wMillSec = 0;
int16_t m_wSeconds = 10;
int16_t m_wMinutes = 2;
int16_t m_wSetSec  = 10;
int16_t m_wSetMin  = 2;

// Cooling global variables
float32_t temperature_cool_on = 26.5f;
float32_t temperature_cool_off = 25.0f;
float32_t temperature_overheat = 29.5f;
float32_t temperature_overheat_solidstate = 32.0f;
float32_t temperature_normal = 27.5f;

// Flow global variable
float32_t flow_low = 2.0f;
float32_t flow_normal = 4.0f;

// Service menu password
char password[6] = "78965\0";
char ip_addr[16] = "\0";
bool ip_addr_updated = false;

// Global State Variables
volatile float32_t temperature = 0;
volatile float32_t flow1 = 9.0f;
volatile float32_t flow2 = 9.0f;
volatile float32_t VoltageMonitor = 0.0f;
volatile float32_t CurrentMonitor = 0.0f;

// Laser ID
LASER_ID LaserID = LASER_ID_SOLIDSTATE;
MENU_ID MenuID = MENU_ID_SOLIDSTATE;

// Private variables
uint16_t pic_id = 0;
bool g_peltier_en = false;
bool prepare = false;   
bool RemoteControl = false;
bool LaserStarted = false;
bool SolidStateLaser_en = false;
bool DiodeLaser_en = false;
bool DiodeLaserOnePulse_en = false;
uint16_t subFlushes = 0;
uint16_t subFlushesCount = 1;
uint32_t Flushes = 0;
uint32_t FlushesCount = 1000000;
uint32_t FlushesSessionLD = 0;
uint32_t FlushesGlobalLD = 0;
uint32_t FlushesSessionSS = 0;
uint32_t FlushesGlobalSS = 0;
uint32_t FlushesSessionSS2 = 0;
uint32_t FlushesGlobalSS2 = 0;
uint32_t FlushesSessionLP = 0;
uint32_t FlushesGlobalLP = 0;
uint16_t switch_filter_threshold = 10;
volatile uint16_t switch_filter = 0;
volatile bool footswitch_en = false;
volatile bool footswitch_on = false;
DGUS_LASERDIODE frameData_LaserDiode;
DGUS_SOLIDSTATELASER frameData_SolidStateLaser;

uint16_t min(uint16_t x, uint16_t y) { return (x>y)?y:x; }
uint16_t max(uint16_t x, uint16_t y) { return (x>y)?x:y; }

// Old code macroses
#define PROFILE_SLOW_MIN_FREQ	1
#define PROFILE_SLOW_MAX_FREQ	6

#define PROFILE_MEDIUM_MIN_FREQ	1
#define PROFILE_MEDIUM_MAX_FREQ	3

#define PROFILE_FAST_MIN_FREQ	6
#define PROFILE_FAST_MAX_FREQ	10

// Laser Diode Configuration Tables
uint16_t MinDurationTable[11] = {60, 60,  20,  20, 20, 10, 10, 10, 10, 10, 10};
uint16_t MaxDurationTable[11] = {400, 400, 120, 120, 100, 80, 65, 57, 50, 44, 40};
uint16_t TableNum[11]         = {0, 11, 11, 11, 9, 8, 7, 7, 6, 5, 4};
uint16_t EnergyTable[110]     = {21,		32,		44,		54,		63,		72,		83,		87,		95,		103,	111, // Updated
																	7,		10,		14,		17,		20,		23,		26,		29,		31,		34,		37,
																	7,		10,		14,		17,		20,		24,		27,		29,		32,		34,		37,
																	7,		10,		13,		17,		20,		24,		26,		29,		32,		0,		0,
																	4,		7,		10,		14,		17,		20,		23,		26,		0,		0,		0,
																	4,		7,		10,		14,		17,		20,		23,		0,		0,		0,		0,
																	4,		7,		10,		14,		17,		20,		23,		0,		0,		0,		0,
																	4,		7,		10,		14,		17,		20,		0,		0,		0,		0,		0,
																	4,		7,		10,		14,		17,		0,		0,		0,		0,		0,		0,
																	4,		7,		10,		14,		0,		0,		0,		0,		0,		0,		0};
uint16_t step_table[10] = {1, 2, 5, 10, 15, 20, 50, 100, 150, 200};

// Laser Diode Global Variables
APP_PROFILE Profile;
GUI_PRESET pstGUI[5];
volatile DGUS_LASERPROFILE	m_structLaserProfile [5];
volatile DGUS_LASERSETTINGS	m_structLaserSettings[5];

void NormalizeStep(uint16_t *min, uint16_t *max, uint16_t *step, uint16_t threshold_numsteps, uint16_t step_tbl[])
{
	uint16_t offset = (*min / *step) * *step;
	uint16_t delta = *max - *min;
	uint16_t i = 0;
	
	*step = step_tbl[i++];
	while ((delta / *step) > threshold_numsteps)
	{
		*step = step_tbl[i++];
	}
		
	*min = offset;
}

// Old laser code for laser diode
void UpdateLimits(uint16_t freq, uint16_t duration, uint16_t energy, APP_PROFILE mode)
{
	bool UpdateFreq = false;
	switch (mode)
	{
		case PROFILE_SINGLE:		
			pstGUI[mode].m_wMinFreq = 1;
			pstGUI[mode].m_wMaxFreq = 1;
			UpdateFreq = true;
			break;
		case PROFILE_SLOW:			
			pstGUI[mode].m_wMinFreq = PROFILE_SLOW_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_SLOW_MAX_FREQ;
			break;
		case PROFILE_MEDIUM:		
			pstGUI[mode].m_wMinFreq = PROFILE_MEDIUM_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_MEDIUM_MAX_FREQ;
			break;
		case PROFILE_FAST:
			pstGUI[mode].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
			break;
	}
	if (mode != PROFILE_SINGLE)	
	{
		if (freq < pstGUI[mode].m_wMinFreq)	UpdateFreq = true;
		if (freq > pstGUI[mode].m_wMaxFreq)	UpdateFreq = true;
	}
	
	// Update Frequency
	if (UpdateFreq) frameData_LaserDiode.laserprofile.Frequency = freq;
	
	uint16_t MinD = MinDurationTable[freq];
	uint16_t MaxD = MaxDurationTable[freq];
	uint16_t Delta = (MaxD - MinD) / (TableNum[freq]-1);
	
	if (mode == PROFILE_MEDIUM)
	{
		MinD = 10;
		MaxD = 100;
	}
	
	// Update duration helpers
	if (duration < MinD)	duration = MinD;
	if (duration > MaxD)	duration = MaxD;
		
	if ((pstGUI[mode].m_wMinDuration != MinD) || (pstGUI[mode].m_wMaxDuration != MaxD))
	{
		pstGUI[mode].m_wMinDuration = MinD;
		pstGUI[mode].m_wMaxDuration = MaxD;	
		pstGUI[mode].updateDuration = true;
	}

	uint16_t index = (duration - MinD) / Delta;
	index = min(max(0, index), TableNum[freq] - 1);
	
	pstGUI[mode].m_wMaxEnergy = EnergyTable[11 * (freq - 1) + index];
		
	uint16_t MaxEnergy = pstGUI[mode].m_wMaxEnergy;
		
	if (pstGUI[mode].m_wMaxEnergy != MaxEnergy)
	{
		pstGUI[mode].m_wMinEnergy = 0;
		pstGUI[mode].m_wMaxEnergy = MaxEnergy;
		pstGUI[mode].updateEnergy = true;
	}
}

bool CheckLimits(uint16_t *freq, uint16_t *duration, uint16_t *energy, APP_PROFILE mode)
{
	bool UpdateFreq = false;
	bool update = false;
	
	switch (mode)
	{
		case PROFILE_SINGLE:
			pstGUI[mode].m_wMinFreq = 1;
			pstGUI[mode].m_wMaxFreq = 1;
			if (*freq != 1)
			{
				*freq = 1;
				UpdateFreq = true;
			}
		break;
		case PROFILE_SLOW:
			pstGUI[mode].m_wMinFreq = PROFILE_SLOW_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_SLOW_MAX_FREQ;
		break;
		case PROFILE_MEDIUM:
			pstGUI[mode].m_wMinFreq = PROFILE_MEDIUM_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_MEDIUM_MAX_FREQ;
		break;
		case PROFILE_FAST:
			pstGUI[mode].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
		break;
	}
	
	if (mode != PROFILE_SINGLE)
	{
		if (*freq < pstGUI[mode].m_wMinFreq)	{	*freq = pstGUI[mode].m_wMinFreq;	UpdateFreq = true;	}
		if (*freq > pstGUI[mode].m_wMaxFreq)	{	*freq = pstGUI[mode].m_wMaxFreq;	UpdateFreq = true;	}
	}
	
	if (UpdateFreq)
	{
		// Update Frequency
		frameData_LaserDiode.laserprofile.Frequency = *freq;
		update = true;
	}
	
	//freq = min(max(1, freq), 10);
	uint16_t MinD = MinDurationTable[*freq];
	uint16_t MaxD = MaxDurationTable[*freq];
	uint16_t Delta = (MaxD - MinD) / (TableNum[*freq]-1);
	uint16_t step ;
	
	if (mode == PROFILE_MEDIUM)
	{
		MinD = 10;
		MaxD = 100;	
	}
	
	bool UpdateDuration = false;
	if (*duration < MinD)	{	*duration = MinD;	UpdateDuration = true;	}
	if (*duration > MaxD)	{	*duration = MaxD;	UpdateDuration = true;	}
	
	if (UpdateDuration)
	{			
		// Update duration helpers	
		step = 1;
		NormalizeStep(&MinD, &MaxD, &step, 20, step_table);
		pstGUI[mode].m_wDurationStep = step;
		pstGUI[mode].m_wDurationNumSteps = (MaxD - MinD)/step;
		pstGUI[mode].m_wMinDuration = MinD;
		pstGUI[mode].m_wMaxDuration = MaxD;
		
		// Update Duration
		frameData_LaserDiode.laserprofile.DurationCnt = (*duration) / pstGUI[mode].m_wDurationStep;
		frameData_LaserDiode.lasersettings.Duration = *duration;
		
		update = true;
	}
	
	uint16_t index = (*duration - MinD) / Delta;
	index = min(max(0, index), TableNum[*freq] - 1);
	
	pstGUI[mode].m_wMaxEnergy = EnergyTable[11 * (*freq - 1) + index];
	
	if (*energy > pstGUI[mode].m_wMaxEnergy)
	{
		*energy = pstGUI[mode].m_wMaxEnergy;
		
		// Update energy helpers
		step = 1;
		uint16_t MaxEnergy = pstGUI[mode].m_wMaxEnergy;
		uint16_t Offset = 0;
		NormalizeStep(&Offset, &MaxEnergy, &step, 20, step_table);
		pstGUI[mode].m_wEnergyStep = step;
		pstGUI[mode].m_wEnergyNumSteps = (pstGUI[mode].m_wMaxEnergy)/step;
		pstGUI[mode].m_wMinEnergy = 0;
		pstGUI[mode].m_wMaxEnergy = MaxEnergy;
		
		// Update Energy
		frameData_LaserDiode.laserprofile.EnergyCnt = *energy / pstGUI[mode].m_wEnergyStep;
		frameData_LaserDiode.lasersettings.Energy = *energy;
		
		update = true;
	}
	
	return update;
}

void CalculateDurationSteps(uint16_t *freq, uint16_t *duration, APP_PROFILE Profile)
{
	uint16_t step = 1;
	uint16_t MinD = pstGUI[Profile].m_wMinDuration;
	uint16_t MaxD = pstGUI[Profile].m_wMaxDuration;
	NormalizeStep(&MinD, &MaxD, &step, 20, step_table);
	pstGUI[Profile].m_wDurationStep = step;
	pstGUI[Profile].m_wDurationNumSteps = (MaxD - MinD)/step;
	pstGUI[Profile].updateDuration = false;
	// Update Duration
	frameData_LaserDiode.laserprofile.DurationCnt = *duration / pstGUI[Profile].m_wDurationStep;
	frameData_LaserDiode.lasersettings.Duration = *duration;
}

void CalculateEnergySteps(uint16_t *freq, uint16_t *energy, APP_PROFILE Profile)
{
	uint16_t step = 1;
	uint16_t MaxEnergy = pstGUI[Profile].m_wMaxEnergy;
	uint16_t MinEnergy = pstGUI[Profile].m_wMinEnergy;
	NormalizeStep(&MinEnergy, &MaxEnergy, &step, 20, step_table);
	pstGUI[Profile].m_wEnergyStep = step;
	pstGUI[Profile].m_wEnergyNumSteps = (pstGUI[Profile].m_wMaxEnergy - MinEnergy)/step;
	pstGUI[Profile].updateEnergy = false;
	// Update Energy
	frameData_LaserDiode.laserprofile.EnergyCnt = *energy / pstGUI[Profile].m_wEnergyStep;
	frameData_LaserDiode.lasersettings.Energy = *energy;
}

void CalculateAllSteps(uint16_t *freq, uint16_t *duration, APP_PROFILE mode)
{
	uint16_t MinD = MinDurationTable[*freq];
	uint16_t MaxD = MaxDurationTable[*freq];
	if (mode == PROFILE_MEDIUM)
	{
		MinD = 10;
		MaxD = 100;
	}
	uint16_t Delta = (MaxD - MinD) / (TableNum[*freq]-1);
	uint16_t step = 1;
	
	if (mode == PROFILE_FAST)
	{
		// Update energy helpers
		pstGUI[PROFILE_FAST].m_wEnergyStep = 1;
		pstGUI[PROFILE_FAST].m_wEnergyNumSteps = 13 - 6;
		pstGUI[PROFILE_FAST].m_wMinEnergy = 6;
		pstGUI[PROFILE_FAST].m_wMaxEnergy = 12;
		
		// Update duration helpers
		pstGUI[PROFILE_FAST].m_wDurationStep = 1;
		pstGUI[PROFILE_FAST].m_wDurationNumSteps = 35 - 18;
		pstGUI[PROFILE_FAST].m_wMinDuration = 18;
		pstGUI[PROFILE_FAST].m_wMaxDuration = 35;
		return;
	}
	
	// Update duration helpers
	NormalizeStep(&MinD, &MaxD, &step, 20, step_table);
	pstGUI[mode].m_wDurationStep = step;
	pstGUI[mode].m_wDurationNumSteps = (MaxD - MinD)/step;
	pstGUI[mode].updateDuration = false;
	
	uint16_t index = (*duration - MinD) / Delta;
	index = min(max(0, index), TableNum[*freq] - 1);
	pstGUI[mode].m_wMaxEnergy = EnergyTable[11 * (*freq - 1) + index];
	
	// Update energy helpers
	step = 1;
	uint16_t MaxEnergy = pstGUI[mode].m_wMaxEnergy;
	uint16_t Offset = 0;
	NormalizeStep(&Offset, &MaxEnergy, &step, 20, step_table);
	pstGUI[mode].m_wEnergyStep = step;
	pstGUI[mode].m_wEnergyNumSteps = (pstGUI[mode].m_wMaxEnergy - Offset)/step;
	pstGUI[mode].updateEnergy = false;
}

bool FreqLimits(uint16_t *freq, APP_PROFILE mode)
{
	bool UpdateFreq = false;
	switch (mode)
	{
		case PROFILE_SINGLE:
			pstGUI[mode].m_wMinFreq = 1;
			pstGUI[mode].m_wMaxFreq = 1;
			if (*freq != 1)
			{
				*freq = 1;
				UpdateFreq = true;
				}
		break;
		case PROFILE_SLOW:
			pstGUI[mode].m_wMinFreq = PROFILE_SLOW_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_SLOW_MAX_FREQ;
		break;
		case PROFILE_MEDIUM:
			pstGUI[mode].m_wMinFreq = PROFILE_MEDIUM_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_MEDIUM_MAX_FREQ;
		break;
		case PROFILE_FAST:
			pstGUI[mode].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
			pstGUI[mode].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
		break;
	}
	if (mode != PROFILE_SINGLE)
	{
		if (*freq < pstGUI[mode].m_wMinFreq)	UpdateFreq = true;
		if (*freq > pstGUI[mode].m_wMaxFreq)	UpdateFreq = true;
	}
	
	return UpdateFreq;
}

bool CheckLimitsFastMode(uint16_t *freq, uint16_t *duration, uint16_t *energy)
{
	pstGUI[PROFILE_FAST].m_wMinFreq = PROFILE_FAST_MIN_FREQ;
	pstGUI[PROFILE_FAST].m_wMaxFreq = PROFILE_FAST_MAX_FREQ;
	
	if (*freq < pstGUI[PROFILE_FAST].m_wMinFreq)	{	*freq = pstGUI[PROFILE_FAST].m_wMinFreq;	}
	if (*freq > pstGUI[PROFILE_FAST].m_wMaxFreq)	{	*freq = pstGUI[PROFILE_FAST].m_wMaxFreq;	}
	
	frameData_LaserDiode.laserprofile.Frequency = *freq;
	
	if (*energy > 13) { *energy = 13;  }
	if (*energy <  6) { *energy = 6; }
		
	// Update energy helpers
	pstGUI[PROFILE_FAST].m_wEnergyStep = 1;
	pstGUI[PROFILE_FAST].m_wEnergyNumSteps = 13 - 6;
	pstGUI[PROFILE_FAST].m_wMinEnergy = 6;
	pstGUI[PROFILE_FAST].m_wMaxEnergy = 13;
	
	// Update Energy
	frameData_LaserDiode.laserprofile.EnergyCnt = *energy / pstGUI[PROFILE_FAST].m_wEnergyStep;
	frameData_LaserDiode.lasersettings.Energy = *energy;	
	
	*duration = (*energy * 1440) / 500; // Calculate max duration
	
	// Update duration helpers
	pstGUI[PROFILE_FAST].m_wDurationStep = 1;
	pstGUI[PROFILE_FAST].m_wDurationNumSteps = 35 - 18;
	pstGUI[PROFILE_FAST].m_wMinDuration = 18;
	pstGUI[PROFILE_FAST].m_wMaxDuration = 35;
		
	// Update Duration
	frameData_LaserDiode.laserprofile.DurationCnt = *duration / pstGUI[PROFILE_FAST].m_wDurationStep;
	frameData_LaserDiode.lasersettings.Duration = *duration;
	
	return true;
}

void LaserPreset(uint16_t *freq, uint16_t *duration, uint16_t *energy, APP_PROFILE mode)
{
	UpdateLimits(*freq, *duration, *energy, mode);
	CalculateAllSteps(freq, duration, mode);
	
	m_structLaserProfile[mode].Frequency = *freq;
	m_structLaserProfile[mode].EnergyCnt = *energy / pstGUI[mode].m_wEnergyStep;
	m_structLaserProfile[mode].DurationCnt = *duration / pstGUI[mode].m_wDurationStep;
	m_structLaserSettings[mode].Duration = *duration;
	m_structLaserSettings[mode].Energy = *energy;
	m_structLaserSettings[mode].FlushesLimit = 4; // deprecated
	
	if (mode == PROFILE_FAST)
		m_structLaserSettings[mode].FlushesLimit = 0;
}

void MelaninPreset(uint16_t melanin)
{
	uint16_t freq = 3;
	uint16_t duration = 65;
	uint16_t energy = 22;
	
	Profile = PROFILE_SLOW;
	frameData_LaserDiode.mode = Profile;
	if (melanin < 10)
	{
		frameData_LaserDiode.phototype = 1;
		freq = 3; duration = 85; energy = 27;
	}
	else
	if (melanin < 20)
	{
		frameData_LaserDiode.phototype = 2;
		freq = 3; duration = 80; energy = 27;
	}
	else
	if (melanin < 35)
	{
		frameData_LaserDiode.phototype = 3;
		freq = 3; duration = 80; energy = 27;
	}
	else
	if (melanin < 49)
	{
		frameData_LaserDiode.phototype = 4;
		freq = 3; duration = 75; energy = 24;
	}
	else
	if (melanin < 72)
	{
		frameData_LaserDiode.phototype = 5;
		freq = 3; duration = 100; energy = 20;
	}
	else
	{
		frameData_LaserDiode.phototype = 6;
		freq = 1; duration = 200; energy = 30;
	}
	
	LaserPreset(&freq, &duration, &energy, Profile);
}

void PhototypePreset(uint16_t phototype)
{
	uint16_t freq = 3;
	uint16_t duration = 65;
	uint16_t energy = 22;
	
	Profile = PROFILE_SLOW;
	frameData_LaserDiode.mode = Profile;
	frameData_LaserDiode.phototype = phototype;
	
	switch (phototype)
	{
		case 1:
		{
			frameData_LaserDiode.melanin = 5;
			freq = 3; duration = 85; energy = 27;
		}
		break;
		case 2:
		{
			frameData_LaserDiode.melanin = 14;
			freq = 3; duration = 80; energy = 27;
		}
		break;
		case 3:
		{
			frameData_LaserDiode.melanin = 27;
			freq = 3; duration = 80; energy = 27;
		}
		break;
		case 4:
		{
			frameData_LaserDiode.melanin = 41;
			freq = 3; duration = 75; energy = 24;
		}
		break;
		case 5:
		{
			frameData_LaserDiode.melanin = 61;
			freq = 3; duration = 100; energy = 20;
		}
		break;
		case 6:
		{
			frameData_LaserDiode.melanin = 85;
			freq = 1; duration = 200; energy = 30;
		}
	}
	
	LaserPreset(&freq, &duration, &energy, Profile);
}

void LoadGlobalVariables(void)
{
	// Copy counters
	memcpy((void*)&FlushesGlobalLD, (void*)&global_flash_data->LaserDiodePulseCounter, sizeof(uint32_t));
	memcpy((void*)&FlushesGlobalSS, (void*)&global_flash_data->SolidStatePulseCounter, sizeof(uint32_t));
	memcpy((void*)&FlushesGlobalSS2,(void*)&global_flash_data->SolidStatePulseCounter2, sizeof(uint32_t));
	memcpy((void*)&FlushesGlobalLP, (void*)&global_flash_data->LongPulsePulseCounter, sizeof(uint32_t));
	
	// Copy profile states
	memcpy((void*)&m_structLaserProfile, (void*)&global_flash_data->m_structLaserProfile, sizeof(m_structLaserProfile));
	memcpy((void*)&m_structLaserSettings, (void*)&global_flash_data->m_structLaserSettings, sizeof(m_structLaserSettings));
}

void fmemcpy(uint8_t* dst, uint8_t* src, uint16_t len)
{
	FLASH_WaitForLastOperation((uint32_t)50000U);
	
	for (uint16_t i = 0; i < len; i++)
	{
		
		CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
		FLASH->CR |= FLASH_PSIZE_BYTE;
		FLASH->CR |= FLASH_CR_PG;
		
		dst[i] = src[i];
		
		FLASH_WaitForLastOperation((uint32_t)50000U);
		FLASH->CR &= (~FLASH_CR_PG);
	}
}

void StoreGlobalVariables(void)
{
	FLASH_EraseInitTypeDef flash_erase = {0};
	
	flash_erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	flash_erase.Banks = FLASH_BANK_1;
	flash_erase.Sector = FLASH_SECTOR_11;
	flash_erase.NbSectors = 1;
	flash_erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	
	uint32_t sector_error = 0;
	
	while (HAL_FLASH_Unlock() != HAL_OK);
	HAL_FLASHEx_Erase(&flash_erase, &sector_error);
	
	FLASH_WaitForLastOperation((uint32_t)50000U);
	HAL_FLASH_Lock();
	
	while (HAL_FLASH_Unlock() != HAL_OK);
	
	/*HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_LASERDATA_BASE, frameData_LaserDiode.PulseCounter);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_LASERDATA_BASE + 4, frameData_SolidStateLaser.PulseCounter);*/
	
	// Copy presets
	fmemcpy((void*)&global_flash_data->LaserDiodePulseCounter, (void*)&FlushesGlobalLD, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->SolidStatePulseCounter, (void*)&FlushesGlobalSS, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->SolidStatePulseCounter2, (void*)&FlushesGlobalSS2, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->LongPulsePulseCounter, (void*)&FlushesGlobalLP, sizeof(uint32_t));
	
	// Copy profile states
	fmemcpy((void*)&global_flash_data->m_structLaserProfile, (void*)&m_structLaserProfile, sizeof(m_structLaserProfile));
	fmemcpy((void*)&global_flash_data->m_structLaserSettings, (void*)&m_structLaserSettings, sizeof(m_structLaserSettings));
	
	HAL_FLASH_Lock();
}
