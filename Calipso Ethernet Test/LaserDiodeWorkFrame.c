#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "SDCard.h"

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
	{
		if (Profile == PROFILE_SINGLE)
		{
			FlushesCount = 1;
		}
		if ((Profile != PROFILE_SINGLE) && (Profile != PROFILE_FAST))
			FlushesCount = 1000000;
		DiodeLaser_en = true;
	}
	else
	{
		DiodeLaser_en = false;
		DiodeLaserOnePulse_en = false;
	}
	
	// Control peltier cooling
	CoolOn();
	CoolSet((frameData_LaserDiode.cooling + 1) * 17);
	
	if (frameData_LaserDiode.buttons.onReadyBtn != 0)
	{
		// On Ready Pressed
		frameData_LaserDiode.buttons.onReadyBtn = 0;
		new_pic_id = FRAME_PICID_LASERDIODE_START;
		__MISC_LASERDIODE_ON();
		update = true;
	}
	
	if (frameData_LaserDiode.buttons.onStartBtn != 0)
	{
		log_LaserDiodeStart(frequency_publish, duration_publish, energy_publish, FlushesGlobalLD);
		
		// On Start Pressed
		frameData_LaserDiode.buttons.onStartBtn = 0;
		new_pic_id = FRAME_PICID_LASERDIODE_STARTED;
		DiodeLaser_en = true;
		update = true;
	}
	
	if (frameData_LaserDiode.buttons.onStopBtn != 0)
	{
		log_LaserDiodeStop(FlushesGlobalLD);
		
		// On Stop Pressed
		frameData_LaserDiode.buttons.onStopBtn = 0;
		new_pic_id = FRAME_PICID_LASERDIODE_INPUT;
		DiodeLaser_en = false;
		DiodeControlPulseStop();
		__MISC_LASERDIODE_OFF();
		SetDACValue(0.0f);
		update = true;
		
		StoreGlobalVariables();
	}
	
	if (frameData_LaserDiode.buttons.onCancelBtn != 0)
	{
		// On Stop Pressed
		frameData_LaserDiode.buttons.onCancelBtn = 0;
		new_pic_id = FRAME_PICID_LASERDIODE_INPUT;
		DiodeLaser_en = false;
		DiodeControlPulseStop();
		__MISC_LASERDIODE_OFF();
		SetDACValue(0.0f);
		update = true;
		
		StoreGlobalVariables();
	}
	
	switch (pic_id)
	{
		case FRAME_PICID_LASERDIODE_READY:
			//__MISC_LASERDIODE_OFF();
			break;
		case FRAME_PICID_LASERDIODE_START:
			//__MISC_LASERDIODE_ON();
			break;
		case FRAME_PICID_LASERDIODE_STARTED:
			//__MISC_LASERDIODE_ON();
			break;
	}
	
	if (frameData_LaserDiode.PulseCounter != FlushesGlobalLD)
	{
		frameData_LaserDiode.PulseCounter = FlushesGlobalLD;
		frameData_LaserDiode.SessionPulseCounter = FlushesSessionLD;
		update = true;
	}
	
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
