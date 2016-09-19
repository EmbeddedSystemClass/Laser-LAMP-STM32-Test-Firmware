#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdbool.h>
#include "arm_math.h"

void Init_DS18B20(void);
bool DS18B20_Reset(void);
void DS18B20_StartConvertion(void);
uint16_t DS18B20_ReadData(void);

#endif
