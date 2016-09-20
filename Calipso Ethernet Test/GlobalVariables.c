#include "GlobalVariables.h"

// Timer global variables
int16_t m_wMillSec = 0;
int16_t m_wSeconds = 10;
int16_t m_wMinutes = 2;
int16_t m_wSetSec  = 10;
int16_t m_wSetMin  = 2;

// Cooling global variables
float32_t temperature_cool_on = 26.5f;
float32_t temperature_cool_off = 26.0f;
float32_t temperature_overheat = 29.0f;
float32_t temperature_normal = 27.0f;

// Flow global variable
float32_t flow_low = 7.0f;
float32_t flow_normal = 8.0f;

// Service menu password
char password[6] = "78965\0";

// Private variables
uint16_t pic_id = 0;
bool peltier_en = false;
bool prepare = false;   
bool RemoteControl = false;
volatile float32_t temperature = 0;
volatile float32_t flow = 9.0f;
DGUS_LASERDIODE frameData_LaserDiode;
DGUS_SOLIDSTATELASER frameData_SolidStateLaser;

void InitializeGlobalVariables()
{
}
