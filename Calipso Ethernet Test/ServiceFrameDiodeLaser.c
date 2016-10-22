#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "LaserMisc.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

#define FRAMEDATA_SERVICEDIODE_BASE				0x0032
#define FRAMEDATA_SERVICEDIODESTATE_BASE		0x003B

typedef struct FRAMEDATA_SERVICEDIODE_STRUCT {
	// control
	uint16_t Voltage_set;
	uint16_t Voltage_en;
	uint16_t duration_set;
	uint16_t frequency_set;
	uint16_t btn_start;
	uint16_t btn_stop;
	
	// values
	uint16_t Voltage_value;
	uint16_t duration_value;
	uint16_t frequency_value;
	
	// states
	uint16_t Power_ready;
	uint16_t Voltage_monitor; // Write only
	uint16_t Fault;
	
	// Power control
	uint16_t Power_en;
} FRAMEDATA_SERVICEDIODE;

typedef struct FRAMEDATA_SERVICEDIODESTATE_STRUCT {
	// states
	uint16_t Power_ready;
	uint16_t Voltage_monitor; // Write only
	uint16_t Fault;
} FRAMEDATA_SERVICEDIODESTATE;

FRAMEDATA_SERVICEDIODE frameData_ServiceDiode;

extern void SetDACValue(float32_t value);

void ServiceDiodeFrame_Process(uint16_t pic_id)
{
	bool update = false;
	FRAMEDATA_SERVICEDIODE* value;
	ReadVariable(FRAMEDATA_SERVICEDIODE_BASE, (void**)&value, sizeof(frameData_ServiceDiode));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_array_w((uint16_t*)&frameData_ServiceDiode, (uint16_t*)value, sizeof(frameData_ServiceDiode));
	else 
		return;
	
	if (frameData_ServiceDiode.Voltage_set == 0x01) 
	{	
		frameData_ServiceDiode.Voltage_set = 0x00;
		
		SetDACValue(10.0f * ((float32_t)frameData_ServiceDiode.Voltage_value / 20.0f));
		update = true;
	}
	
	if (frameData_ServiceDiode.duration_set == 0x01) 
	{	
		SetPulseDuration_ms(frameData_ServiceDiode.duration_value, frameData_ServiceDiode.duration_value * 2);
		frameData_ServiceDiode.duration_set = 0x00;
		update = true;
	}
	
	if (frameData_ServiceDiode.frequency_set == 0x01) 
	{
		SetPulseFrequency(frameData_ServiceDiode.frequency_value);
		frameData_ServiceDiode.frequency_set = 0x00;
		update = true;
	}
	
	if (frameData_ServiceDiode.btn_start == 0x01) 
	{	
		DiodeControlPulseStart();
		frameData_ServiceDiode.btn_start = 0x00;
		DiodeLaser_en = true;
		update = true;
	}
	
	if (frameData_ServiceDiode.btn_stop == 0x01) 
	{	
		DiodeControlPulseStop();
		frameData_ServiceDiode.btn_stop = 0x00;
		DiodeLaser_en = false;
		update = true;
	}
	
	if (frameData_ServiceDiode.Voltage_en == 0x01) 
		__MISC_LASERDIODE_ON();
	else
		__MISC_LASERDIODE_OFF();
	
	if (update)
	{
		WriteVariableConvert16(FRAMEDATA_SERVICEDIODE_BASE, &frameData_ServiceDiode, sizeof(frameData_ServiceDiode));
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	FRAMEDATA_SERVICEDIODESTATE state;
	if (__MISC_GETLASERDIODEFAULTSTATE()) state.Fault = 1;	else state.Fault = 0;
	state.Voltage_monitor = (uint16_t)(CurrentMonitor * 20.0f);
	
	WriteVariableConvert16(FRAMEDATA_SERVICEDIODESTATE_BASE, &state, sizeof(state));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}
