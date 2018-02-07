#ifndef __ERB_ENRGY_TABLE_H
#define __ERB_ENRGY_TABLE_H

#include <stdint.h>

#define ERB_VOLTAGES_NUM	11

extern uint16_t global_Erb_Duration_Table[3 * ERB_VOLTAGES_NUM];
extern uint16_t global_Erb_Voltage_Table[3 * ERB_VOLTAGES_NUM];
extern uint16_t global_Erb_Energy_Table[3 * ERB_VOLTAGES_NUM];

#endif
