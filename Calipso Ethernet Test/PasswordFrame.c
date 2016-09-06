#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "SolidStateLaser.h"

#define FRAMEDDATA_PASSWORD_BASE		0x0010

char frameData_Password[6];

void PasswordFrame_Process(uint16_t pic_id)
{
	char* value;
	ReadVariable(FRAMEDDATA_PASSWORD_BASE, (void**)&value, 5);
	osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, 100);
	memcpy(frameData_Password, value, 5);
	frameData_Password[5] = 0;
	
	if (strcmp(frameData_Password, "78965") == 0)
		SetPicId(FRAME_PICID_SERVICE);
}
