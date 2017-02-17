#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <math.h>
#include "arm_math.h"

#include "DGUS.h"

#include "LaserMisc.h"

#define FLASH_LASERDATA_BASE 0x080E0000

#define DEBUG_SOLID_STATE_LASER
#define NEW_SOUNDSCHEME
//#define NEW_COOLSCHEME
#define NEW_DOUBLECOOLSCHEME

extern int32_t LOGHASH[16];

#define LOG_ID_TEMPERATURE	1
#define LOG_ID_FLOW					2
#define LOG_ID_COOLINGLEVEL	3
#define LOG_ID_PELTIER			4
#define LOG_ID_COOLINGFAN		5
#define LOG_ID_COUNTER1			6
#define LOG_ID_COUNTER2			7
#define LOG_ID_COUNTER3			8
#define LOG_ID_COUNTER4			9
#define LOG_ID_FREQUENCY		10
#define LOG_ID_POWER				11
#define LOG_ID_DURATION			12

//Date & time
extern DWIN_TIMEDATE datetime;

// DGUS control variables
extern uint16_t g_wDGUSTimeout;

// Timer control variables
extern int16_t m_wMillSec;
extern int16_t m_wSeconds;
extern int16_t m_wMinutes;
extern int16_t m_wSetSec;
extern int16_t m_wSetMin;

// Temperature control variables
extern float32_t temperature_cool_on;
extern float32_t temperature_cool_off;
extern float32_t temperature_overheat;
extern float32_t temperature_overheat_solidstate;
extern float32_t temperature_normal;

// Publish data
extern float32_t frequency_publish;
extern float32_t duration_publish;
extern float32_t energy_publish;

// Flow global variable
extern float32_t flow_low;
extern float32_t flow_normal;

// Service menu password
extern char password[6];
extern char ip_addr[16];
extern bool ip_addr_updated;

// Global state variables
extern volatile float32_t temperature;
extern volatile float32_t flow1;
extern volatile float32_t flow2;
extern volatile float32_t VoltageMonitor;
extern volatile float32_t CurrentMonitor;

// Laser ID
extern LASER_ID LaserID;
extern MENU_ID MenuID;

typedef struct PROFILE_FRACTLASER_STRUCT
{
	uint16_t mode_freq_min[3];
	uint16_t mode_freq_max[3];
	//uint16_t mode_energy_min[3];
	//uint16_t mode_energy_max[3];
} PROFILE_FRACTLASER, *PPROFILE_FRACTLASER;
                                              
// Private variables
extern uint16_t pic_id;
extern bool g_peltier_en;
extern bool prepare;
extern bool RemoteControl;
extern bool WiFiConnectionEstabilished;
extern bool SolidStateLaser_en;
extern bool DiodeLaser_en;
extern bool DiodeLaserOnePulse_en;
extern bool LaserStarted;
extern uint16_t subFlushes;
extern uint16_t subFlushesCount;
extern uint32_t Flushes;
extern uint32_t FlushesCount;
extern uint32_t FlushesSessionLD;
extern uint32_t FlushesGlobalLD;
extern uint32_t FlushesSessionSS;
extern uint32_t FlushesGlobalSS;
extern uint32_t FlushesSessionSS2;
extern uint32_t FlushesGlobalSS2;
extern uint32_t FlushesSessionLP;
extern uint32_t FlushesGlobalLP;
extern uint32_t FlushesSessionFL;
extern uint32_t FlushesGlobalFL;
extern volatile bool footswitch_en;
extern volatile bool footswitch_on;
extern volatile uint16_t switch_filter;
extern uint16_t switch_filter_threshold;
extern DGUS_LASERDIODE frameData_LaserDiode;
extern DGUS_SOLIDSTATELASER frameData_SolidStateLaser;
extern DGUS_SOLIDSTATELASER frameData_FractLaser;
extern PROFILE_FRACTLASER config_FractLaser;

// Log Structures
typedef struct LOG_EVENT_STRUCT
{
	DWIN_TIMEDATE time;
	uint16_t cmd_type;
	uint16_t cmd_id;
	float32_t fvalue;
	int32_t ivalue;
	char data[256];
} LOG_EVENT, *PLOG_EVENT;

// Laser Diode Data Structures

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

typedef struct FLASH_GLOBAL_DATA_STRUCT
{
	// Laser counters presed
	uint32_t LaserDiodePulseCounter;
	uint32_t SolidStatePulseCounter;
	uint32_t SolidStatePulseCounter2;
	uint32_t LongPulsePulseCounter;
	uint32_t FractLaserPulseCounter;
	
	// GUI preset
	DGUS_LASERPROFILE	m_structLaserProfile [5];
	DGUS_LASERSETTINGS m_structLaserSettings[5];
} FLASH_GLOBAL_DATA, *PFLASH_GLOBAL_DATA;

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

void LoadGlobalVariablesFromSD(FILE* fp);
void StoreGlobalVariablesFromSD(FILE* fp);
void ClearGlobalVariables(void);
void LoadGlobalVariables(void);
void StoreGlobalVariables(void);
void TryStoreGlobalVariables(void);

// Flash data
extern PFLASH_GLOBAL_DATA global_flash_data;

#endif
