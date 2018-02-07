#ifndef __IPL_ENRGY_TABLE_H
#define __IPL_ENRGY_TABLE_H

#include <stdint.h>

#define IPL_VOLTAGES_NUM	14
#define IPL_DURATIONS_NUM	26

extern uint16_t global_IPL_Duration_Table[IPL_DURATIONS_NUM];
extern uint16_t global_IPL_Voltage_Table [IPL_VOLTAGES_NUM];
extern uint16_t global_IPL_Energy_Table  [IPL_VOLTAGES_NUM * IPL_DURATIONS_NUM];

uint16_t GetIPLEnergy(uint16_t voltage_id, uint16_t duration_id);

#endif
