#include "Erb_Energy_Table.h"

uint16_t global_Erb_Duration_Table[3 * ERB_VOLTAGES_NUM] = { 300,	 300,  300,  300,  300,	 300,
																													   500,	 500,  500,  500,  500,	 500,
																													   700,	 700,  700,  700,  700,	 700};
uint16_t global_Erb_Voltage_Table [3 * ERB_VOLTAGES_NUM] = { 340,  360,  380,  400,  420,  440,
																														 340,  360,  380,  400,  420,  440,
																														 340,  360,  380,  400,  420,  440};
uint16_t global_Erb_Energy_Table  [3 * ERB_VOLTAGES_NUM] = {   2,	  13,	  39,	  81,	 165,  275,
																														  63,	 175,  250,  388,  525,  750,
																														 150,  275,  538,  675,  888, 1125};