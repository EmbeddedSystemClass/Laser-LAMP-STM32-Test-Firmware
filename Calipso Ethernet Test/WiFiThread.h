#ifndef WIFITHREAD_H
#define WIFITHREAD_H

#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os.h"

#define WIFI_EVENT_RECEIVE_COMPLETED	0x004
#define WIFI_EVENT_SEND_COMPLETED			0x008
#define WIFI_EVENT_RECEIVE_TIMEOUT		0x010

#define WIFI_EVENT_RECEIVE_STRING			0x020
#define WIFI_EVENT_RECEIVE_WIND				0x040
#define WIFI_EVENT_TEMPERATURE_UPDATE	0x080

#define WIFI_EVENT_REMOTECONTROL			0x100

// WiFi CMD
#define WIFI_CMD_STARTSCANNING				1

#define FRAME_SIZE	64
#define BUFFER_SIZE 2048
#define BUFFER_MASK 0x7ff

extern bool WiFi_State_CommandMode;
extern bool WiFi_State_ScanningMode;

extern osThreadId tid_WiFiThread;
extern osThreadId tid_UserWiFiThread;
extern osMessageQId qid_WiFiCMDQueue;

typedef struct WIFI_AP_STRUCT
{
	uint16_t	Channel;
	char			SSID[32];
	int16_t		RSSI;
	bool wpa2;
	bool wps;
} WIFI_AP, *PWIFI_AP;

extern WIFI_AP WiFi_APs[12];

#endif
