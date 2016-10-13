#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "LaserMisc.h"
#include "WiFiThread.h"
#include "GlobalVariables.h"

#include <math.h>
#include "arm_math.h"

#include "cmsis_os.h"

extern void SetDACValue(float32_t value);

extern char WiFi_NetworkPassword[32];

void WifiAuthenticationFrame_Process(uint16_t pic_id)
{
	char* value;
	ReadVariable(FRAMEDATA_WIFIAUTHENTICATION_PASSWORD, (void**)&value, 32);
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		memcpy(WiFi_NetworkPassword, value, 32);
	else 
		return;
	
	for (uint16_t i = 0; i < 32; i++)
		if (WiFi_NetworkPassword[i] == (char)0xff) WiFi_NetworkPassword[i] = 0;
	//WiFi_NetworkPassword[5] = 0;
}
