#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void FractLaserPrepare_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_FractLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
			convert_laserdata_ss(&frameData_FractLaser, value);
	else 
		return;
	
	switch (pic_id)
	{
		case FRAME_PICID_FRACTLASER_FLOWERROR:
			if (flow1 > flow_normal)
			{
				new_pic_id = FRAME_PICID_FRACTLASER_SIMMERSTART;
			}
			update = true;
			break;
		case FRAME_PICID_FRACTLASER_OVERHEATING:
			if (temperature < temperature_normal)
			{
				new_pic_id = FRAME_PICID_FRACTLASER_SIMMERSTART;
			}
			update = true;
			break;
		case FRAME_PICID_FRACTLASER_FAULT:
			break;
	}
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_FractLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
