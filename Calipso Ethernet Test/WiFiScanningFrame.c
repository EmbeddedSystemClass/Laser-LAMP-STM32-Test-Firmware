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

DGUS_WIFISCANNINGLINE frameData_WiFiScanning[12];

void WifiScanningFrame_Process(uint16_t pic_id)
{
	uint16_t i = 0;
	
	osStatus status = osMessagePut(qid_WiFiCMDQueue, WIFI_CMD_STARTSCANNING, 3000);
	
	if (status != osEventTimeout)
	{
		WiFi_State_ScanningMode = true;
		while (WiFi_State_ScanningMode);
		
		for (i = 0; i < 12; i++)
		{
			frameData_WiFiScanning[i].channel = WiFi_APs[i].Channel;
			frameData_WiFiScanning[i].RSSI = WiFi_APs[i].RSSI;
			memcpy(frameData_WiFiScanning[i].SSID, WiFi_APs[i].SSID, 32);
			frameData_WiFiScanning[i].WPA2 = WiFi_APs[i].wpa2;
			frameData_WiFiScanning[i].WPA = WiFi_APs[i].wps;
		}
		
		i = 0;
		for (i = 0; i < 12; i++)
		if (frameData_WiFiScanning[i].SSID[0] != '\0')
		{
			WriteWifiNetDataConvert16(FRAMEDATA_WIFISCANNING_LINE0_BASE + 0x100 * i, &frameData_WiFiScanning[i]);
			osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		}
	}
}
