#include "Erb_Energy_Table.h"

uint16_t global_Erb_Duration_Table[3 * ERB_VOLTAGES_NUM] = { 300,	 300,  300,  300,  300,	 300,
																													   500,	 500,  500,  500,  500,	 500,
																													   700,	 700,  700,  700,  700,	 700};
uint16_t global_Erb_Voltage_Table [3 * ERB_VOLTAGES_NUM] = { 340,  360,  380,  400,  420,  440,
																														 340,  360,  380,  400,  420,  440,
																														 340,  360,  380,  400,  420,  440};
uint16_t global_Erb_Energy_Table  [3 * ERB_VOLTAGES_NUM] = {   2,	  50,	 100,	 200,	 250,  260,
																														  90,	 220,  300,  600, 1000, 1300,
																														 170,  450,  750, 1000, 1200, 1500};