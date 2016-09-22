#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h" 

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void LaserDiodeInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(frameData_LaserDiode));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	// State - input data
	if (frameData_LaserDiode.state != 0)
		{
			frameData_LaserDiode.state = 0;
			update = true;
		}
	
	if (frameData_LaserDiode.buttons.onInputBtn != 0)
	{
		// On Input Pressed
		frameData_LaserDiode.buttons.onInputBtn = 0;
		
		prepare = true;
		peltier_en = true;
		
		if (temperature > 29.0f)
			new_pic_id = FRAME_PICID_LASERDIODE_TEMPERATUREOUT;
			//SetPicId(FRAME_PICID_LASERDIODE_TEMPERATUREOUT, 100);
		else
			new_pic_id = FRAME_PICID_LASERDIODE_PREPARETIMER;
			//SetPicId(FRAME_PICID_LASERDIODE_PREPARETIMER, 100);
		
		update = true;
	}
	
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, 100);
}
