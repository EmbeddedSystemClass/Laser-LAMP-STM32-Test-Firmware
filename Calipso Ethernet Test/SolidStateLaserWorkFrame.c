#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void SolidStateLaserWork_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_SolidStateLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100).status != osEventTimeout))
		convert_laserdata_ss(&frameData_SolidStateLaser, value);
	else 
		return;
	
	osDelay(50);
	
	__SOLIDSTATELASER_DISCHARGEOFF();
	
	if (pic_id > 35 && pic_id <= 42)
		__SOLIDSTATELASER_HVON();
	else
		__SOLIDSTATELASER_HVOFF();
	
	if (pic_id >= 39 && pic_id <= 42)
		__SOLIDSTATELASER_SIMMERON();
	else
		__SOLIDSTATELASER_SIMMEROFF();
	
	if (frameData_SolidStateLaser.buttons.onSimmerBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onSimmerBtn = 0;
		
		new_pic_id = FRAME_PICID_SOLIDSTATE_SIMMER;
		
		update = true;
	}
	
	if (pic_id == FRAME_PICID_SOLIDSTATE_SIMMER)
	{
		osDelay(1000);
		SetPicId(FRAME_PICID_SOLIDSTATE_START, 100);
	}
	
	if (frameData_SolidStateLaser.buttons.onStartBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onStartBtn = 0;
		
		//LampControlPulseStart();
		
		new_pic_id = FRAME_PICID_SOLIDSTATE_WORK;
		
		update = true;
	}
	
	if (frameData_SolidStateLaser.buttons.onStopBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onStopBtn = 0;
		
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		
		new_pic_id = FRAME_PICID_SOLIDSTATE_INPUT;
		
		update = true;
	}
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, 100);
}
