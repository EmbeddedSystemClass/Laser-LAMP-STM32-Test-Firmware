
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"
#include "SPWF01.h"
#include "DGUS.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <math.h>
#include "arm_math.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
extern ARM_DRIVER_USART Driver_USART3;

#define WIFI_EVENT_RECEIVE_COMPLETED	0x004
#define WIFI_EVENT_SEND_COMPLETED			0x008
#define WIFI_EVENT_RECEIVE_TIMEOUT		0x010

#define WIFI_EVENT_RECEIVE_STRING			0x020
#define WIFI_EVENT_RECEIVE_WIND				0x040
#define WIFI_EVENT_TEMPERATURE_UPDATE	0x080

#define WIFI_EVENT_REMOTECONTROL			0x100

#define FRAME_SIZE	64
#define BUFFER_SIZE 2048
#define BUFFER_MASK 0x7ff

bool  WiFi_State_CommandMode = true;

char  ATRCV[BUFFER_SIZE];
char  token[256];
char* tokenPtr[10];
char  buffer_rx[256];
char* buffer_tx;
uint16_t frame_offset;
uint32_t frame_pos;
uint16_t frame_read;
uint32_t frame_write;
 
void WiFiThread (void const *argument);
void UserWiFiThread (void const *argument);
osThreadId tid_WiFiThread;
osThreadId tid_UserWiFiThread;
osThreadDef (WiFiThread, osPriorityNormal, 1, 0);
osThreadDef (UserWiFiThread, osPriorityNormal, 1, 0);

/* Private functions ---------------------------------------------------------*/
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

int Init_WiFi_Thread (void) {

  tid_WiFiThread = osThreadCreate (osThread(WiFiThread), NULL);
  if (!tid_WiFiThread) return(-1);
	
	tid_UserWiFiThread = osThreadCreate (osThread(UserWiFiThread), NULL);
  if (!tid_UserWiFiThread) return(-1);
	
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
	while (frame_read <= frame_write)
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
	osThreadYield ();
	
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
	Driver_USART3.Receive(ATRCV, FRAME_SIZE);
	
  while (1) {
    ; // Insert thread code here...
		pos = 0;
		while (!GetStringFromWiFi(buffer_rx, (uint16_t*)&pos))
			osThreadYield ();
		buffer_rx[pos] = '\0';
		
		if ((pos > 4) && (buffer_rx[0] == '+') &&	(buffer_rx[1] == 'W') && (buffer_rx[2] == 'I') &&	(buffer_rx[3] == 'N') && (buffer_rx[4] == 'D'))
		{
			i = 0;
			tokenPtr[i]=strtok(buffer_rx, ":");
			i++;

			while (tokenPtr[i-1] != NULL)
			{ 
				tokenPtr[i] = strtok(NULL, ":");
				i++;		
			}
			
			uint16_t id = atol(tokenPtr[1]);
			if (id == WIND_MSG_COMMAND_MODE)
				WiFi_State_CommandMode = true;
			if (id == WIND_MSG_DATA_MODE)
				WiFi_State_CommandMode = false;
			
			osSignalSet(tid_UserWiFiThread, WIFI_EVENT_RECEIVE_WIND | WIFI_EVENT_RECEIVE_STRING);
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
	Driver_USART3.Send(str, strlen(str));
	osThreadYield (); // Wait "OK"
	while (osSignalWait(WIFI_EVENT_RECEIVE_STRING, 1000).status != osEventTimeout)
	{
		if (strcmp(strtok(buffer_rx, ":\n\r"), "ERROR") == 0)
			return false;
		if (strcmp(buffer_rx, "OK") == 0)
			return true;
	}
	return false;
}

void AsyncSendAT(char* str)
{
	Driver_USART3.Send(str, strlen(str));
}

int16_t GetID()
{
	if (osSignalWait(WIFI_EVENT_RECEIVE_STRING, 1000).status != osEventTimeout)
	{
		if (strcmp(strtok(buffer_rx, ":\n\r"), "ID") == 0)
			return atol(strtok(NULL, ":\n\r"));
	}
	return -1;
}

bool WaitOK()
{
	while (osSignalWait(WIFI_EVENT_RECEIVE_STRING, 1000).status != osEventTimeout)
	{
		if (strcmp(strtok(buffer_rx, ":\n\r"), "ERROR") == 0)
			return false;
		if (strcmp(buffer_rx, "OK") == 0)
			return true;
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

void UserWiFiThread (void const *argument) {
	RemoteControl = false;
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
	//if (!SendAT(AT)) return;

	/*// STA mode
	SendAT("AT+S.SSIDTXT=ASUS\r\n");
	SendAT("AT+S.SCFG=wifi_wpa_psk_text,host1234\r\n");
	SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
	SendAT("AT+S.SCFG=wifi_mode,1\r\n");
	SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
	SendAT("AT&W\r\n");
	SendAT("AT+CFUN=1\r\n");
	
	// Mini AP mode
	SendAT("AT+S.SSIDTXT=Calipso\r\n");
	SendAT("AT+S.SCFG=wifi_priv_mode,0\r\n");
	SendAT("AT+S.SCFG=wifi_mode,3\r\n");
	SendAT("AT+S.SCFG=ip_ipaddr,192.168.1.10\r\n");
	SendAT("AT+S.SCFG=ip_gw,192.168.1.1\r\n");
	SendAT("AT+S.SCFG=ip_dns,192.168.1.1\r\n");
	SendAT("AT+S.SCFG=ip_netmask,255.255.255.0\r\n");
	SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
	SendAT("AT&W\r\n");
	SendAT("AT+CFUN=1\r\n");
	
	// STA mode
	SendAT("AT+S.SSIDTXT=ELTEX-40C0\r\n");
	SendAT("AT+S.SCFG=wifi_wpa_psk_text,GP21167802\r\n");
	SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
	SendAT("AT+S.SCFG=wifi_mode,1\r\n");
	SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
	SendAT("AT&W\r\n");*/
	
	// Restart WiFi
	AsyncSendAT("AT+CFUN=1\r\n");
	
	// Wait for link up
	WaitForWINDCommands(10, 1, (int)WIND_MSG_WIFIUP);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
	
	/*
	// Connect to server
	SendAT("AT+S.SOCKON=109.123.137.154,32000,t,ind\r\n");
	GetID();
	WaitOK();
	
	// Send to server
	SendAT("AT+S.SOCKW=0,15\r\n");
	
	// Send data to client
	Driver_USART3.Send("Hello client!\r\n", 15);
	
	// Wait OK
	WaitOK();
	
	// Send to server
	SendAT("AT+S.SOCKW=0,14\r\n");
	
	// Send data to client
	Driver_USART3.Send("How are you!\r\n", 14);
	
	// Wait OK
	WaitOK();*/
		
	// Listening on port 32000 using TCP
	SendAT("AT+S.SOCKD=32000,t\r\n");
	
	// Wait for data mode input
	WaitForWINDCommands(10, 1, (int)WIND_MSG_DATA_MODE);
	
  while (1) {
    char str[256];
		if (!WiFi_State_CommandMode)
		{
			RemoteControl = true;
			__SOLIDSTATELASER_HVON();
		
			osEvent event = osSignalWait(0, 1000);
			
			// Remote control
			if ((event.value.signals & WIFI_EVENT_REMOTECONTROL) != 0)
			{
				uint16_t las_id = atol(tokenPtr[1]);
				
				if (las_id == 0)
				{
					if (strcmp(tokenPtr[2], "START") == 0)
					{
						LampControlPulseStart();
					}
					if (strcmp(tokenPtr[2], "STOP") == 0)
					{
						LampControlPulseStop();
					}
					if (strcmp(tokenPtr[2], "SIMMER") == 0)
						if (strcmp(tokenPtr[3], "OFF"))
							__SOLIDSTATELASER_SIMMEROFF();
					if (strcmp(tokenPtr[2], "SIMMER") == 0)
						if (strcmp(tokenPtr[3], "ON"))
							__SOLIDSTATELASER_SIMMERON();
					if (strcmp(tokenPtr[2], "DURATION") == 0)
					{
						uint16_t value = atol(tokenPtr[3]);
						LampSetPulseDuration(value);
					}
					if (strcmp(tokenPtr[2], "FREQUENCY") == 0)
					{
						uint16_t value = atol(tokenPtr[3]);
						LampSetPulseFrequency(value);
					}
					if (strcmp(tokenPtr[2], "ENERGY") == 0)
					{
						uint16_t value = atol(tokenPtr[3]);
					}
				}
			}
		
			// Temperature update
			//if ((event.value.signals & WIFI_EVENT_TEMPERATURE_UPDATE) != 0)
			{
				// Send data to client
				sprintf(str, "%f C\n", temperature);
				Driver_USART3.Send(str, strlen(str));
			}
		}
		else
		{
			if (RemoteControl)
			{
				RemoteControl = false;
				__SOLIDSTATELASER_HVOFF();
			}
			// Wait for data mode input
			WaitForWINDCommands(1, 1, (int)WIND_MSG_DATA_MODE);
		}
		
    osThreadYield ();
  }
}
