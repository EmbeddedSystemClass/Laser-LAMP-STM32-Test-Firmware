#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

DGUS_LASERDIODE frameData_LaserDiode;

extern volatile float32_t temperature;

void LaserDiodeInput_Process(uint16_t pic_id)
{
	bool update = false;
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(frameData_LaserDiode));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100);
	
	conver_laserdata(&frameData_LaserDiode, value);
	
	if (frameData_LaserDiode.buttons.onInputBtn != 0)
	{
		// On Input Pressed
		frameData_LaserDiode.buttons.onInputBtn = 0;
		
		if (temperature > 29.0f)
			SetPicId(FRAME_PICID_LASERDIODE_TEMPERATUREOUT, 100);
		else
			SetPicId(FRAME_PICID_LASERDIODE_PREPARETIMER, 100);
		
		update = true;
	}
	
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	}
}
