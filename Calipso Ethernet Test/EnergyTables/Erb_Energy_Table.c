#include "Erb_Energy_Table.h"

uint16_t global_Erb_Duration_Table[3 * ERB_VOLTAGES_NUM] = { 330,	 330,  330,  330,  330,	 330,	 330,  330,  330,  330,	 330,	// soft
																													   530,	 530,  530,  530,  530,	 530,	 530,  530,  530,  530,	 530, // standart
																													   700,	 700,  700,  700,  700,	 700,	 700,  700,  700,  700,	 700};// hard
uint16_t global_Erb_Voltage_Table [3 * ERB_VOLTAGES_NUM] = { 340,  330,  360,  370,  380,  390,  400,  410,  420,  430,  440,
																														 340,  330,  360,  370,  380,  390,  400,  410,  420,  430,  440,
																														 340,  330,  360,  370,  380,  390,  400,  410,  420,  430,  440};
uint16_t global_Erb_Energy_Table  [3 * ERB_VOLTAGES_NUM] = {  18,		34,	  50,	 	75,	 100,	 150,	 200,	 225,	 250,	 255,  260,
																														  90,	 155,	 220,	 260,  300,	 450,  600,	 800, 1000,	1150, 1300,
																														 170,  310,	 450,	 600,  750,	 875, 1000,	1100, 1200,	1350, 1500};

float ErbVoltageTrim(float voltage, uint32_t counter)
{
	return voltage + (float)((uint32_t)counter / 50000);
}

float SSVoltageTrim(float voltage, uint32_t counter)
{
	return voltage + (float)((uint32_t)counter / 50000);
}