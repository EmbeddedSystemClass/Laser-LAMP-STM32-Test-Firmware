#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "LaserMisc.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

#define FRAMEDATA_COOLINGSERVICE_BASE					0x0F00
#define FRAMEDATA_COOLINGSERVICESTATE_BASE		0x0F06

DGUS_COOLINGSETTINGS frameData_CoolingService;

extern void SetDACValue(float32_t value);

void CoolingServiceFrame_Process(uint16_t pic_id)
{
	bool update = false;
	DGUS_COOLINGSETTINGS* value;
	ReadVariable(FRAMEDATA_COOLINGSERVICE_BASE, (void**)&value, sizeof(frameData_CoolingService));
	
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_array_w((uint16_t*)&frameData_CoolingService, (uint16_t*)value, sizeof(frameData_CoolingService));
	else 
		return; // Timeout
	
	if (frameData_CoolingService.CoolOffT != (uint16_t)(temperature_cool_off * 10))
	{
		frameData_CoolingService.CoolOffT = (uint16_t)(temperature_cool_off * 10);
		update = true;
	}
	
	if (frameData_CoolingService.CoolOnT != (uint16_t)(temperature_cool_on * 10))
	{
		frameData_CoolingService.CoolOnT = (uint16_t)(temperature_cool_on * 10);
		update = true;
	}
	
	if (frameData_CoolingService.DiodeOverheatingT != (uint16_t)(temperature_overheat * 10))
	{
		frameData_CoolingService.DiodeOverheatingT = (uint16_t)(temperature_overheat * 10);
		update = true;
	}
	
	if (frameData_CoolingService.SSOverheatingT != (uint16_t)(temperature_overheat_solidstate * 10))
	{
		frameData_CoolingService.SSOverheatingT = (uint16_t)(temperature_overheat_solidstate * 10);
		update = true;
	}
	
	if (frameData_CoolingService.FlowLowThreshold != (uint16_t)(flow_low * 10))
	{
		frameData_CoolingService.FlowLowThreshold = (uint16_t)(flow_low * 10);
		update = true;
	}
	
	if (frameData_CoolingService.FlowNormalThreshold != (uint16_t)(flow_normal * 10))
	{
		frameData_CoolingService.FlowNormalThreshold = (uint16_t)(flow_normal * 10);
		update = true;
	}
	
	uint16_t flowlow;
	if (flow1 < flow_low) flowlow = 1; else flowlow = 0;
	if (frameData_CoolingService.FlowLow != flowlow) 
	{
		frameData_CoolingService.FlowLow = flowlow;
		update = true;
	}
	
	uint16_t flowready;
	if (flow1 >= flow_normal) flowready = 1; else flowready = 0;
	if (frameData_CoolingService.FlowReady != flowready) 
	{
		frameData_CoolingService.FlowReady = flowready;
		update = true;
	}
	
	uint16_t overheating;
	if (temperature > temperature_overheat) overheating = 1; else overheating = 0;
	if (frameData_CoolingService.Overheating != overheating) 
	{
		frameData_CoolingService.Overheating = overheating;
		update = true;
	}
	
	uint16_t ready;
	if ((flowlow != 1) && (flowready == 1) && (overheating != 0) && (temperature < temperature_cool_on))
		ready = 1; else ready = 0;
	if (frameData_CoolingService.TemperatureReady != ready) 
	{
		frameData_CoolingService.TemperatureReady = ready;
		update = true;
	}
	
	if (update)
	{
		WriteVariableConvert16(FRAMEDATA_COOLINGSERVICE_BASE, &frameData_CoolingService, sizeof(frameData_CoolingService));
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
}
