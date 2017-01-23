#include <string.h>
#include "DGUS.h"
#include "SDCard.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "LaserMisc.h"
#include "WiFiThread.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

#include "cmsis_os.h"

void LogFrame_Process(uint16_t pic_id)
{	
	char str[256];
	uint16_t i = 0;
	uint16_t log_offset = 0;
	
	bool update = false;
	uint16_t* value;
	ReadVariable(FRAMEDATA_LOGOFFSET, (void**)&value, 6);
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		log_offset = convert_w(*value);
	else 
		return;	
	
	for (i = 0; i < 12; i++)
	{
		WriteVariable(FRAMEDATA_WIFISCANNING_LINE0_BASE + 0x100 * i, log_table[i].log_line, (strlen(log_table[i].log_line)<1)?1:strlen(log_table[i].log_line));
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	//WriteVariable(FRAMEDATA_WIFISCANNING_LINE0_BASE, log_table, 0x100 * 12);
	//osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	
	LOG_UPDATE_TABLE(log_offset);
	
	//osDelay(1000);
}