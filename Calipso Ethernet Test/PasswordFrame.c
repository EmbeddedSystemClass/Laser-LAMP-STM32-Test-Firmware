#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#define FRAMEDDATA_PASSWORD_BASE		0x0010

char frameData_Password[6];

void PasswordFrame_Process(uint16_t pic_id)
{
	char* value;
	ReadVariable(FRAMEDDATA_PASSWORD_BASE, (void**)&value, 6);
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		memcpy(frameData_Password, value, 5);
	else 
		return;
	frameData_Password[5] = 0;
	
	if (strcmp(frameData_Password, password) == 0)
		SetPicId(FRAME_PICID_SERVICE, g_wDGUSTimeout);
}
