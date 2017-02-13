#ifndef WIFITHREAD_H
#define WIFITHREAD_H

#include "WiFiDriver.h"

extern osThreadId tid_CalipsoWiFiThread;
extern osMessageQId qid_WiFiCMDQueue;

int Init_WiFi_Thread (void);

#endif
