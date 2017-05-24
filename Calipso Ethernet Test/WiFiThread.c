#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#include "Driver_USART.h"
#include "SDCard.h"
#include "DGUS.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <math.h>
#include "arm_math.h"
#include "WiFiThread.h"
#include "WiFiDriver.h"

#include "SPWF01.h"

extern osThreadId tid_MainThread;

// RTOS Variables
osPoolId pid_WiFi_APs_Pool;
osTimerId tid_WiFiTimer;
osThreadId tid_CalipsoWiFiThread;
osMessageQId qid_WiFiCMDQueue;

void CalipsoWiFiThread (void const *argument);
void WiFiTimer_Callback(void const *arg);

/*----------------------------------------------------------------------------
 *      WiFi Module OS Variables
 *---------------------------------------------------------------------------*/
osPoolDef(WiFi_APs_Pool, 16, WIFI_AP);
osTimerDef (WiFiTimer, WiFiTimer_Callback);
osThreadDef (CalipsoWiFiThread, osPriorityNormal, 1, 0);
osMessageQDef(WiFiCMDQueue, 16, uint32_t);

PWIFI_AP WiFi_APs[16];

char WiFi_NetworkPassword[32];
char WiFi_NetworkSSID[32];

char  PHPSESSID[64];
char  httpBuffer[2048];

/* Private functions ---------------------------------------------------------*/	
void WiFiTimer_Callback(void const *arg) 
{
	for (uint16_t i = 0; i < 16; i++)
	{
		if (WiFi_APs[i]->live == 0)
		{
			memset(WiFi_APs[i], 0, sizeof(WIFI_AP));
			
			for (uint16_t j = i; j < 15; j++)
				memcpy(WiFi_APs[j], WiFi_APs[j+1], sizeof(WIFI_AP));
			
			memset(WiFi_APs[15], 0, sizeof(WIFI_AP));
		}
		else
			WiFi_APs[i]->live--;
	}
}

bool parseHTTP(char* responseHTTP, bool (*header_callback)(char* name, char* value))
{
	char* header[16];
	int pos = 0, i = 0;
	header[pos++] = strtok(responseHTTP, "\r\n");

	while (header[pos-1] != NULL)
		header[pos++] = strtok(NULL, "\r\n");
	
	for (i = 0; i < pos; i++)
	{
		char* name = strtok(header[i], ":");
		char* value = strtok(NULL, "\r\n");

		if (header_callback(name, value))
			return true;
	}

	return false;
}

bool cookie_handler(char* name, char* value)
{
	if (strcmp(name, "Set-Cookie") == 0)
	{
		char* option = strtok(value, "; ");

		do
		{
			char* name = strtok(option, "=");
			char* value = strtok(NULL, ";= ");

			if (strcmp(name, "PHPSESSID") == 0)
			{ 
				strcpy(PHPSESSID, value);
				return true;
			}

			option = strtok(NULL, ";");
		} while (option != NULL);
	}

	return false;
}

bool parseFTP(char* str, uint16_t* resp_code, uint16_t* port)
{
	uint16_t len = strlen(str);
	uint16_t bracket_idx = 0;
	
	while (str[bracket_idx] != '(' && bracket_idx < len) bracket_idx++;
	
	if (str[bracket_idx] == '(' && bracket_idx > 0)
	{
		char* h1 = strtok(&str[bracket_idx+1], " ,)");
		char* h2 = strtok(NULL, " ,)");
		char* h3 = strtok(NULL, " ,)");
		char* h4 = strtok(NULL, " ,)");
		char* p1 = strtok(NULL, " ,)");
		char* p2 = strtok(NULL, " ,)");
		
		*port = atol(p1)*256 + atol(p2);
	}
	else 
		return false;
	
	char* response_code = strtok(str, " ");
	*resp_code = atol(response_code);
	return true;
}

bool downloadFTP(FILE* fp, char* URI)
{
	bool result = false;
	char uri_str[256];
	uint16_t len = 0;
	uint16_t len0 = 0;
	uint16_t len1 = 0;
	uint16_t resp_code = 0;
	uint16_t port = 0;
	static uint32_t foffset = 0;
	
	sprintf(uri_str, "retr %s\n", URI);
	
	if (WiFiConnectionEstabilished)
	{
		int16_t sock_id = socket_connect("innolaser-service.ru", 21);
		int16_t recv_sock_id = -1;
		
		if (sock_id >= 0)
		{			
			int16_t resp_sock_id = -1;
			
			if (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			socket_write(sock_id, "user ftpdevice\n", 15);
			
			if (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			socket_write(sock_id, "pass xejQmMs2#4AN\n", 18);
			
			if (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			socket_write(sock_id, "pasv\n", 5);
			
			if (socket_pending_data(&len, &resp_sock_id, 10000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			if (parseFTP(httpBuffer, &resp_code, &port) && resp_code == 227)
			{
				recv_sock_id = socket_connect("innolaser-service.ru", port);
				
				if (recv_sock_id >= 0)
				{
					socket_write(sock_id, uri_str, strlen(uri_str)); // uri = "retr /etc/crontab\n", 18
					
					osDelay(1000);
					
					len0 = 0;
					len1 = 0;
					while (socket_pending_data(&len, &resp_sock_id, 1000))
						if (resp_sock_id == recv_sock_id)
							//socket_fread(resp_sock_id, fp, len);
							len1 = len;
						else
							//socket_read(resp_sock_id, httpBuffer, len);
							len0 = len;
						
					fseek(fp, foffset, SEEK_SET);
					socket_fread(recv_sock_id, fp, len1);
					foffset += len1;
					socket_read(sock_id, httpBuffer, len0);
					
					len = socket_qpending_data(recv_sock_id);
					if (len > 0)
					{
						fseek(fp, foffset, SEEK_SET);
						socket_fread(recv_sock_id, fp, len);
						foffset += len;
					}
					
					socket_close(recv_sock_id);
					result = true;
				}
			}
			
			while (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
		
			len = socket_qpending_data(sock_id);
			if (len > 0)
				socket_read(sock_id, httpBuffer, len);
			socket_close(sock_id);
		}
	}
	
	return result;
}

void WiFiThread_Idle()
{
	static bool downloaded = false;
	char filename[256];
	int i = 0;
	
	if (!downloaded && WiFiConnectionEstabilished)
	{
		FILE* fp = fopen("CalipsoFirmwareV2REV1_0.bin", "wb");
		
		for (i = 0; i < 53; i++) //105
		{
			sprintf(filename, "/var/www/html/Firmware/CalipsoV2REV1_0/CalipsoFirmwareV2REV1_0.bin.part%d", i);
			while (!downloadFTP(fp, filename));
		}
		fclose(fp);
		
		downloaded = true;
	}
}

void WiFiThread_PublishToServer()
{
	char log[512];
	uint16_t len = 0;
	int16_t sock_id = -1;
	
	if (WiFiConnectionEstabilished)
	{
		static bool authentification = false;
		static bool authentification_start = true;
		
		if (!authentification && authentification_start)
		{
			char* auth_request = "GET http://innolaser-service.ru/service-api/device_auth.php?login=laser3&password=host1234 HTTP/1.1\r\nHost: innolaser-service.ru\r\nConnection: keep-alive\r\nCache-Control: no-cache, no-store, max-age=0\r\n\r\n\r\n";
			sock_id = socket_connect("innolaser-service.ru", 80);
			
			if (sock_id >= 0)
			{
				int16_t resp_sock_id = -1;
				socket_write(sock_id, auth_request, strlen(auth_request));
			
				if (socket_pending_data(&len, &resp_sock_id, 10000))
					socket_read(resp_sock_id, httpBuffer, len);
				
				if (parseHTTP(httpBuffer, cookie_handler))
					authentification = true;
				
				authentification_start = false;
				
				len = socket_qpending_data(sock_id);
				if (len > 0)
					socket_read(sock_id, httpBuffer, len);
				socket_close(sock_id);
			}
		}
		else
		{
			sock_id = socket_connect("innolaser-service.ru", 80);
			
			if (sock_id >= 0)
			{
				int16_t resp_sock_id = -1;
				//sprintf(log, "GET http://innolaser-service.ru/service-api/device_update.php?temperature=%.1f&flow=%.1f	HTTP/1.1\r\nHost: innolaser-service.ru\r\nCookie: PHPSESSID=%s; path=/\r\nPragma: no-cache\r\n\r\n\r\n", temperature, flow1, PHPSESSID);
				sprintf(log, "GET http://innolaser-service.ru/service-api/device_update.php?cooling_level=%d&working=%d&cooling=%d&peltier=%d&temperature=%.1f&flow1=%.1f&flow2=%.1f&frequency=%f&duration=%f&power=%f&cntld=%d&cntss=%d&cntss2=%d&cntlp=%d&cntfl=%d	HTTP/1.1\r\nHost: innolaser-service.ru\r\nConnection: keep-alive\r\nCookie: PHPSESSID=%s; path=/\r\nPragma: no-cache\r\n\r\n\r\n",
								cooling_level, LaserStarted, g_cooling_en, g_peltier_en, temperature, flow1, flow2, frequency_publish, duration_publish, energy_publish, FlushesGlobalLD, FlushesGlobalSS, FlushesGlobalSS2, FlushesGlobalLP, FlushesGlobalFL, PHPSESSID);
				
				socket_write(sock_id, log, strlen(log));
				
				if (socket_pending_data(&len, &resp_sock_id, 10000))
					socket_read(resp_sock_id, httpBuffer, len);
				
				len = socket_qpending_data(sock_id);
				if (len > 0)
					socket_read(sock_id, httpBuffer, len);
				socket_close(sock_id);
			}
		}
	}
}

void WiFiThread_Scan()
{
	bool stop = false;
	uint16_t cnt = 0;
	//WiFi_State_ScanningMode = true;
	
	AsyncSendAT("AT+S.SCAN\r\n");
	
	while (!stop)
	{
		osEvent event = osSignalWait(WIFI_EVENT_RECEIVE_STRING, 1000);
		
		// if not timeout, check for input WIND command
		if (event.status != osEventTimeout)
		{
			uint16_t i = 0;
			
			if (strcmp(buffer_rx, "OK") == 0)
			{
				stop = true;
				continue;
			}
			
			tokenPtr[i]=strtok(buffer_rx, "\t ");
			i++;
	
			while (tokenPtr[i-1] != NULL)
			{
				if (i == 8)
					tokenPtr[i] = strtok(NULL, "'");
				else
					tokenPtr[i] = strtok(NULL, "\t ");
				i++;		
			}
			
			char ssid[32];
			
			if (strcmp(tokenPtr[7], "SSID:") == 0)
			{
				memcpy(ssid, tokenPtr[8], strlen(tokenPtr[8])+1);
				
				for (cnt = 0; cnt < 16; cnt++)
				{
					if (WiFi_APs[cnt]->SSID[0]==0) break;
					if (strcmp(WiFi_APs[cnt]->SSID, ssid)==0) break;
				}
			
				{
					if (strcmp(tokenPtr[7], "SSID:") == 0)
						memcpy(WiFi_APs[cnt]->SSID, tokenPtr[8], strlen(tokenPtr[8])+1);
					
					if (strcmp(tokenPtr[3], "CHAN:") == 0)
						WiFi_APs[cnt]->Channel = atol(tokenPtr[4]);
					
					if (strcmp(tokenPtr[5], "RSSI:") == 0)
						WiFi_APs[cnt]->RSSI		= atol(tokenPtr[6]);
					
					if (strcmp(tokenPtr[9], "CAPS:") == 0)
					{
						WiFi_APs[cnt]->wpa2 = (tokenPtr[11] != NULL);
						WiFi_APs[cnt]->wps = (tokenPtr[12] != NULL);
					}
					WiFi_APs[cnt]->live = 10;
				}
			}
		}
		else
		{
			//WiFi_State_ScanningMode = false;
			osSignalSet(tid_MainThread, WIFI_EVENT_SCANNINGCOMPLETE);
			return;
		}
	}
	
	//WiFi_State_ScanningMode = false;
	osSignalSet(tid_MainThread, WIFI_EVENT_SCANNINGCOMPLETE);
}

void WiFiThread_Link()
{
	char str_ssid[256];
	char str_pass[256];
	int16_t id = -1;
	
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
	
	//Set WiFi SSID
	sprintf(str_ssid, "AT+S.SSIDTXT=%s\r\n", WiFi_NetworkSSID);
	//sprintf(str_ssid, "AT+S.SSIDTXT=ASUS\r\n");
	SendAT(str_ssid);
	
	//Set WiFi network password
	sprintf(str_pass, "AT+S.SCFG=wifi_wpa_psk_text,%s\r\n", WiFi_NetworkPassword);
	//sprintf(str_pass, "AT+S.SCFG=wifi_wpa_psk_text, host1234\r\n");
	SendAT(str_pass);
	
	//Save data & restart WiFi Module
	SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
	SendAT("AT+S.SCFG=wifi_mode,1\r\n");
	SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
	//SendAT("AT+S.SCFG=nv_wifi_macaddr, 00:80:E1:FF:C6:B3\r\n");
	//SendAT("AT+S.SCFG=nv_wifi_macaddr, 00:80:E1:FF:C6:B4\r\n");
	SendAT("AT&W\r\n");
	//SendAT("AT+CFUN=1\r\n");
	
	// Restart WiFi
	AsyncSendAT("AT+CFUN=1\r\n");
	
	// Wait for link up
	if ((id = WaitForWINDCommands(10, 1, (int)WIND_MSG_WIFIUP)) != -1)
	{
		if (id == WIND_MSG_WIFIUP)
		{
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
			osSignalSet(tid_MainThread, WIFI_EVENT_LINKCOMPLETE);
			WiFiConnectionEstabilished = true;
		}
	}
	
	if (WiFi_Deauthentification)
	{
		WiFi_Deauthentification = false;
		//Set WiFi SSID
		SendAT("AT+S.SSIDTXT= \r\n");
		SendAT("AT+S.SCFG=wifi_wpa_psk_text, \r\n");
		SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
		SendAT("AT+S.SCFG=wifi_mode,1\r\n");
		SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
		SendAT("AT&W\r\n");
	
		// Restart WiFi
		AsyncSendAT("AT+CFUN=1\r\n");
	}
}

void CalipsoWiFiThread (void const *argument) {
	int16_t id = -1;
	
	// Init global variables
	WiFi_State_ScanningMode = false;
	WiFiConnectionEstabilished = false;
	RemoteControl = false;
	
	// Reastart WiFi Module
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
	//SendAT("AT&V\r\n");
	SendAT("AT+CFUN=1\r\n");
	
	// Wait for link up
	if ((id = WaitForWINDCommands(10, 1, (int)WIND_MSG_WIFIUP)) != -1)
	{
		if (id == WIND_MSG_WIFIUP)
		{
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
			osSignalSet(tid_MainThread, WIFI_EVENT_LINKCOMPLETE);
			WiFiConnectionEstabilished = true;
		}
	}
	
	if (WiFi_Deauthentification)
	{
		WiFi_Deauthentification = false;
		//Set WiFi SSID
		SendAT("AT+S.SSIDTXT= \r\n");
		SendAT("AT+S.SCFG=wifi_wpa_psk_text, \r\n");
		SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
		SendAT("AT+S.SCFG=wifi_mode,1\r\n");
		SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
		SendAT("AT&W\r\n");
	
		// Restart WiFi
		AsyncSendAT("AT+CFUN=1\r\n");
	}
	
	// Main command loop
  while (1) {
		osEvent event = osMessageGet(qid_WiFiCMDQueue, 1000);
		
		if (event.status == osEventTimeout)
			WiFiThread_PublishToServer(); // Send data to the "innolaser-service.ru" server
		else
		if (event.status == osEventMessage)
		{
			uint32_t cmd = event.value.v;
			switch (cmd)
			{
				case WIFI_CMD_STARTSCANNING:
					WiFiThread_Scan();
					break;
				case WIFI_CMD_STARTLINKING:
					WiFiThread_Link();
					break;
			}
		}
		
    osThreadYield ();
  }
}

int Init_WiFi_Thread (void) {
	
	tid_CalipsoWiFiThread = osThreadCreate (osThread(CalipsoWiFiThread), NULL);
  if (!tid_CalipsoWiFiThread) return(-1);
	
	qid_WiFiCMDQueue = osMessageCreate(osMessageQ(WiFiCMDQueue), NULL);
	if (!qid_WiFiCMDQueue) return(-1);
	
	// Create periodic timer
  tid_WiFiTimer = osTimerCreate(osTimer(WiFiTimer), osTimerPeriodic, NULL);
  if (tid_WiFiTimer != NULL) {    // Periodic timer created
    // start timer with periodic 10ms interval
    osStatus status = osTimerStart (tid_WiFiTimer, 1000);            
    if (status != osOK) {
      // Timer could not be started
    }
  }
	
	pid_WiFi_APs_Pool = osPoolCreate(osPool(WiFi_APs_Pool));
	if (pid_WiFi_APs_Pool != NULL)
		for (uint16_t i = 0; i < 16; i++)
		{
			WiFi_APs[i] = (PWIFI_AP)osPoolCAlloc(pid_WiFi_APs_Pool);
			memset(WiFi_APs[i], 0, sizeof(WIFI_AP));
		}
		
	return Init_WiFiDriver_Thread(tid_CalipsoWiFiThread);
}
