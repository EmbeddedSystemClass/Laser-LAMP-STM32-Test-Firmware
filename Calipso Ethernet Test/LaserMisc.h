#ifndef __LASERMISC_H
#define __LASERMISC_H

#include "stm32f4xx_hal.h"
#include <math.h>
#include "arm_math.h"

typedef enum LASER_ID_ENUM {
	LASER_ID_DIODELASER  = 0x00,
	LASER_ID_SOLIDSTATE  = 0x01,
	LASER_ID_SOLIDSTATE2 = 0x02,
	LASER_ID_LONGPULSE   = 0x03,
} LASER_ID;

typedef enum MENU_ID_ENUM {
	MENU_ID_DIODELASER  = 0x00,
	MENU_ID_SOLIDSTATE  = 0x01,
	MENU_ID_SOLIDSTATE2 = 0x02,
	MENU_ID_LONGPULSE   = 0x03,
	MENU_ID_MENU  		  = 0x04,
} MENU_ID;

// Relays
#define MISC_GPIO_RELAY1									GPIO_PIN_4	// relay 1
#define MISC_GPIO_RELAY2									GPIO_PIN_5	// relay 2
#define MISC_GPIO_RELAY3									GPIO_PIN_6	// relay 3
#define MISC_GPIO_RELAY4									GPIO_PIN_7	// relay 4

// Outputs
#define GPIO_PIN_VoltageMonitor_nCS				GPIO_PIN_0 	// PORT GPIOG
#define GPIO_PIN_CurrentMonitor_nCS				GPIO_PIN_1 	// PORT GPIOG
#define GPIO_PIN_CurrentProgram_nCS				GPIO_PIN_7 	// PORT GPIOE
//#define GPIO_PIN_LaserDiodeLock											// Not Used
#define GPIO_PIN_LaserDiodeEnable					GPIO_PIN_14	// PORT GPIOF
#define GPIO_PIN_LaserLED									GPIO_PIN_3	// PORT GPIOA
#define GPIO_PIN_LaserLED2								GPIO_PIN_11	// PORT GPIOB

// Inputs
#define GPIO_PIN_Laser_ID0								GPIO_PIN_3 	// PORT GPIOE
#define GPIO_PIN_Laser_ID1								GPIO_PIN_2 	// PORT GPIOE
#define GPIO_PIN_LaserDiode_Pulse					GPIO_PIN_5 	// PORT GPIOE
#define GPIO_PIN_SimmerSensor							GPIO_PIN_4 	// PORT GPIOF
#define GPIO_PIN_ChargeModuleOn						GPIO_PIN_3 	// PORT GPIOF
#define GPIO_PIN_ChargeModuleFault				GPIO_PIN_1 	// PORT GPIOF
#define GPIO_PIN_ChargeModuleOvervoltage	GPIO_PIN_0 	// PORT GPIOF
#define GPIO_PIN_ChargeModuleOverheating	GPIO_PIN_14 // PORT GPIOC
#define GPIO_PIN_ChargeModuleReady				GPIO_PIN_15 // PORT GPIOC
#define GPIO_PIN_LaserDiodeFault					GPIO_PIN_13	// PORT GPIOF

// set outputs
#define __MISC_RELAY1_ON()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY1, GPIO_PIN_SET)
#define __MISC_RELAY2_ON()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY2, GPIO_PIN_SET)
#define __MISC_RELAY3_ON()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY3, GPIO_PIN_SET)
#define __MISC_RELAY4_ON()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY4, GPIO_PIN_SET)

#define __MISC_LASERDIODE_ON()	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_LaserDiodeEnable, GPIO_PIN_SET)
#define __MISC_LASERLED_ON()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_LaserLED,         GPIO_PIN_SET)
#define __MISC_LASERLED2_ON()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_LaserLED2,        GPIO_PIN_SET)

// reset outputs
#define __MISC_RELAY1_OFF()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY1, GPIO_PIN_RESET)
#define __MISC_RELAY2_OFF()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY2, GPIO_PIN_RESET)
#define __MISC_RELAY3_OFF()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY3, GPIO_PIN_RESET)
#define __MISC_RELAY4_OFF()	HAL_GPIO_WritePin(GPIOB, MISC_GPIO_RELAY4, GPIO_PIN_RESET)

#define __MISC_LASERDIODE_OFF()	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_LaserDiodeEnable, GPIO_PIN_RESET)
#define __MISC_LASERLED_OFF()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_LaserLED,         GPIO_PIN_RESET)
#define __MISC_LASERLED2_OFF()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_LaserLED2,        GPIO_PIN_RESET)

// get inputs
#define __MISC_LASER_ID0()									HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_Laser_ID0)
#define __MISC_LASER_ID1()									HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_Laser_ID1)
#define __MISC_GETSIMMERSENSOR()						HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_SimmerSensor) == GPIO_PIN_RESET
#define __MISC_GETCHARGEMODULEPOWERSTATE()	HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_ChargeModuleOn) == GPIO_PIN_RESET
#define __MISC_GETCHARGEMODULEFAULTSTATE()	HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_ChargeModuleFault) == GPIO_PIN_RESET
#define __MISC_GETCHARGEMODULEOVSTATE()			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_ChargeModuleOvervoltage) == GPIO_PIN_RESET
#define __MISC_GETCHARGEMODULEOVHSTATE()		HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_ChargeModuleOverheating) == GPIO_PIN_RESET
#define __MISC_GETCHARGEMODULEREADYSTATE()	HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_ChargeModuleReady) == GPIO_PIN_RESET

#define __MISC_GETLASERDIODEFAULTSTATE()		HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LaserDiodeFault) == GPIO_PIN_RESET

void SpeakerInit(void);
void FlowInit(void);
void CoolInit(void);
void CoolOn(void);
void CoolOff(void);
void CoolSet(uint16_t cool);
void SoundOn(void);
void SoundOff(void);

LASER_ID GetLaserID(void);

#endif
