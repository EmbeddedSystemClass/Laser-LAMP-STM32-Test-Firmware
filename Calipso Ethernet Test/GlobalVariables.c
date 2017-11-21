#include "GlobalVariables.h"
#include "stm32f4xx_hal_flash.h"
#include <string.h>
#include "SDCard.h"
#include "CANBus.h"
#include "I2CBus.h"

//Date & time
DWIN_TIMEDATE datetime = {0};

int32_t LOGHASH[16];

// Flash data
PFLASH_GLOBAL_DATA global_flash_data = (PFLASH_GLOBAL_DATA)FLASH_LASERDATA_BASE;

// DGUS Control variables
uint16_t g_wDGUSTimeout = 200;

// Timer global variables
int16_t m_wMillSec = 0;
int16_t m_wSeconds = 0;
int16_t m_wMinutes = 3;
int16_t m_wSetSec  = 0;
int16_t m_wSetMin  = 3;

// Cooling global variables
float32_t temperature_cool_on = 26.5f;
float32_t temperature_cool_off = 25.0f;
float32_t temperature_overheat = 29.5f;
float32_t temperature_overheat_solidstate = 32.0f;
float32_t temperature_normal = 27.5f;

// Publish data
float32_t frequency_publish = 0.0f;
float32_t duration_publish = 0.0f;
float32_t energy_publish = 0.0f;
uint16_t cooling_level = 6;
bool g_peltier_en = false;
bool g_cooling_en = false;

// Flow global variable
float32_t flow_low = 1.0f;
float32_t flow_normal = 2.0f;

// Service menu password
char password[6] = "78965\0";
char ip_addr[16] = "\0";
bool ip_addr_updated = false;

// Global State Variables
volatile float32_t temperature_slot0 = 0.0f;
volatile float32_t temperature_slot1 = 0.0f;
volatile int8_t slot0_can_id;
volatile int8_t slot1_can_id;
volatile LASER_ID slot0_id;
volatile LASER_ID slot1_id;
volatile float32_t temperature = 0;
volatile float32_t flow1 = 9.0f;
volatile float32_t flow2 = 9.0f;
volatile float32_t VoltageMonitor = 0.0f;
volatile float32_t CurrentMonitor = 0.0f;

// Laser ID
//LASER_ID LaserID = LASER_ID_SOLIDSTATE;
MENU_ID MenuID = MENU_ID_SOLIDSTATE;

// Private variables
uint32_t LaserSet = 0;
uint16_t pic_id = 0;
bool prepare = false;   
bool RemoteControl = false;
bool WiFiConnectionEstabilished = false;
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
uint32_t FlushesSessionFL = 0;
uint32_t FlushesGlobalFL = 0;
uint32_t FlushesSessionIPL = 0;
uint32_t FlushesGlobalIPL = 0;
uint32_t FlushesSession1340nm = 0;
uint32_t FlushesGlobal1340nm = 0;
uint16_t switch_filter_threshold = 10;
volatile uint16_t switch_filter = 0;
volatile bool footswitch_en = false;
volatile bool footswitch_on = false;
DGUS_LASERDIODE frameData_LaserDiode;
DGUS_SOLIDSTATELASER frameData_SolidStateLaser;
DGUS_SOLIDSTATELASER frameData_FractLaser;
PROFILE_FRACTLASER config_FractLaser = {{1,1,1}, {5,3,3}};

uint16_t min(uint16_t x, uint16_t y) { return (x>y)?y:x; }
uint16_t max(uint16_t x, uint16_t y) { return (x>y)?x:y; }

static uint32_t _FlushesGlobalLD  = 0;
static uint32_t _FlushesGlobalSS  = 0;
static uint32_t _FlushesGlobalSS2 = 0;
static uint32_t _FlushesGlobalLP  = 0;
static uint32_t _FlushesGlobalFL  = 0;
static uint32_t _FlushesGlobalIPL  = 0;
static uint32_t _FlushesGlobal1340nm  = 0;

void LoadGlobalVariablesFromSD(FILE* fp)
{
	fscanf(fp, "Laser diode counter: %d\n", 			&FlushesGlobalLD );
	fscanf(fp, "Tattoo removal 1 counter: %d\n",	&FlushesGlobalSS );
	fscanf(fp, "Tattoo removal 2 counter: %d\n",	&FlushesGlobalSS2);
	fscanf(fp, "Long pulse counter: %d\n",				&FlushesGlobalLP );
	fscanf(fp, "Fractional laser counter: %d\n",	&FlushesGlobalFL );
	fscanf(fp, "IPL counter: %d\n",								&FlushesGlobalIPL );
	fscanf(fp, "1340nm counter: %d\n",						&FlushesGlobal1340nm );
	_FlushesGlobalLD  = FlushesGlobalLD;
	_FlushesGlobalSS  = FlushesGlobalSS;
	_FlushesGlobalSS2 = FlushesGlobalSS2;
	_FlushesGlobalLP  = FlushesGlobalLP;
	_FlushesGlobalFL  = FlushesGlobalFL;
	_FlushesGlobalIPL = FlushesGlobalIPL;
	_FlushesGlobal1340nm  = FlushesGlobal1340nm;
}

void StoreGlobalVariablesFromSD(FILE* fp)
{
	fprintf(fp, "Laser diode counter: %d\n", 			FlushesGlobalLD );
	fprintf(fp, "Tattoo removal 1 counter: %d\n",	FlushesGlobalSS );
	fprintf(fp, "Tattoo removal 2 counter: %d\n",	FlushesGlobalSS2);
	fprintf(fp, "Long pulse counter: %d\n",				FlushesGlobalLP );
	fprintf(fp, "Fractional laser counter: %d\n",	FlushesGlobalFL );
	fprintf(fp, "IPL counter: %d\n",							FlushesGlobalIPL );
	fprintf(fp, "1340nm counter: %d\n",						FlushesGlobal1340nm );
	_FlushesGlobalLD  = FlushesGlobalLD;
	_FlushesGlobalSS  = FlushesGlobalSS;
	_FlushesGlobalSS2 = FlushesGlobalSS2;
	_FlushesGlobalLP  = FlushesGlobalLP;
	_FlushesGlobalFL  = FlushesGlobalFL;
	_FlushesGlobalIPL = FlushesGlobalIPL;
	_FlushesGlobal1340nm  = FlushesGlobal1340nm;
}

void LoadGlobalVariables(void)
{
	slot1_id = GetLaserID();
#ifdef CAN_SUPPORT
	uint8_t len = 4;	
	//CANReadRegister(SLOT_ID_1, CAN_MESSAGE_TYPE_REGISTER_UID, (uint8_t*)slot1_can_uid, &len);
	
	if (!CANReadRegister(SLOT_ID_0, CAN_MESSAGE_TYPE_REGISTER_ID, (uint8_t*)&slot0_can_id, &len))
	{
		slot0_can_id = -1;
		slot0_id = LASER_ID_NOTCONNECTED;
	}
	else
		slot0_id = IdentifyEmmiter(slot0_can_id, &LaserSet);
	
	if (!CANReadRegister(SLOT_ID_1, CAN_MESSAGE_TYPE_REGISTER_ID, (uint8_t*)&slot1_can_id, &len))
	{
		slot1_can_id = -1;
		slot1_id = LASER_ID_NOTCONNECTED;
	}
	else
		slot1_id = IdentifyEmmiter(slot1_can_id, &LaserSet);
	
	CANDeviceReadCounter(SLOT_ID_0, slot0_can_id);
	CANDeviceReadCounter(SLOT_ID_1, slot1_can_id);
#else
#ifdef USE_EMBEDDED_EEPROM
		LoadCounterFromEEPROM(&FlushesGlobalLD,  EEPROM_LASERDIODE_CNT_MEM_ADDRESS);	_FlushesGlobalLD  = FlushesGlobalLD;
		LoadCounterFromEEPROM(&FlushesGlobalSS,  EEPROM_SOLIDSTATE_CNT_MEM_ADDRESS);	_FlushesGlobalSS  = FlushesGlobalSS;
		LoadCounterFromEEPROM(&FlushesGlobalSS2, EEPROM_SOLIDSTATE2_CNT_MEM_ADDRESS);	_FlushesGlobalSS2 = FlushesGlobalSS2;
		LoadCounterFromEEPROM(&FlushesGlobalLP,  EEPROM_LONGPULSE_CNT_MEM_ADDRESS);		_FlushesGlobalLP  = FlushesGlobalLP;
		LoadCounterFromEEPROM(&FlushesGlobalFL,  EEPROM_FRACTIONAL_CNT_MEM_ADDRESS);	_FlushesGlobalFL  = FlushesGlobalFL;
		//LoadCounterFromEEPROM(&FlushesGlobalIPL,  EEPROM_IPL_CNT_MEM_ADDRESS);		_FlushesGlobalIPL  = FlushesGlobalIPL;
		//LoadCounterFromEEPROM(&FlushesGlobal1340nm,  EEPROM_1340_CNT_MEM_ADDRESS);	_FlushesGlobal340nm  = FlushesGlobal1340nm;
#else
	if (!sdcard_ready)
	{
		// Copy counters
		memcpy((void*)&FlushesGlobalLD, (void*)&global_flash_data->LaserDiodePulseCounter, sizeof(uint32_t));
		memcpy((void*)&FlushesGlobalSS, (void*)&global_flash_data->SolidStatePulseCounter, sizeof(uint32_t));
		memcpy((void*)&FlushesGlobalSS2,(void*)&global_flash_data->SolidStatePulseCounter2, sizeof(uint32_t));
		memcpy((void*)&FlushesGlobalLP, (void*)&global_flash_data->LongPulsePulseCounter, sizeof(uint32_t));
		memcpy((void*)&FlushesGlobalFL, (void*)&global_flash_data->FractLaserPulseCounter, sizeof(uint32_t));
		memcpy((void*)&FlushesGlobalIPL, (void*)&global_flash_data->IPLLaserPulseCounter, sizeof(uint32_t));
		memcpy((void*)&FlushesGlobal1340nm, (void*)&global_flash_data->Laser1340nmPulseCounter, sizeof(uint32_t));
		
		// Copy profile states
		//memcpy((void*)&m_structLaserProfile, (void*)&global_flash_data->m_structLaserProfile, sizeof(m_structLaserProfile));
		//memcpy((void*)&m_structLaserSettings, (void*)&global_flash_data->m_structLaserSettings, sizeof(m_structLaserSettings));
	}
	else
	{
		FILE* fp = fopen("dump.txt", "r");
		if (fp != NULL)
		{
			LoadGlobalVariablesFromSD(fp);
			fclose(fp);
		}
		else
		{
			fp = fopen("dump.txt", "w");
			StoreGlobalVariablesFromSD(fp);
			fclose(fp);
		}
	}
#endif
#endif
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

void ClearGlobalVariables(void)
{
#ifdef USE_EMBEDDED_EEPROM
	FlushesGlobalLD  = 0;
	FlushesGlobalSS  = 0;
	FlushesGlobalSS2 = 0;
	FlushesGlobalLP  = 0;
	FlushesGlobalFL  = 0;
	FlushesGlobalIPL  = 0;
	FlushesGlobal1340nm  = 0;
	StoreCounterToEEPROM(&FlushesGlobalLD,  EEPROM_LASERDIODE_CNT_MEM_ADDRESS);		_FlushesGlobalLD  = FlushesGlobalLD;
	StoreCounterToEEPROM(&FlushesGlobalSS,  EEPROM_SOLIDSTATE_CNT_MEM_ADDRESS);		_FlushesGlobalSS  = FlushesGlobalSS;
	StoreCounterToEEPROM(&FlushesGlobalSS2, EEPROM_SOLIDSTATE2_CNT_MEM_ADDRESS);	_FlushesGlobalSS2 = FlushesGlobalSS2;
	StoreCounterToEEPROM(&FlushesGlobalLP,  EEPROM_LONGPULSE_CNT_MEM_ADDRESS);		_FlushesGlobalLP  = FlushesGlobalLP;
	StoreCounterToEEPROM(&FlushesGlobalFL,  EEPROM_FRACTIONAL_CNT_MEM_ADDRESS);		_FlushesGlobalFL  = FlushesGlobalFL;
	//StoreCounterToEEPROM(&FlushesGlobalIPL,  EEPROM_IPL_CNT_MEM_ADDRESS);		_FlushesGlobalIPL  = FlushesGlobalIPL;
	//StoreCounterToEEPROM(&FlushesGlobal1340nm,  EEPROM_1340_CNT_MEM_ADDRESS);		_FlushesGlobal1340nm  = FlushesGlobal1340nm;
#else
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
	
	FlushesGlobalLD = 0;
	FlushesGlobalSS = 0;
	FlushesGlobalSS2 = 0;
	FlushesGlobalLP = 0;
	FlushesGlobalIPL = 0;
	FlushesGlobal1340nm = 0;
	
	// Copy presets
	fmemcpy((void*)&global_flash_data->LaserDiodePulseCounter, (void*)&FlushesGlobalLD, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->SolidStatePulseCounter, (void*)&FlushesGlobalSS, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->SolidStatePulseCounter2, (void*)&FlushesGlobalSS2, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->LongPulsePulseCounter, (void*)&FlushesGlobalLP, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->IPLLaserPulseCounter, (void*)&FlushesGlobalIPL, sizeof(uint32_t));
	fmemcpy((void*)&global_flash_data->Laser1340nmPulseCounter, (void*)&FlushesGlobal1340nm, sizeof(uint32_t));
	
	// Copy profile states
	//fmemcpy((void*)&global_flash_data->m_structLaserProfile, (void*)&m_structLaserProfile, sizeof(m_structLaserProfile));
	//fmemcpy((void*)&global_flash_data->m_structLaserSettings, (void*)&m_structLaserSettings, sizeof(m_structLaserSettings));
	
	HAL_FLASH_Lock();
#endif
}

void StoreGlobalVariables(void)
{
#ifdef USE_EMBEDDED_EEPROM
		if (_FlushesGlobalLD  != FlushesGlobalLD ) {StoreCounterToEEPROM(&FlushesGlobalLD,  EEPROM_LASERDIODE_CNT_MEM_ADDRESS);	_FlushesGlobalLD  = FlushesGlobalLD; }
		if (_FlushesGlobalSS  != FlushesGlobalSS ) {StoreCounterToEEPROM(&FlushesGlobalSS,  EEPROM_SOLIDSTATE_CNT_MEM_ADDRESS);	_FlushesGlobalSS  = FlushesGlobalSS; }
		if (_FlushesGlobalSS2 != FlushesGlobalSS2) {StoreCounterToEEPROM(&FlushesGlobalSS2, EEPROM_SOLIDSTATE2_CNT_MEM_ADDRESS);_FlushesGlobalSS2 = FlushesGlobalSS2;	}
		if (_FlushesGlobalLP  != FlushesGlobalLP ) {StoreCounterToEEPROM(&FlushesGlobalLP,  EEPROM_LONGPULSE_CNT_MEM_ADDRESS);	_FlushesGlobalLP  = FlushesGlobalLP; }
		if (_FlushesGlobalFL  != FlushesGlobalFL ) {StoreCounterToEEPROM(&FlushesGlobalFL,  EEPROM_FRACTIONAL_CNT_MEM_ADDRESS);	_FlushesGlobalFL  = FlushesGlobalFL; }
#else
	if (!sdcard_ready)
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
		
		// Copy presets
		fmemcpy((void*)&global_flash_data->LaserDiodePulseCounter, (void*)&FlushesGlobalLD, sizeof(uint32_t));
		fmemcpy((void*)&global_flash_data->SolidStatePulseCounter, (void*)&FlushesGlobalSS, sizeof(uint32_t));
		fmemcpy((void*)&global_flash_data->SolidStatePulseCounter2, (void*)&FlushesGlobalSS2, sizeof(uint32_t));
		fmemcpy((void*)&global_flash_data->LongPulsePulseCounter, (void*)&FlushesGlobalLP, sizeof(uint32_t));
		fmemcpy((void*)&global_flash_data->FractLaserPulseCounter, (void*)&FlushesGlobalFL, sizeof(uint32_t));
		fmemcpy((void*)&global_flash_data->FractLaserPulseCounter, (void*)&FlushesGlobalFL, sizeof(uint32_t));
		
		/*// Copy profile states
		fmemcpy((void*)&global_flash_data->m_structLaserProfile, (void*)&m_structLaserProfile, sizeof(m_structLaserProfile));
		fmemcpy((void*)&global_flash_data->m_structLaserSettings, (void*)&m_structLaserSettings, sizeof(m_structLaserSettings));*/
		
		HAL_FLASH_Lock();
	}
#endif
}

void CANDeviceWriteCounter(LASER_ID laser_id, uint8_t laser_can_id)
{
	uint8_t len = 4;
	switch (laser_id)
	{
		case LASER_ID_FRACTLASER:
			if (_FlushesGlobalFL  != FlushesGlobalFL )
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalFL, len);
				_FlushesGlobalFL  = FlushesGlobalFL;
			}
			break;
		case LASER_ID_SOLIDSTATE:
			if (_FlushesGlobalSS  != FlushesGlobalSS )
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalSS, len);
				_FlushesGlobalSS  = FlushesGlobalSS;
			}
			break;
		case LASER_ID_SOLIDSTATE2:
			if (_FlushesGlobalSS2 != FlushesGlobalSS2)
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalSS2, len);
				_FlushesGlobalSS2 = FlushesGlobalSS2;
			}
			break;
		case LASER_ID_LONGPULSE:
			if (_FlushesGlobalLP  != FlushesGlobalLP )
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalLP, len);
				_FlushesGlobalLP  = FlushesGlobalLP;
			}
			break;
		case LASER_ID_DIODELASER:
			if (_FlushesGlobalLD  != FlushesGlobalLD )
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalLD, len);
				_FlushesGlobalLD  = FlushesGlobalLD;
			}
			break;
		case LASER_ID_IPL:
			if (_FlushesGlobalIPL  != FlushesGlobalIPL )
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalIPL, len);
				_FlushesGlobalIPL  = FlushesGlobalIPL;
			}
			break;
		case LASER_ID_1340NM:
			if (_FlushesGlobal1340nm  != FlushesGlobal1340nm )
			{
				CANWriteRegister(laser_can_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobal1340nm, len);
				_FlushesGlobal1340nm  = FlushesGlobal1340nm;
			}
			break;
		case LASER_ID_UNKNOWN:
			break;
		case LASER_ID_NOTCONNECTED:
			break;
	}
}

void TryStoreGlobalVariables(void)
{
#ifdef CAN_SUPPORT
	CANDeviceWriteCounter(slot0_id, SLOT_ID_0);
	CANDeviceWriteCounter(slot1_id, SLOT_ID_1);
#else
#ifdef USE_EMBEDDED_EEPROM
		if (_FlushesGlobalLD  != FlushesGlobalLD ) {StoreCounterToEEPROM(&FlushesGlobalLD,  EEPROM_LASERDIODE_CNT_MEM_ADDRESS);		_FlushesGlobalLD  = FlushesGlobalLD; }
		if (_FlushesGlobalSS  != FlushesGlobalSS ) {StoreCounterToEEPROM(&FlushesGlobalSS,  EEPROM_SOLIDSTATE_CNT_MEM_ADDRESS);		_FlushesGlobalSS  = FlushesGlobalSS; }
		if (_FlushesGlobalSS2 != FlushesGlobalSS2) {StoreCounterToEEPROM(&FlushesGlobalSS2, EEPROM_SOLIDSTATE2_CNT_MEM_ADDRESS);	_FlushesGlobalSS2 = FlushesGlobalSS2;	}
		if (_FlushesGlobalLP  != FlushesGlobalLP ) {StoreCounterToEEPROM(&FlushesGlobalLP,  EEPROM_LONGPULSE_CNT_MEM_ADDRESS);		_FlushesGlobalLP  = FlushesGlobalLP; }
		if (_FlushesGlobalFL  != FlushesGlobalFL ) {StoreCounterToEEPROM(&FlushesGlobalFL,  EEPROM_FRACTIONAL_CNT_MEM_ADDRESS);		_FlushesGlobalFL  = FlushesGlobalFL; }
		//if (_FlushesGlobalIPL  != FlushesGlobalIPL ) {StoreCounterToEEPROM(&FlushesGlobalIPL,  EEPROM_IPL_CNT_MEM_ADDRESS);		_FlushesGlobalIPL  = FlushesGlobalIPL; }
		//if (_FlushesGlobal1340nm  != FlushesGlobal1340nm ) {StoreCounterToEEPROM(&FlushesGlobal1340nm,  EEPROM_1340_CNT_MEM_ADDRESS);		_FlushesGlobal1340nm  = FlushesGlobal1340nm; }
#else
	if (sdcard_ready)
	{
		bool update = false;
		if (_FlushesGlobalLD  != FlushesGlobalLD ) update = true;
		if (_FlushesGlobalSS  != FlushesGlobalSS ) update = true;
		if (_FlushesGlobalSS2 != FlushesGlobalSS2) update = true;
		if (_FlushesGlobalLP  != FlushesGlobalLP ) update = true;
		if (_FlushesGlobalFL  != FlushesGlobalFL ) update = true;
		if (_FlushesGlobalIPL != FlushesGlobalIPL) update = true;
		if (_FlushesGlobal1340nm != FlushesGlobal1340nm) update = true;
		
		if (update)
		{
			FILE* fp = fopen("dump.txt", "w");
			StoreGlobalVariablesFromSD(fp);
			fclose(fp);
		}
	}
#endif
#endif
}

void SolidStateLaserPulseReset(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_FRACTLASER:
			FlushesSessionFL = 0;
			break;
		case LASER_ID_SOLIDSTATE:
			FlushesSessionSS = 0;
			break;
		case LASER_ID_SOLIDSTATE2:
			FlushesSessionSS2 = 0;
			break;
		case LASER_ID_LONGPULSE:
			FlushesSessionLP = 0;
			break;
		case LASER_ID_IPL:
			FlushesSessionIPL = 0;
			break;
		case LASER_ID_1340NM:
			FlushesSession1340nm = 0;
			break;
		case LASER_ID_DIODELASER:
			break;
		case LASER_ID_UNKNOWN:
			break;
		case LASER_ID_NOTCONNECTED:
			break;
	}		
}

void SolidStateLaserPulseInc(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_FRACTLASER:
			FlushesSessionFL++;
			FlushesGlobalFL++;
			break;
		case LASER_ID_SOLIDSTATE:
			FlushesSessionSS++;
			FlushesGlobalSS++;
			break;
		case LASER_ID_SOLIDSTATE2:
			FlushesSessionSS2++;
			FlushesGlobalSS2++;
			break;
		case LASER_ID_LONGPULSE:
			FlushesSessionLP++;
			FlushesGlobalLP++;
			break;
		case LASER_ID_IPL:
			FlushesSessionIPL++;
			FlushesGlobalIPL++;
			break;
		case LASER_ID_1340NM:
			FlushesSession1340nm++;
			FlushesGlobal1340nm++;
			break;
		case LASER_ID_DIODELASER:
			break;
		case LASER_ID_UNKNOWN:
			break;
		case LASER_ID_NOTCONNECTED:
			break;
	}		
}

uint32_t GetSolidStateGlobalPulse(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_FRACTLASER:
			return FlushesGlobalFL;
		case LASER_ID_SOLIDSTATE:
			return FlushesGlobalSS;
		case LASER_ID_SOLIDSTATE2:
			return FlushesGlobalSS2;
		case LASER_ID_LONGPULSE:
			return FlushesGlobalLP;
		case LASER_ID_DIODELASER:
			return FlushesGlobalLD;
		case LASER_ID_IPL:
			return FlushesGlobalIPL;
		case LASER_ID_1340NM:
			return FlushesGlobal1340nm;
		case LASER_ID_UNKNOWN:
			return 0xff;
		case LASER_ID_NOTCONNECTED:
			return 0xff;
	}		
	return 0;
}
uint32_t GetSolidStateSessionPulse(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_FRACTLASER:
			return FlushesSessionFL;
		case LASER_ID_SOLIDSTATE:
			return FlushesSessionSS;
		case LASER_ID_SOLIDSTATE2:
			return FlushesSessionSS2;
		case LASER_ID_LONGPULSE:
			return FlushesSessionLP;
		case LASER_ID_DIODELASER:
			return FlushesSessionLD;
		case LASER_ID_IPL:
			return FlushesSessionIPL;
		case LASER_ID_1340NM:
			return FlushesSession1340nm;
		case LASER_ID_UNKNOWN:
			return 0xff;
		case LASER_ID_NOTCONNECTED:
			return 0xff;
	}	
	return 0;
}

LASER_ID GetLaserID()
{
	return LASER_ID_FRACTLASER;
	/*
	if (__MISC_LASER_ID0() == GPIO_PIN_SET)
	{
		if (__MISC_LASER_ID1() == GPIO_PIN_SET)
		{			
			return LASER_ID_LONGPULSE;
		}
		else
		{
			return LASER_ID_SOLIDSTATE;
		}
	}
	else
	{
		if (__MISC_LASER_ID1() == GPIO_PIN_SET)
		{
			return LASER_ID_SOLIDSTATE2;
		}
		else
		{
			return LASER_ID_FRACTLASER;
		}
	}
	*/
}

bool CheckEmmiter(LASER_ID laser_id)
{
#ifdef CAN_SUPPORT
	switch (laser_id)
	{
		case LASER_ID_FRACTLASER:
			return ((LaserSet & LASER_ID_MASK_FRACTIONAL)  != 0);
		case LASER_ID_SOLIDSTATE:
			return ((LaserSet & LASER_ID_MASK_SOLIDSTATE)  != 0);
		case LASER_ID_SOLIDSTATE2:
			return ((LaserSet & LASER_ID_MASK_SOLIDSTATE2) != 0);
		case LASER_ID_LONGPULSE:
			return ((LaserSet & LASER_ID_MASK_LONGPULSE)   != 0);
		case LASER_ID_DIODELASER:
			return ((LaserSet & LASER_ID_MASK_DIODELASER)  != 0);
		case LASER_ID_IPL:
			return ((LaserSet & LASER_ID_MASK_IPL)  != 0);
		case LASER_ID_1340NM:
			return ((LaserSet & LASER_ID_MASK_1340NM)  != 0);
		case LASER_ID_UNKNOWN:
			return false;
		case LASER_ID_NOTCONNECTED:
			return false;
		//default:
		//	return false;
	}
	return false;
#else
	if (laser_id == LASER_ID_DIODELASER) return true;
	return (GetLaserID() == laser_id);
#endif
}

LASER_ID IdentifyEmmiter(uint8_t id, uint32_t *laser_set)
{
	if (id == (uint8_t)LASER_CAN_ID_DIODELASER	) { *laser_set |= LASER_ID_MASK_DIODELASER;  return LASER_ID_DIODELASER;  };
	if (id == (uint8_t)LASER_CAN_ID_SOLIDSTATE	) { *laser_set |= LASER_ID_MASK_SOLIDSTATE;  return LASER_ID_SOLIDSTATE;  };
	if (id == (uint8_t)LASER_CAN_ID_SOLIDSTATE2 ) { *laser_set |= LASER_ID_MASK_SOLIDSTATE2; return LASER_ID_SOLIDSTATE2; };
	if (id == (uint8_t)LASER_CAN_ID_LONGPULSE	  ) { *laser_set |= LASER_ID_MASK_LONGPULSE;   return LASER_ID_LONGPULSE;   };
	if (id == (uint8_t)LASER_CAN_ID_FRACTIONAL	) { *laser_set |= LASER_ID_MASK_FRACTIONAL;  return LASER_ID_FRACTLASER;  };
	if (id == (uint8_t)LASER_CAN_ID_IPL					) { *laser_set |= LASER_ID_MASK_IPL;				 return LASER_ID_IPL;  				};
	if (id == (uint8_t)LASER_CAN_ID_1340NM			) { *laser_set |= LASER_ID_MASK_1340NM;			 return LASER_ID_1340NM;  		};
	return LASER_ID_UNKNOWN;
}

void CANDeviceReadCounter(uint8_t slot_id, uint8_t id)
{
	uint8_t len = 4;
	switch (IdentifyEmmiter(id, &LaserSet))
	{
		case LASER_ID_FRACTLASER:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalFL, &len);
			_FlushesGlobalFL = FlushesGlobalFL;
			break;
		case LASER_ID_SOLIDSTATE:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalSS, &len);
			_FlushesGlobalSS = FlushesGlobalSS;
			break;
		case LASER_ID_SOLIDSTATE2:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalSS2, &len);
			_FlushesGlobalSS2 = FlushesGlobalSS2;
			break;
		case LASER_ID_LONGPULSE:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalLP, &len);
			_FlushesGlobalLP = FlushesGlobalLP;
			break;
		case LASER_ID_DIODELASER:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalLD, &len);
			_FlushesGlobalLD = FlushesGlobalLD;
			break;
		case LASER_ID_IPL:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobalIPL, &len);
			_FlushesGlobalIPL = FlushesGlobalIPL;
			break;
		case LASER_ID_1340NM:
			CANReadRegister(slot_id, CAN_MESSAGE_TYPE_REGISTER_CNT, (uint8_t*)&FlushesGlobal1340nm, &len);
			_FlushesGlobal1340nm = FlushesGlobal1340nm;
			break;
		case LASER_ID_UNKNOWN:
			break;
		case LASER_ID_NOTCONNECTED:
			break;
	}
}
