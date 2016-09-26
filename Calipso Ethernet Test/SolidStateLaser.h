#ifndef __SOLIDSTATELASER_H
#define __SOLIDSTATELASER_H

#include "stm32f4xx_hal.h"
#include <math.h>
#include "arm_math.h"

#define GPIO_PIN_FOOTSWITCH				GPIO_PIN_15 // PORT GPIOF, IN

#define GPIO_PIN_LAMP_Ready				GPIO_PIN_15 // PORT GPIOC, IN
#define GPIO_PIN_LAMP_HVINH				GPIO_PIN_13 // PORT GPIOC, OUT
#define GPIO_PIN_LAMP_HVOn				GPIO_PIN_3  // PORT GPIOF, IN
#define GPIO_PIN_LAMP_Fault 			GPIO_PIN_1  // PORT GPIOF, IN
#define GPIO_PIN_LAMP_OV    			GPIO_PIN_0  // PORT GPIOF, IN
#define GPIO_PIN_LAMP_OT    			GPIO_PIN_14 // PORT GPIOC, IN
#define GPIO_PIN_LAMP_SIMMERSEN 	GPIO_PIN_4	// PORT GPIOF, IN
#define GPIO_PIN_LAMP_SIMMEREN	 	GPIO_PIN_5	// PORT GPIOF, OUT
#define GPIO_PIN_LAMP_DISCHARGE		GPIO_PIN_2	// PORT GPIOF, OUT

#define __SOLIDSTATELASER_HVON()	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_LAMP_HVINH, GPIO_PIN_SET)
#define __SOLIDSTATELASER_HVOFF()	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_LAMP_HVINH, GPIO_PIN_RESET)

#define __SOLIDSTATELASER_SIMMERON()	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_LAMP_SIMMEREN, GPIO_PIN_SET)
#define __SOLIDSTATELASER_SIMMEROFF()	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_LAMP_SIMMEREN, GPIO_PIN_RESET)

#define __SOLIDSTATELASER_DISCHARGEON()	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_LAMP_DISCHARGE, GPIO_PIN_RESET)
#define __SOLIDSTATELASER_DISCHARGEOFF()	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_LAMP_DISCHARGE, GPIO_PIN_SET)

void LampControlInit(void);
void LampControlPulseStart(void);
void LampControlPulseStop(void);
void DiodeControlPulseStart(void);
void DiodeControlPulseStop(void);

void SetPulseDuration_us(uint16_t duration);
void SetPulseDuration_ms(uint16_t duration);
void SetPulseFrequency(float32_t frequency);

#endif
