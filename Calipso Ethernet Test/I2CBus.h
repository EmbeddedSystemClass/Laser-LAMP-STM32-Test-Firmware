#ifndef I2CBUS_H
#define I2CBUS_H

#include "stdbool.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

// Embedded EEPROM device address
#define LASER_EEPROM_I2C_ADDRESS								0x50

// EEPROM addresses for counters
#define EEPROM_LASERDIODE_CNT_MEM_ADDRESS				0x0000
#define EEPROM_SOLIDSTATE_CNT_MEM_ADDRESS				0x0004
#define EEPROM_SOLIDSTATE2_CNT_MEM_ADDRESS			0x0008
#define EEPROM_LONGPULSE_CNT_MEM_ADDRESS				0x000c
#define EEPROM_FRACTIONAL_CNT_MEM_ADDRESS				0x0010

extern I2C_HandleTypeDef hi2c1;

bool Init_I2C(void);
void Deinit_I2C(void);
void LoadCounterFromEEPROM(uint32_t *LaserPulseCounter, uint16_t memAddr);
void StoreCounterToEEPROM(uint32_t *LaserPulseCounter, uint16_t memAddr);

#endif
