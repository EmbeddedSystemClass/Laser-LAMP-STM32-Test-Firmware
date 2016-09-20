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
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(frameData_LaserDiode));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100);
	
	convert_laserdata(&frameData_LaserDiode, value);
	
	switch (pic_id)
	{
		case FRAME_PICID_LASERDIODE_FLOWERROR:
			//frameData_LaserDiode.timer.timer_minutes = (uint16_t)temperature;
			//frameData_LaserDiode.timer.timer_seconds = (uint16_t)(temperature * 10.0f) % 10;
			if (flow > flow_normal)
			{
				if (!prepare)
				{
					frameData_LaserDiode.state = 1;
					SetPicId(FRAME_PICID_LASERDIODE_READY, 100);
				}
				else
					SetPicId(FRAME_PICID_LASERDIODE_PREPARETIMER, 100);
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
					SetPicId(FRAME_PICID_LASERDIODE_READY, 100);
				}
				else
					SetPicId(FRAME_PICID_LASERDIODE_PREPARETIMER, 100);
			}
			update = true;
			break;
		case FRAME_PICID_LASERDIODE_PREPARETIMER:
			frameData_LaserDiode.timer.timer_minutes = m_wMinutes;
			frameData_LaserDiode.timer.timer_seconds = m_wSeconds;
			if (!prepare)
			{
				frameData_LaserDiode.state = 1;
				SetPicId(FRAME_PICID_LASERDIODE_READY, 100);
			}
			update = true;
			break;
	}
	
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	}
}
