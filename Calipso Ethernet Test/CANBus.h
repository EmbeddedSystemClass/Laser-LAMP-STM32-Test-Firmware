#ifndef __CANBUS_H
#define __CANBUS_H

#include "stm32f4xx_hal_can.h"

extern CAN_HandleTypeDef hcan1;

void Init_CAN();

#endif