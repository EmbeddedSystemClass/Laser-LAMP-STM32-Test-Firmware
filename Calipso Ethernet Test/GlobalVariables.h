#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <math.h>
#include "arm_math.h"

#include "DGUS.h"

#include "LaserMisc.h"
#include "LaserDiodeGUIPreset.h"

#define FLASH_LASERDATA_BASE 0x080E0000

#define DEBUG_SOLID_STATE_LASER		// Turn off control scheme for solid state interface debug
#define NEW_SOUNDSCHEME						// use class-D amplifier
//#define NEW_COOLSCHEME					// amplified PWM by two mossfet channels
#define NEW_DOUBLECOOLSCHEME			// Added second PWM channel
//#define OLD_STYLE_LASER_SW			// One connector combine configuration
#define LASERIDCHECK_LASERDIODE 	// Laser diode enable check
#define LASERIDCHECK_IPL				 	// IPL enable check
//#define MAINMENU_IPL				 			// IPL enable check
#define LDPREPARETIMER_ENABLE			// Enable prepare timer for Laser Diode
//#define DEBUG_BRK								// Enable error breakpoints
#define FLOW_CHECK								// Enable flow check
#define CAN_SUPPORT								// CAN support enable
//#define USE_EMBEDDED_EEPROM			// Calipso EEPROM

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
#define LOG_ID_COUNTER5			10
#define LOG_ID_COUNTER6			11
#define LOG_ID_FREQUENCY		12
#define LOG_ID_POWER				13
#define LOG_ID_DURATION			14

// CAN ID for emmiters
#define LASER_CAN_ID_DIODELASER		0x01
#define LASER_CAN_ID_SOLIDSTATE		0x81
#define LASER_CAN_ID_SOLIDSTATE2	0x84 // reserved
#define LASER_CAN_ID_LONGPULSE		0x82
#define LASER_CAN_ID_FRACTIONAL		0x83
#define LASER_CAN_ID_IPL					0x85
#define LASER_CAN_ID_1440NM				0x86
#define LASER_CAN_ID_2940NM				0x87


// Base emmiter types
#define LASER_ID_MASK_DIODELASER	0x00000001
#define LASER_ID_MASK_SOLIDSTATE	0x00000002
#define LASER_ID_MASK_SOLIDSTATE2	0x00000004
#define LASER_ID_MASK_LONGPULSE		0x00000008
#define LASER_ID_MASK_FRACTIONAL	0x00000010
#define LASER_ID_MASK_IPL					0x00000020
#define LASER_ID_MASK_1440NM			0x00000040
#define LASER_ID_MASK_2940NM			0x00000080


typedef enum LASER_ID_ENUM {
	LASER_ID_FRACTLASER   = 0x00,
	LASER_ID_SOLIDSTATE   = 0x01,
	LASER_ID_SOLIDSTATE2  = 0x02,
	LASER_ID_LONGPULSE    = 0x03,
	LASER_ID_DIODELASER   = 0x04,
	LASER_ID_IPL				  = 0x05,
	LASER_ID_1440NM			  = 0x06,
	LASER_ID_2940NM			  = 0x07,
	LASER_ID_UNKNOWN      = 0xff,
	LASER_ID_NOTCONNECTED = 0x77
} LASER_ID;

typedef enum MENU_ID_ENUM {
	MENU_ID_FRACTLASER   = 0x00,
	MENU_ID_SOLIDSTATE   = 0x01,
	MENU_ID_SOLIDSTATE2  = 0x02,
	MENU_ID_LONGPULSE    = 0x03,
	MENU_ID_MENU  		   = 0x04,
	MENU_ID_DIODELASER   = 0x05,
	MENU_ID_IPL          = 0x06,
	MENU_ID_1340NM       = 0x07,
	MENU_ID_2940NM			 = 0x08
} MENU_ID;

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
extern bool g_peltier_en;
extern bool g_cooling_en;
extern uint16_t cooling_level;

// Flow global variable
extern float32_t flow_low;
extern float32_t flow_normal;

// Service menu password
extern char password[6];
extern char ip_addr[16];
extern bool ip_addr_updated;

// Global state variables
extern volatile float32_t temperature_slot0;
extern volatile float32_t temperature_slot1;
extern volatile int8_t slot0_can_id;
extern volatile int8_t slot1_can_id;
extern volatile LASER_ID slot0_id;
extern volatile LASER_ID slot1_id;
extern volatile float32_t temperature;
extern volatile float32_t flow1;
extern volatile float32_t flow2;
extern volatile float32_t VoltageMonitor;
extern volatile float32_t CurrentMonitor;

typedef struct UUID_struct
{
	uint16_t X;
	uint16_t Y;
	uint8_t WAF_NUM;
	char LOT_NUM[7];
} UUID;


extern volatile UUID slot0_can_uid;
extern volatile UUID slot1_can_uid;
// Laser ID
//extern LASER_ID LaserID;
extern MENU_ID MenuID;

typedef struct PROFILE_FRACTLASER_STRUCT
{
	uint16_t mode_freq_min[3];
	uint16_t mode_freq_max[3];
	//uint16_t mode_energy_min[3];
	//uint16_t mode_energy_max[3];
} PROFILE_FRACTLASER, *PPROFILE_FRACTLASER;
                                              
// Private variables
extern uint32_t LaserSet;
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
extern uint32_t Flushes;				//?
extern uint32_t FlushesCount;		//?
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
extern uint32_t FlushesSessionIPL;
extern uint32_t FlushesGlobalIPL;
extern uint32_t FlushesSession1340nm;
extern uint32_t FlushesGlobal1340nm;
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

typedef struct FLASH_GLOBAL_DATA_STRUCT
{
	// Laser counters presed
	uint32_t LaserDiodePulseCounter;
	uint32_t SolidStatePulseCounter;
	uint32_t SolidStatePulseCounter2;
	uint32_t LongPulsePulseCounter;
	uint32_t FractLaserPulseCounter;
	uint32_t IPLLaserPulseCounter;
	uint32_t Laser1340nmPulseCounter;
	uint32_t Laser2940nmPulseCounter;
	
	/*// GUI preset
	DGUS_LASERPROFILE	m_structLaserProfile [5];
	DGUS_LASERSETTINGS m_structLaserSettings[5];*/
} FLASH_GLOBAL_DATA, *PFLASH_GLOBAL_DATA;

uint16_t min(uint16_t x, uint16_t y);
uint16_t max(uint16_t x, uint16_t y);

void LoadGlobalVariablesFromSD(FILE* fp);
void StoreGlobalVariablesFromSD(FILE* fp);
void ClearGlobalVariables(void);
void LoadGlobalVariables(void);
void StoreGlobalVariables(void);
void TryStoreGlobalVariables(void);

void SolidStateLaserPulseReset(LASER_ID laser_id);
void SolidStateLaserPulseInc(LASER_ID laser_id);
uint32_t GetSolidStateGlobalPulse(LASER_ID laser_id);
uint32_t GetSolidStateSessionPulse(LASER_ID laser_id);

LASER_ID GetLaserID(void);
bool CheckEmmiter(LASER_ID laser_id);
LASER_ID IdentifyEmmiter(uint8_t id, uint32_t *laser_set);
void CANDeviceReadCounter(uint8_t slot_id, uint8_t id);
void CANDeviceWriteCounter(LASER_ID laser_id, uint8_t laser_can_id);

// Flash data
extern PFLASH_GLOBAL_DATA global_flash_data;

#endif
