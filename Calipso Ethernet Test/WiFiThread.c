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

void WiFiThread_Idle()
{
	uint16_t len = 0;
	uint16_t resp_code = 0;
	uint16_t port = 0;
	
	if (WiFiConnectionEstabilished)
	{
		int16_t sock_id = socket_connect("innolaser-service.ru", 21);
		int16_t recv_sock_id = -1;
		
		if (sock_id >= 0)
		{			
			int16_t resp_sock_id = -1;
			
			if (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			socket_write(sock_id, "user ftpdevice\n", 16);
			
			if (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			socket_write(sock_id, "pass xejQmMs2#4AN\n", 19);
			
			if (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			socket_write(sock_id, "pasv\n", 6);
			
			if (socket_pending_data(&len, &resp_sock_id, 10000))
				socket_read(resp_sock_id, httpBuffer, len);
			
			if (parseFTP(httpBuffer, &resp_code, &port) && resp_code == 227)
			{
				recv_sock_id = socket_connect("innolaser-service.ru", port);
				
				/*if (socket_pending_data(&len, &resp_sock_id, 1000))
					socket_read(resp_sock_id, httpBuffer, len);*/
				
				socket_write(sock_id, "retr /etc/crontab\n", 19);
				
				while (socket_pending_data(&len, &resp_sock_id, 1000))
					socket_read(resp_sock_id, httpBuffer, len);
				
				len = socket_qpending_data(recv_sock_id);
				if (len > 0)
					socket_read(recv_sock_id, httpBuffer, len);
				
				socket_close(recv_sock_id);
			}
			
			while (socket_pending_data(&len, &resp_sock_id, 1000))
				socket_read(resp_sock_id, httpBuffer, len);
		
			len = socket_qpending_data(sock_id);
			if (len > 0)
				socket_read(sock_id, httpBuffer, len);
			socket_close(sock_id);
		}
	}
}

void WiFiThread_PublishToServer()
{	
	char str[256];
	char log[512];
	static int16_t id = -1;
	
	if (WiFiConnectionEstabilished)
	{
		static bool authentification = false;
		static bool authentification_start = true;
		
		if (!authentification && authentification_start)
		{			
			char* auth_request = "GET http://innolaser-service.ru:3000/device_auth.php?login=vlad&password=ovchin_1988 HTTP/1.1\r\nHost: innolaser-service.ru\r\nCache-Control: no-cache, no-store, max-age=0\r\n\r\n\r\n";
			uint16_t len = strlen(auth_request);
			
			// Connect to the server
			//if (!WiFi_SocketConnected)
			{
				AsyncSendAT("AT+S.SOCKON=innolaser-service.ru,3000,t,ind\r\n");		
				id = GetID(WIFi_ConnectionTimeout);		
				if (id < 0) return;
				if (!WaitOK(WIFi_ConnectionTimeout)) return; // Wait 3 seconds for connection
				WiFi_SocketConnected[id] = true;
			}
			
			// Send HTTP request
			sprintf(str, "AT+S.SOCKW=%d,%d\r\n", id, len);	
			AsyncSendAT(strcat(str, auth_request));

			if (WaitOK(WIFi_RequestTimeout))
			{
				WaitForWINDCommands(10, 1, (int)WIND_MSG_PENDING_DATA);
				len = atol(tokenPtr[4]);
				
				// Read response
				sprintf(str, "AT+S.SOCKR=%d,%d\r\n", id, len);	SendAT(str);
				
				//WaitForWINDCommands(10, 1, (int)WIND_MSG_SOCKET_CLOSED);
				
				char* response = GetResponsePtr();//&ATRCV[(frame_read + 1) & BUFFER_MASK];
				
				memcpy(httpBuffer, response, len);
				
				WaitOK(1000);
				
				if (parseHTTP(httpBuffer, cookie_handler))
					authentification = true;
				
				authentification_start = false;
			}
			
			// Close socket
			//if (WiFi_SocketClosed)
			{
				WiFi_SocketConnected[id] = false;
				WiFi_SocketClosed[id] = false;
				sprintf(str, "AT+S.SOCKC=%d\r\n", id);
				SendAT(str);
			}
		}
		else
		{					
			// Connect to the server
			//if (!WiFi_SocketConnected)
			{
				log_wifi(datetime, "AT+S.SOCKON=innolaser-service.ru,3000,t,ind");
				AsyncSendAT("AT+S.SOCKON=innolaser-service.ru,3000,t,ind\r\n");
				id = GetID(WIFi_ConnectionTimeout);
				if (id < 0) 
				{
					log_wifi(datetime, "ERROR: Invalid socket id");
					return;
				}
				if (!WaitOK(WIFi_ConnectionTimeout)) // Wait 3 seconds for connection
				{
					log_wifi(datetime, "ERROR: Connection failed");
					return;
				}
				WiFi_SocketConnected[id] = true;
			}
			
			// Log socket id;
			sprintf(str, " ID response : %d", id);
			log_wifi(datetime, str);
			
			sprintf(log, "GET http://innolaser-service.ru:3000/device_update.php?cooling_level=%d&working=%d&cooling=%d&peltier=%d&temperature=%.1f&flow=%.1f&frequency=%d&power=%d	HTTP/1.1\r\nHost: innolaser-service.ru\r\nCookie: PHPSESSID=%s; path=/\r\nPragma: no-cache\r\n\r\n\r\n", 6, 1, 1, 1, temperature, flow1, 10, 100, PHPSESSID);
			uint16_t len = strlen(log);
			
			// Send HTTP GET			
			WiFi_PendingData[0] = false;
			sprintf(str, "AT+S.SOCKW=%d,%d\r", id, len);
			log_wifi(datetime, "AT+S.SOCKW + HTTP GET REQUEST");
			AsyncSendAT(strcat(str, log));
			
			WiFi_SocketClosed[id] = false;
			if (WaitOK(WIFi_RequestTimeout))
			{
				// Wait for pending data
				if (!WiFi_PendingData[0])
					WaitForWINDCommands(30, 1, (int)WIND_MSG_PENDING_DATA);
				len = WiFi_PendingDataSize[0];
				sprintf(str, "Pending data : %d", len);
				log_wifi(datetime, str);
				
				// Read response
				sprintf(str, "AT+S.SOCKR=%d,%d\r\n", id, len);	
				log_wifi(datetime, "AT+S.SOCKR : ");
				if (!WiFi_SocketClosed[id])
					SendAT(str);
				
				//WaitForWINDCommands(10, 1, (int)WIND_MSG_SOCKET_CLOSED);
			}
			else
				log_wifi(datetime, "ERROR: Failed to receive HTTP response");
			
			// Close socket
			//if (WiFi_SocketClosed)
			{
				WiFi_SocketConnected[id] = false;
				WiFi_SocketClosed[id] = false;
				sprintf(str, "AT+S.SOCKC=%d\r\n", id);
				log_wifi(datetime, "AT+S.SOCKC : ");
				SendAT(str);
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
	if (WaitForWINDCommands(10, 1, (int)WIND_MSG_WIFIUP))
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
		osSignalSet(tid_MainThread, WIFI_EVENT_LINKCOMPLETE);
		WiFiConnectionEstabilished = true;
	}
}

void CalipsoWiFiThread (void const *argument) {
	// Init global variables
	WiFi_State_ScanningMode = false;
	WiFiConnectionEstabilished = false;
	RemoteControl = false;
	
	// Start wifi logging
	//start_wifi(datetime);
	
	//Set WiFi SSID
	SendAT("AT+S.SSIDTXT=ASUS\r\n");
	SendAT("AT+S.SCFG=wifi_wpa_psk_text,host1234\r\n");
	SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
	SendAT("AT+S.SCFG=wifi_mode,1\r\n");
	SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
	SendAT("AT&W\r\n");
	
	// Restart WiFi
	AsyncSendAT("AT+CFUN=1\r\n");
	
	// Reastart WiFi Module
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
	//SendAT("AT&V\r\n");
	SendAT("AT+CFUN=1\r\n");
	
	// Main command loop
  while (1) {
		osEvent event = osMessageGet(qid_WiFiCMDQueue, 3000);
		
		if (event.status == osEventTimeout)
			WiFiThread_Idle(); // Send data to the "innolaser-service.ru" server
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
