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
#include "WiFiDriver.h"

#include "SPWF01.h"
 
extern ARM_DRIVER_USART Driver_USART3;

/*----------------------------------------------------------------------------
 *      WiFi Module Global Variables
 *---------------------------------------------------------------------------*/
bool WiFi_State_CommandMode = true;
bool WiFi_State_ScanningMode = false;
bool WiFi_SocketClosed[8] = {false, false, false, false, false, false, false, false};
bool WiFi_SocketConnected[8] = {false, false, false, false, false, false, false, false};
//bool WiFi_ConnectionEstabilished[8] = {false, false, false, false, false, false, false, false};
bool WiFi_OK_Received = false;
bool WiFi_ERROR_Received = false;
bool WiFi_PendingData = false;

uint16_t WiFi_PendingDataSize = 0;

/*----------------------------------------------------------------------------
 *      WiFi Module Config Variables
 *---------------------------------------------------------------------------*/
uint16_t WIFi_ConnectionTimeout = 15000;
uint16_t WIFi_RequestTimeout = 15000;

PWIFI_AP WiFi_APs[16];

// RTOS Variables
osThreadId tid_WiFiThread;
osThreadId tid_UserWiFiThread;

/*----------------------------------------------------------------------------
 *      WiFi Module Local Variables
 *---------------------------------------------------------------------------*/
char  ATRCV[BUFFER_SIZE];
char  token[256];
char* tokenPtr[32];
char  buffer_rx[256];
char* buffer_tx;
uint32_t frame_offset;
uint32_t frame_pos;
uint32_t frame_read;
uint32_t frame_write;
 
void WiFiThread (void const *argument);

/*----------------------------------------------------------------------------
 *      WiFi Module OS Variables
 *---------------------------------------------------------------------------*/
osThreadDef (WiFiThread, osPriorityNormal, 1, 0);

void WIFI_USART_callback(uint32_t event)
{
		uint16_t num = 0;
	
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
				/* Frame received, set next */
				frame_offset += FRAME_SIZE;
				frame_offset &= BUFFER_MASK;
		
				/* Continue receiving */
				Driver_USART3.Receive(&ATRCV[frame_offset], FRAME_SIZE);
				frame_pos += FRAME_SIZE;
				frame_write = frame_pos;
		
				/* Success: Wakeup Thread */
				osSignalSet(tid_WiFiThread, WIFI_EVENT_RECEIVE_COMPLETED);
        break;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
				/* Success: Wakeup Thread */
				break;
    case ARM_USART_EVENT_SEND_COMPLETE:
				/* Success: Wakeup Thread */
				osSignalSet(tid_WiFiThread, WIFI_EVENT_SEND_COMPLETED);
				break;
    case ARM_USART_EVENT_TX_COMPLETE:
        /* Success: Wakeup Thread */
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
				num = Driver_USART3.GetRxCount();	
				frame_write = frame_pos + num;
		
				/* Success: Wakeup Thread */
				osSignalSet(tid_WiFiThread, WIFI_EVENT_RECEIVE_TIMEOUT);
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
        //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}

int Init_WiFiDriver_Thread (osThreadId userThread) {

	tid_UserWiFiThread = userThread;
  tid_WiFiThread = osThreadCreate (osThread(WiFiThread), NULL);
  if (!tid_WiFiThread) return(-1);
	
	//Initialize the USART driver 
  Driver_USART3.Initialize(WIFI_USART_callback);
  //Power up the USART peripheral 
  Driver_USART3.PowerControl(ARM_POWER_FULL);
	
  //Configure the USART to 115200 Bits/sec 
  Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS |
												ARM_USART_DATA_BITS_8 |
												ARM_USART_PARITY_NONE |
												ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_RTS_CTS, 115200);
	
	// Enable Receiver and Transmitter lines 
  Driver_USART3.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART3.Control (ARM_USART_CONTROL_RX, 1);
	
	frame_offset = 0;
	frame_pos = 0;
	frame_write = FRAME_SIZE;
	frame_read = 0;
  
  return(0);
}

bool SkeepATStart()
{
	while (frame_read < frame_write)
	{
		if ((ATRCV[frame_read & BUFFER_MASK] != '\n') && (ATRCV[frame_read & BUFFER_MASK] != '\r'))
			return true;
		frame_read++;
	}
	return false;
}

bool CopyWhileNotEnd(char* dst, uint16_t* pos)
{
	while (frame_read < frame_write)
	{
		if ((ATRCV[frame_read & BUFFER_MASK] == '\n') || (ATRCV[frame_read & BUFFER_MASK] == '\r'))
		{
			frame_read++;
			return true;
		}
		dst[(*pos)++] = ATRCV[frame_read & BUFFER_MASK];
		frame_read++;
	}
	return false;
}

bool GetStringFromWiFi(char* buffer, uint16_t *pos)
{	
	// Wait for WiFi ready
	osEvent event = osSignalWait(WIFI_EVENT_RECEIVE_COMPLETED | WIFI_EVENT_RECEIVE_TIMEOUT, 100);
	
	// Skip chars before string
	if (!SkeepATStart()) return false;
	
	// Copy string
	if (!CopyWhileNotEnd(buffer, pos)) return false;
	
	return true;
}

void WiFiThread (void const *argument) {
	volatile uint16_t pos = 0;
	uint16_t i;
	char* str = NULL;
	//start_slog(datetime);
	Driver_USART3.Receive(ATRCV, FRAME_SIZE);
	
  while (1) {
    ; // Insert thread code here...
		pos = 0;
		while (!GetStringFromWiFi(buffer_rx, (uint16_t*)&pos))
			osThreadYield ();
		buffer_rx[pos] = '\0';
		log_wifi(datetime, buffer_rx);
		
		if ((pos > 2) && ((str = strstr(buffer_rx, "OK")) != 0))
			WiFi_OK_Received = true;
		
		if ((pos > 2) && ((str = strstr(buffer_rx, "ERROR")) != 0))
			WiFi_ERROR_Received = true;
		
		str = buffer_rx;
		
		if ((pos > 4) && ((str = strstr(str, "+WIND")) != 0))
		{
			i = 0;
			tokenPtr[i]=strtok(str, ":");
			i++;
			str += 1;
	
			while (tokenPtr[i-1] != NULL)
			{ 
				tokenPtr[i] = strtok(NULL, ":");
				i++;		
			}
			
			// Get id of WIND message
			uint16_t id = atol(tokenPtr[1]);
			
			// Command/data mode
			if (id == WIND_MSG_COMMAND_MODE)		WiFi_State_CommandMode = true;
			if (id == WIND_MSG_DATA_MODE)				WiFi_State_CommandMode = false;
			
			// Connection estabilished
			if (id == WIND_MSG_WIFIUP)
			{
				WiFiConnectionEstabilished = true;
				ip_addr_updated = true;
				memcpy(ip_addr, tokenPtr[3], strlen(tokenPtr[3])+1);
			}
			
			// Socket closed
			if (id == WIND_MSG_SOCKET_CLOSED)
			{
				uint16_t socket_id = atol(tokenPtr[3]);
				WiFi_SocketConnected[socket_id] = false;
				WiFi_SocketClosed[socket_id] = true;
			}
			
			if (id == WIND_MSG_PENDING_DATA)
			{
				WiFi_PendingData = true;
				WiFi_PendingDataSize = atol(tokenPtr[4]);
			}
			
			// if error occured
			if (id == WIND_MSG_HEAPTOOSMALL	|| id == WIND_MSG_HWR_FAILURE 		|| id == WIND_MSG_WATCHDOG_TERMINATING	|| id == WIND_MSG_SYSCLK_FAIL ||
					id == WIND_MSG_HARDFAULT		|| id == WIND_MSG_STACK_OVERFLOW	|| id == WIND_MSG_MALLOC_FAILED 				|| id == WIND_MSG_ERROR || 
					id == WIND_MSG_POWERSAVE_FAILED || id == WIND_MSG_POWERON) 
			{
				// Error handle here
				memset(WiFi_SocketConnected, 0, sizeof(WiFi_SocketConnected));
				memset(WiFi_SocketClosed, 0xff, sizeof(WiFi_SocketClosed));
				//memset(WiFi_ConnectionEstabilished, 0, sizeof(WiFi_ConnectionEstabilished));
				
				WiFiConnectionEstabilished = false;
				/*WiFi_SocketConnected = false;
				WiFi_SocketClosed = true;*/
			}
			
			osSignalSet(tid_UserWiFiThread, WIFI_EVENT_RECEIVE_WIND | WIFI_EVENT_RECEIVE_STRING);
			
			osThreadYield ();
		}
		else
		if ((pos > 3) && (buffer_rx[0] == 'L') &&	(buffer_rx[1] == 'A') && (buffer_rx[2] == 'S'))
		{
			i = 0;
			tokenPtr[i]=strtok(buffer_rx, ":=");
			i++;

			while (tokenPtr[i-1] != NULL)
			{ 
				tokenPtr[i] = strtok(NULL, ":=");
				i++;		
			}
			
			osSignalSet(tid_UserWiFiThread, WIFI_EVENT_RECEIVE_STRING | WIFI_EVENT_REMOTECONTROL);
		}
		else
			osSignalSet(tid_UserWiFiThread, WIFI_EVENT_RECEIVE_STRING);
		
    osThreadYield ();
  }
}

bool SendAT(char* str)
{
	WiFi_OK_Received = false;
	WiFi_ERROR_Received = false;
	
	Driver_USART3.Send(str, strlen(str));
	if (WiFi_OK_Received) return true;
	if (WiFi_ERROR_Received) return true;
	
	while (osSignalWait(WIFI_EVENT_RECEIVE_STRING, 1000).status != osEventTimeout)
	{
		if (WiFi_OK_Received) return true;
		if (WiFi_ERROR_Received) return true;
		
		if (strcmp(strtok(buffer_rx, ":\n\r"), "ERROR") == 0)
			return false;
		if (strcmp(buffer_rx, "OK") == 0)
			return true;
		osThreadYield (); // Wait "OK"
	}
	return false;
}

void AsyncSendAT(char* str)
{
	WiFi_OK_Received = false;
	WiFi_ERROR_Received = false;
	
	Driver_USART3.Send(str, strlen(str));
}

int16_t GetID(uint32_t timeout)
{
	while(osSignalWait(WIFI_EVENT_RECEIVE_STRING, timeout).status != osEventTimeout)
	{
		if (strcmp(strtok(buffer_rx, " :\n\r"), "ID") == 0)
			return atol(strtok(NULL, " :\n\r"));
	}
	return -1;
}

char* GetResponsePtr()
{
	return &ATRCV[(frame_read + 1) & BUFFER_MASK];
}

bool WaitOK(uint32_t timeout)
{
	if (WiFi_OK_Received) return true;
	if (WiFi_ERROR_Received) return true;
	
	while (osSignalWait(WIFI_EVENT_RECEIVE_STRING, timeout).status != osEventTimeout)
	{
		if (WiFi_OK_Received) return true;
		if (WiFi_ERROR_Received) return true;
		
		if (strstr(buffer_rx, "ERROR") != 0)
			return false;
		if (strstr(buffer_rx, "OK") != 0)
			return true;
		
		osThreadYield (); // Wait "OK"
	}
	return false;
}

int16_t WaitForWINDCommands(uint16_t timeout, uint16_t argc, ...)
{
	va_list ap;
	for (int i = 0; i < timeout; i++)
	{
		osEvent event = osSignalWait(WIFI_EVENT_RECEIVE_WIND, 3000);
		
		// if not timeout, check for input WIND command
		if (event.status != osEventTimeout)
		{			
			uint16_t id = atol(tokenPtr[1]);
			
			va_start(ap, argc);
			for (int j = 0; j < argc; j++)
					if (id == va_arg(ap, int))
					{
						va_end(ap);
						return id;
					}
					
			va_end(ap);
		}
		
		osThreadYield ();
	}
	
	return -1;
}

int16_t socket_connect(char* name, uint16_t port)
{
	char str[256];
	int16_t id = -1;
	
	// Connect to the host with 'name'
	sprintf(str, "AT+S.SOCKON=%s,%d,t,ind\r\n", name, port);
	AsyncSendAT(str);		
	
	// Connection id
	id = GetID(WIFi_ConnectionTimeout);		
	if (id < 0) return -1;
	
	// Wait for complete operation
	if (!WaitOK(WIFi_ConnectionTimeout)) return id; // Wait 3 seconds for connection
	WiFi_SocketConnected[id] = true;
	
	// return connection id
	return id;
}

void socket_write(uint16_t id, char* buffer, uint16_t len)
{
	char str[256];
	sprintf(str, "AT+S.SOCKW=%d,%d\r\n", id, len);	
	AsyncSendAT(strcat(str, buffer));
}
