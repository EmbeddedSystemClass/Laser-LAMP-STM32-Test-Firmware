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
extern char WiFi_NetworkSSID[32];

DGUS_WIFISCANNINGLINE frameData_WiFiScanning[12];
uint16_t frameData_NetworkIndex;

void WifiScanningFrame_Init(uint16_t pic_id)
{
	frameData_NetworkIndex = 0;
	
	WriteVariableConvert16(FRAMEDATA_WIFISCANNINGSSID_INDEX, &frameData_NetworkIndex, 2);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	
	SetPicId(FRAME_PICID_SERVICE_WIFISCANNING, g_wDGUSTimeout);
}

void WifiScanningFrame_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t* value;
	ReadVariable(FRAMEDATA_WIFISCANNINGSSID_INDEX, (void**)&value, 6);
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		frameData_NetworkIndex = convert_w(*value);
	else 
		return;
	
	if (frameData_NetworkIndex > 0)
	{		
		memcpy(WiFi_NetworkSSID, WiFi_APs[frameData_NetworkIndex-1]->SSID, 32);
		WriteVariable(FRAMEDATA_WIFIAUTHENTICATION_SSID, WiFi_NetworkSSID, 32);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
		
		frameData_NetworkIndex = 0;
		update = true;
		
		SetPicId(FRAME_PICID_SERVICE_WIFIAUTHENTICATION, g_wDGUSTimeout);
	}
	
	if (update)
	{
		WriteVariableConvert16(FRAMEDATA_WIFISCANNINGSSID_INDEX, &frameData_NetworkIndex, 2);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	static uint16_t cnt = 0;
	
	if ((cnt % 10) == 0)
	{
		// Send message to wifi thread to start scanning wifi network
		osStatus status = osMessagePut(qid_WiFiCMDQueue, WIFI_CMD_STARTSCANNING, 3000);
		if (status != osEventTimeout)
		{
			// Wait for wifi network scanning complete
			osEvent event = osSignalWait(WIFI_EVENT_SCANNINGCOMPLETE, 3000);
			if (event.status != osEventTimeout)
			{
				// Update network list for DGUS display
				uint16_t i = 0;
				for (i = 0; i < 12; i++)
				{
					frameData_WiFiScanning[i].channel = WiFi_APs[i]->Channel;
					frameData_WiFiScanning[i].RSSI = WiFi_APs[i]->RSSI;
					memcpy(frameData_WiFiScanning[i].SSID, WiFi_APs[i]->SSID, 32);
					frameData_WiFiScanning[i].WPA2 = WiFi_APs[i]->wpa2;
					frameData_WiFiScanning[i].WPA = WiFi_APs[i]->wps;
				}
				
				i = 0;
				for (i = 0; i < 12; i++)
				{
					WriteWifiNetDataConvert16(FRAMEDATA_WIFISCANNING_LINE0_BASE + 0x100 * i, &frameData_WiFiScanning[i]);
					osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
				}
			}
		}
	}
	cnt++;
}

void WiFiLinkFrame_Process()
{
	// Send message to wifi thread to start scanning wifi network
	osStatus status = osMessagePut(qid_WiFiCMDQueue, WIFI_CMD_STARTLINKING, 3000);
	if (status != osEventTimeout)
	{
		// Wait for wifi network scanning complete
		osEvent event = osSignalWait(WIFI_EVENT_LINKCOMPLETE, 10000);
		if (event.status != osEventTimeout)
		{
			SetPicId(FRAME_PICID_MAINMENU, g_wDGUSTimeout);
		}
		else
			SetPicId(FRAME_PICID_SERVICE_WIFISCANNING, g_wDGUSTimeout);
	}
	else
		SetPicId(FRAME_PICID_SERVICE_WIFISCANNING, g_wDGUSTimeout);
}
