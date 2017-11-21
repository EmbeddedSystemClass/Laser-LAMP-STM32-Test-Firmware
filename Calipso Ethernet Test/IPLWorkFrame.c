#include <string.h>
#include "Driver_USART.h"

#include "DGUS.h"
#include "SDCard.h"
#include "LaserMisc.h"
#include "SolidStateLaser.h"

// Calypso System Global Configuration and Variables
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

static uint16_t stacked_pic_id = FRAME_PICID_IPL_COOLING_TIMER;

void IPLWork_Process(uint16_t pic_id)
{
	volatile bool update = false;
	uint16_t new_pic_id = pic_id;
	
	uint16_t timer_min = frameData_LaserDiode.timer.timer_minutes;
	uint16_t timer_sec = frameData_LaserDiode.timer.timer_seconds;
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(DGUS_LASERDIODE));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	// Start pressed
	if (frameData_LaserDiode.buttons.onStartBtn != 0)
	{				
		// On Start Pressed
		frameData_LaserDiode.buttons.onStartBtn = 0;
		
		__MISC_LASERLED_ON();
		
		new_pic_id = FRAME_PICID_IPL_WORK;
		
		update = true;
	}
	
	if (frameData_LaserDiode.buttons.onStopBtn != 0)
	{		
		// On Input Pressed
		frameData_LaserDiode.buttons.onStopBtn = 0;
		
		__MISC_LASERLED_OFF();
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		frameData_FractLaser.state = 0;
		new_pic_id = FRAME_PICID_IPL_INPUT;
		update = true;
		StoreGlobalVariables();
	}
	
	if (frameData_LaserDiode.buttons.onCancelBtn != 0)
	{
		// On Input Pressed
		frameData_LaserDiode.buttons.onCancelBtn = 0;
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		frameData_FractLaser.state = 0;
		new_pic_id = FRAME_PICID_IPL_INPUT;
		update = true;
		StoreGlobalVariables();
	}
	
	// Update IPL counter
	if (frameData_LaserDiode.PulseCounter != GetSolidStateGlobalPulse(slot1_id))
	{
		frameData_LaserDiode.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
		frameData_LaserDiode.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);
		update = true;
	}
	
	// Update IPL interface
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	// Update IPL menu
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
