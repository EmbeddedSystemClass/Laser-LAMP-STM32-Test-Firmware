#ifndef __LP_ENRGY_TABLE_H
#define __LP_ENRGY_TABLE_H

#include <stdint.h>

#define SHORT_DURATIONS_NUM	10
#define STDRD_DURATIONS_NUM	9
#define LONGP_DURATIONS_NUM	10
#define VOLTAGES_SNUM 8
#define VOLTAGES_NUM 16//8

extern uint16_t duration_short[SHORT_DURATIONS_NUM];
extern uint16_t duration_stdrt[STDRD_DURATIONS_NUM+1];
extern uint16_t duration_long [LONGP_DURATIONS_NUM];

extern uint16_t energy_tbl[VOLTAGES_SNUM*SHORT_DURATIONS_NUM + VOLTAGES_NUM*SHORT_DURATIONS_NUM*2];
extern uint16_t voltage_tbl[VOLTAGES_SNUM*SHORT_DURATIONS_NUM + VOLTAGES_NUM*SHORT_DURATIONS_NUM*2];

#endif