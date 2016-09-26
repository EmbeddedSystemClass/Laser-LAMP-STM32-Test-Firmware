#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void LaserDiodeWork_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(frameData_LaserDiode));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	if (pic_id == FRAME_PICID_LASERDIODE_STARTED)
		DiodeLaser_en = true;
	else
		DiodeLaser_en = false;
	
	if (frameData_LaserDiode.buttons.onReadyBtn != 0)
	{
		// On Ready Pressed
		frameData_LaserDiode.buttons.onReadyBtn = 0;
		//SetPicId(FRAME_PICID_LASERDIODE_START, 100);
		new_pic_id = FRAME_PICID_LASERDIODE_START;
		update = true;
	}
	
	if (frameData_LaserDiode.buttons.onStartBtn != 0)
	{
		// On Start Pressed
		frameData_LaserDiode.buttons.onStartBtn = 0;
		//SetPicId(FRAME_PICID_LASERDIODE_STARTED, 100);
		new_pic_id = FRAME_PICID_LASERDIODE_STARTED;
		update = true;
	}
	
	if (frameData_LaserDiode.buttons.onStopBtn != 0)
	{
		// On Stop Pressed
		frameData_LaserDiode.buttons.onStopBtn = 0;
		//SetPicId(FRAME_PICID_LASERDIODE_INPUT, 100);
		new_pic_id = FRAME_PICID_LASERDIODE_INPUT;
		update = true;
	}
	
	switch (pic_id)
	{
		case FRAME_PICID_LASERDIODE_READY:
			break;
		case FRAME_PICID_LASERDIODE_START:
			break;
		case FRAME_PICID_LASERDIODE_STARTED:
			break;
	}
	
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
