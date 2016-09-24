#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void LaserDiodePrepare_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(frameData_LaserDiode));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	switch (pic_id)
	{
		case FRAME_PICID_LASERDIODE_FLOWERROR:
			frameData_LaserDiode.timer.timer_minutes = (uint16_t)flow1;
			frameData_LaserDiode.timer.timer_seconds = (uint16_t)(flow1 * 10.0f) % 10;
			if (flow1 > flow_normal)
			{
				if (!prepare)
				{
					frameData_LaserDiode.state = 1;
					new_pic_id = FRAME_PICID_LASERDIODE_READY;
				}
				else
					new_pic_id = FRAME_PICID_LASERDIODE_PREPARETIMER;
			}
			update = true;
			break;
		case FRAME_PICID_LASERDIODE_TEMPERATUREOUT:
			frameData_LaserDiode.timer.timer_minutes = (uint16_t)temperature;
			frameData_LaserDiode.timer.timer_seconds = (uint16_t)(temperature * 10.0f) % 10;
			frameData_LaserDiode.coolIcon = 1;
			if (temperature < temperature_normal)
			{
				if (!prepare)
				{
					frameData_LaserDiode.state = 1;
					new_pic_id = FRAME_PICID_LASERDIODE_READY;
				}
				else
					new_pic_id = FRAME_PICID_LASERDIODE_PREPARETIMER;
			}
			update = true;
			break;
		case FRAME_PICID_LASERDIODE_PREPARETIMER:
			frameData_LaserDiode.timer.timer_minutes = m_wMinutes;
			frameData_LaserDiode.timer.timer_seconds = m_wSeconds;
			if (!prepare)
			{
				frameData_LaserDiode.state = 1;
				new_pic_id = FRAME_PICID_LASERDIODE_READY;
			}
			update = true;
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
