#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdbool.h>
#include "arm_math.h"

void Init_DS18B20();
bool DS18B20_Reset();
void DS18B20_StartConvertion();
uint16_t DS18B20_ReadData();

#endif