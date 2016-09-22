#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

extern void SetDACValue(float32_t value);

void SolidStateLaserInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_SolidStateLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, 100).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100).status != osEventTimeout))
		convert_laserdata_ss(&frameData_SolidStateLaser, value);
	else 
		return;
	
	LampSetPulseFrequency(frameData_SolidStateLaser.laserprofile.Frequency);
	LampSetPulseDuration(frameData_SolidStateLaser.lasersettings.Energy * 20);
	
	if (frameData_SolidStateLaser.lasersettings.Energy != frameData_SolidStateLaser.laserprofile.EnergyCnt)
	{
		frameData_SolidStateLaser.lasersettings.Energy = frameData_SolidStateLaser.laserprofile.EnergyCnt; // deprecated
		update = true;
	}

	__SOLIDSTATELASER_DISCHARGEON();
	
	if (frameData_SolidStateLaser.buttons.onInputBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onInputBtn = 0;
		
		new_pic_id = FRAME_PICID_SOLIDSTATE_SIMMERSTART;
		
		SetDACValue(9.0f);
		
		__SOLIDSTATELASER_DISCHARGEOFF();
		
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
