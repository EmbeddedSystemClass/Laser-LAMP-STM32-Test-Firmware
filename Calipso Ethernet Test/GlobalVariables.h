#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <math.h>
#include "arm_math.h"

#include "DGUS.h"

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
extern float32_t temperature_normal;

// Flow global variable
extern float32_t flow_low;
extern float32_t flow_normal;

// Service menu password
extern char password[6];

// Global state variables
extern volatile float32_t temperature;
extern volatile float32_t flow;
extern volatile float32_t VoltageMonitor;
extern volatile float32_t CurrentMonitor;

// Private variables
extern uint16_t pic_id;
extern bool peltier_en;
extern bool prepare;
extern bool RemoteControl;
extern bool LaserStarted;
extern volatile bool footswitch_en;
extern volatile bool footswitch_on;
extern volatile uint16_t switch_filter;
extern uint16_t switch_filter_threshold;
extern DGUS_LASERDIODE frameData_LaserDiode;
extern DGUS_SOLIDSTATELASER frameData_SolidStateLaser;

#endif
