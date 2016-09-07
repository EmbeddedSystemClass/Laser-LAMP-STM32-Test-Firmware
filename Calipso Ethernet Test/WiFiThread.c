
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_USART.h"

#include <string.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
extern ARM_DRIVER_USART Driver_USART3;

#define WIFI_EVENT_RECEIVE_COMPLETED	0x04
#define WIFI_EVENT_SEND_COMPLETED			0x08
#define WIFI_EVENT_RECEIVE_TIMEOUT		0x10

#define WIFI_EVENT_RECEIVE_STRING			0x20

#define FRAME_SIZE	64
#define BUFFER_SIZE 2048
#define BUFFER_MASK 0x7ff

char  ATRCV[BUFFER_SIZE];
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

// AT command list
char AT                [] = "AT\r\n\0";
char AT_CFUN           [] = "AT+CFUN\r\n\0";
char AT_S_HELP         [] = "AT+S.HELP\r\n\0";
char AT_S_GCFG         [] = "AT+S.GCFG";
char AT_S_SCFG         [] = "AT+S.SCFG\r\n\0";
char AT_S_SSIDTXT      [] = "AT+S.SSIDTXT\r\n\0";
char AT_V              [] = "AT&V\r\n\0";
char AT_F              [] = "AT&F\r\n\0";
char AT_W              [] = "AT&W\r\n\0";
char AT_S_NVW          [] = "AT+S.NVW\r\n\0";
char AT_S_STS          [] = "AT+S.STS\r\n\0";
char AT_S_PEERS        [] = "AT+S.PEERS\r\n\0";
char AT_S_PING         [] = "AT+S.PING\r\n\0";
char AT_S_SOCKON       [] = "AT+S.SOCKON\r\n\0";
char AT_S_SOCKOS       [] = "AT+S.SOCKOS\r\n\0";
char AT_S_SOCKW        [] = "AT+S.SOCKW\r\n\0";
char AT_S_SOCKQ        [] = "AT+S.SOCKQ\r\n\0";
char AT_S_SOCKR        [] = "AT+S.SOCKR\r\n\0";
char AT_S_SOCKC        [] = "AT+S.SOCKC\r\n\0";
char AT_S_SOCKD        [] = "AT+S.SOCKD\r\n\0";
char AT_S_             [] = "AT+S.\r\n\0";
char AT_S_HTTPGET      [] = "AT+S.HTTPGET\r\n\0";
char AT_S_HTTPPOST     [] = "AT+S.HTTPPOST\r\n\0";
char AT_S_FSC          [] = "AT+S.FSC\r\n\0";
char AT_S_FSA          [] = "AT+S.FSA\r\n\0";
char AT_S_FSD          [] = "AT+S.FSD\r\n\0";
char AT_S_FSL          [] = "AT+S.FSL\r\n\0";
char AT_S_FSP          [] = "AT+S.FSP\r\n\0";
char AT_S_MFGTEST      [] = "AT+S.MFGTEST\r\n\0";
char AT_S_PEMDATA      [] = "AT+S.PEMDATA\r\n\0";
char AT_S_WIFI         [] = "AT+S.WIFI\r\n\0";
char AT_S_ROAM         [] = "AT+S.ROAM\r\n\0";
char AT_S_GPIOC        [] = "AT+S.GPIOC\r\n\0";
char AT_S_GPIOR        [] = "AT+S.GPIOR\r\n\0";
char AT_S_GPIOW        [] = "AT+S.GPIOW\r\n\0";
char AT_S_FWUPDATE     [] = "AT+S.FWUPDATE\r\n\0";
char AT_S_HTTPDFSUPDATE[] = "AT+S.HTTPDFSUPDATE\r\n\0";
char AT_S_HTTPDFSERASE [] = "AT+S.HTTPDFSERASE \r\n\0";
char AT_S_HTTPD        [] = "AT+S.HTTPD\r\n\0";
char AT_S_SCAN         [] = "AT+S.SCAN\r\n\0";
char AT_S_ADC          [] = "AT+S.ADC\r\n\0";
char AT_S_DAC          [] = "AT+S.DAC\r\n\0";
char AT_S_PWM          [] = "AT+S.PWM\r\n\0";
char AT_S_TLSCERT      [] = "AT+S.TLSCERT\r\n\0";
char AT_S_TLSCERT2     [] = "AT+S.TLSCERT2\r\n\0";
char AT_S_TLSDOMAIN    [] = "AT+S.TLSDOMAIN\r\n\0";
char AT_S_SETTIME      [] = "AT+S.SETTIME\r\n\0";
char AT_S_RMPEER       [] = "AT+S.RMPEER\r\n\0";
char AT_S_HTTPREQ      [] = "AT+S.HTTPREQ\r\n\0";
char AT_S_FSR          [] = "AT+S.FSR\r\n\0";
char AT_S_HTTPDFSWRITE [] = "AT+S.HTTPDFSWRITE \r\n\0";

bool SkeepATStart()
{
	bool skip = false;
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
		dst[*pos] = ATRCV[frame_read & BUFFER_MASK];
		(*pos)++;
		frame_read++;
	}
	return false;
}

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
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
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
	Driver_USART3.Receive(ATRCV, FRAME_SIZE);
	
  while (1) {
    ; // Insert thread code here...
		pos = 0;
		while (!GetStringFromWiFi(buffer_rx, (uint16_t*)&pos))
			osThreadYield ();
		buffer_rx[pos] = '\0';
		osSignalSet(tid_UserWiFiThread, WIFI_EVENT_RECEIVE_STRING);
    osThreadYield ();
  }
}

bool SendAT(char* str)
{
	Driver_USART3.Send(str, strlen(str));
	osThreadYield (); // Receive "OK"
	osSignalWait(WIFI_EVENT_RECEIVE_STRING, osWaitForever);
	if (strcmp(buffer_rx, "OK") != 0)
		return false;
	return true;
}

void UserWiFiThread (void const *argument) {
	// Test WiFi
	Driver_USART3.Send(AT, strlen(AT));
	
	// Receive "OK"
	osThreadYield ();
	osSignalWait(WIFI_EVENT_RECEIVE_STRING, osWaitForever);
	if (strcmp(buffer_rx, "OK") != 0)
		return;
	
	// Get Host Name
	buffer_tx = strcat(AT_S_GCFG, "=ip_hostname\r\n\0");
	Driver_USART3.Send(buffer_tx, strlen(buffer_tx));
	osThreadYield (); // Receive host name
	osSignalWait(WIFI_EVENT_RECEIVE_STRING, osWaitForever);
	osThreadYield (); // Receive "OK"
	osSignalWait(WIFI_EVENT_RECEIVE_STRING, osWaitForever);
	
	SendAT("AT+S.SSIDTXT=ASUS\r\n");
	SendAT("AT+S.SCFG=wifi_wpa_psk_text,host1234\r\n");
	SendAT("AT+S.SCFG=wifi_priv_mode,2\r\n");
	SendAT("AT+S.SCFG=wifi_mode,1\r\n");
	SendAT("AT+S.SCFG=ip_use_dhcp,1\r\n");
	SendAT("AT&W\r\n");
	SendAT("AT+CFUN=1\r\n");
	
  while (1) {
    ; // Insert thread code here...
		
		//Driver_USART3.Send(AT_S_SCAN, strlen(AT_S_SCAN));
		osSignalWait(WIFI_EVENT_RECEIVE_STRING, 3000);
		while (strcmp(buffer_rx, "OK") != 0)
		{
			osThreadYield ();
			osSignalWait(WIFI_EVENT_RECEIVE_STRING, 3000);
		}
    osThreadYield ();
  }
}
