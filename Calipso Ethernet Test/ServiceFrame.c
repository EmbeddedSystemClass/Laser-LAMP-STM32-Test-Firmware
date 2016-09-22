#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "LaserMisc.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

#define FRAMEDATA_SERVICE_BASE				0x0020
#define FRAMEDATA_SERVICESTATE_BASE		0x002B

typedef struct FRAMEDATA_SERVICE_STRUCT {
	// control
	uint16_t simmer_en;
	uint16_t HV_set;
	uint16_t HV_en;
	uint16_t discharge_en;
	uint16_t duration_set;
	uint16_t frequency_set;
	uint16_t btn_start;
	uint16_t btn_stop;
	// values
	uint16_t HV_value;
	uint16_t duration_value;
	uint16_t frequency_value;
	
	// states
	uint16_t simmer_ready;
	uint16_t HV_ready;
	uint16_t HV_On;
	uint16_t OV;
	uint16_t OVH;
	uint16_t Fault;
} FRAMEDATA_SERVICE;

typedef struct FRAMEDATA_SERVICESTATE_STRUCT {
	// states
	uint16_t simmer_ready;
	uint16_t HV_ready;
	uint16_t HV_On;
	uint16_t OV;
	uint16_t OVH;
	uint16_t Fault;
	
	// Write only
	uint16_t HV_monitor;
} FRAMEDATA_SERVICESTATE;

FRAMEDATA_SERVICE frameData_Service;

extern void SetDACValue(float32_t value);

void ServiceFrame_Process(uint16_t pic_id)
{
	bool update = false;
	FRAMEDATA_SERVICE* value;
	ReadVariable(FRAMEDATA_SERVICE_BASE, (void**)&value, sizeof(frameData_Service));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100).status != osEventTimeout))
		convert_array_w((uint16_t*)&frameData_Service, (uint16_t*)value, sizeof(frameData_Service));
	else 
		return;
	
	if (frameData_Service.HV_set == 0x01) 
	{	
		frameData_Service.HV_set = 0x00;
		
		SetDACValue(10.0f * ((float32_t)frameData_Service.HV_value / 450.0f));
		update = true;
	}
	
	if (frameData_Service.duration_set == 0x01) 
	{	
		LampSetPulseDuration(frameData_Service.duration_value-75);
		frameData_Service.duration_set = 0x00;
		update = true;
	}
	
	if (frameData_Service.frequency_set == 0x01) 
	{
		LampSetPulseFrequency(frameData_Service.frequency_value);
		frameData_Service.frequency_set = 0x00;
		update = true;
	}
	
	if (frameData_Service.btn_start == 0x01) 
	{	
		LampControlPulseStart();
		frameData_Service.btn_start = 0x00;
		update = true;
	}
	
	if (frameData_Service.btn_stop == 0x01) 
	{	
		LampControlPulseStop();
		frameData_Service.btn_stop = 0x00;
		update = true;
	}
	
	if (frameData_Service.HV_en == 0x01) 
		__SOLIDSTATELASER_HVON();
	else
		__SOLIDSTATELASER_HVOFF();
	
	if (frameData_Service.simmer_en == 0x01) 
		__SOLIDSTATELASER_SIMMERON();
	else
		__SOLIDSTATELASER_SIMMEROFF();
	
	if (frameData_Service.discharge_en == 0x01) 
		__SOLIDSTATELASER_DISCHARGEON();
	else
		__SOLIDSTATELASER_DISCHARGEOFF();
	
	if (update)
	{
		WriteVariableConvert16(FRAMEDATA_SERVICE_BASE, &frameData_Service, sizeof(frameData_Service));
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	}
	
	FRAMEDATA_SERVICESTATE state;
	if (__MISC_GETSIMMERSENSOR()					) state.simmer_ready = 1;	else state.simmer_ready = 0;
	if (__MISC_GETCHARGEMODULEPOWERSTATE()) state.HV_On = 1;				else state.HV_On = 0;
	if (__MISC_GETCHARGEMODULEFAULTSTATE()) state.Fault = 1;				else state.Fault = 0;
	if (__MISC_GETCHARGEMODULEOVSTATE()		) state.OV = 1;						else state.OV = 0;
	if (__MISC_GETCHARGEMODULEOVHSTATE()	) state.OVH = 1;					else state.OVH = 0;
	if (__MISC_GETCHARGEMODULEREADYSTATE()) state.HV_ready = 1;			else state.HV_ready = 0;
	state.HV_monitor = (uint16_t)(VoltageMonitor * 45.0f);
	
	WriteVariableConvert16(FRAMEDATA_SERVICESTATE_BASE, &state, sizeof(state));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
}
