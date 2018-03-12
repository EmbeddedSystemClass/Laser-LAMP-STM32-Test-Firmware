#include "Fract_Energy_Table.h"

//1440 nm energy table
uint16_t modeFract1440nmDurationTable[3 * FRACT1440NM_NUM_ENERGY] = { 500,  500,  500,  500,  500,  500,  500,  500,  500,  500,
																																		 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
																																		 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000};
uint16_t modeFract1440nmVoltageTable [3 * FRACT1440NM_NUM_ENERGY] = { 260,  280,  300,  320,  340,  360,  380,  400,  420,  440,
																																			260,  280,  300,  320,  340,  360,  380,  400,  420,  440,
																																			260,  280,  300,  320,  340,  360,  380,  400,  420,  440};
uint16_t modeFract1440nmEnergyTable  [3 * FRACT1440NM_NUM_ENERGY] = { 120,	201,	504,	637,	920, 1243, 1590, 2138, 2325, 2707,
																																			512,	965, 1474, 2053, 2661, 3392, 4048, 4800, 5392, 6160,
																																		 1088, 1923, 2387, 3920, 4464, 5568, 6672, 7792, 8896, 9984};

//1340 nm energy table
uint16_t modeFract1340nmDurationTable[3 * FRACT1340NM_NUM_ENERGY] = { 600,  600,  600,  600,  600,  600,  600,
																																		 1000, 1000, 1000, 1000, 1000, 1000, 1000,
																																		 2000, 2000, 2000, 2000, 2000, 2000, 2000};
uint16_t modeFract1340nmVoltageTable [3 * FRACT1340NM_NUM_ENERGY] = {	320,  340,  360,  380,  400,  420,  440,
																																			320,  340,  360,  380,  400,  420,  440,
																																			320,  340,  360,  380,  400,  420,  440};
uint16_t modeFract1340nmEnergyTable  [3 * FRACT1340NM_NUM_ENERGY] = { 1100,	1190,	1280,	1320,	1360, 1390, 1500,
																																			1100,	1250, 1400, 1600, 1700, 1800, 2000,
																																		  1750,	2100, 2400, 2600, 2800, 3300, 3400};

float FractVoltageTrim(float voltage, uint32_t counter)
{
	return voltage + (float)((uint32_t)counter / 50000);
}
