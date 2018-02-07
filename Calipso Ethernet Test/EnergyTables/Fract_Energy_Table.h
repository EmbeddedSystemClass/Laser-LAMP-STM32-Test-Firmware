#ifndef __FRACT_ENERGY_TABLE_H
#define __FRACT_ENERGY_TABLE_H

#include <stdint.h>

#define FRACT1440NM_NUM_ENERGY 10
#define FRACT1340NM_NUM_ENERGY 7

extern uint16_t modeFract1440nmDurationTable[3 * FRACT1440NM_NUM_ENERGY];
extern uint16_t modeFract1440nmVoltageTable [3 * FRACT1440NM_NUM_ENERGY];
extern uint16_t modeFract1440nmEnergyTable  [3 * FRACT1440NM_NUM_ENERGY];

extern uint16_t modeFract1340nmDurationTable[3 * FRACT1340NM_NUM_ENERGY];
extern uint16_t modeFract1340nmVoltageTable [3 * FRACT1340NM_NUM_ENERGY];
extern uint16_t modeFract1340nmEnergyTable  [3 * FRACT1340NM_NUM_ENERGY];

#endif