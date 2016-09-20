#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void SolidStateLaserPrepare_Process(uint16_t pic_id)
{
	bool update = false;
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_SolidStateLaser));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100);
	
	convert_laserdata_ss(&frameData_SolidStateLaser, value);
	
	/*if (frameData_SolidStateLaser.buttons.onSimmerBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onSimmerBtn = 0;
		
		SetPicId(FRAME_PICID_SOLIDSTATE_SIMMER, 100);
		
		update = true;
	}
	
	if (pic_id == FRAME_PICID_SOLIDSTATE_SIMMER)
	{
		osDelay(1000);
		SetPicId(FRAME_PICID_SOLIDSTATE_START, 100);
	}*/
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	}
}
