#ifndef WIFITHREAD_H
#define WIFITHREAD_H

#include "WiFiDriver.h"

// WiFi AP scanning strucrure
typedef struct WIFI_AP_STRUCT
{
	uint16_t	Channel;
	char			SSID[32];
	int16_t		RSSI;
	bool wpa2;
	bool wps;
	uint16_t	live;
} WIFI_AP, *PWIFI_AP;

// WiFi AP scanning buffer
extern PWIFI_AP WiFi_APs[16];

extern osThreadId tid_CalipsoWiFiThread;
extern osMessageQId qid_WiFiCMDQueue;

int Init_WiFi_Thread (void);

#endif
