
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "DGUS.h"
#include "Driver_USART.h"

#include <string.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 /* USART Driver */
extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART3;
extern ARM_DRIVER_USART* DGUS_USART_Driver;

#define WIFI_EVENT_RECEIVE_COMPLETED	0x02
#define WIFI_EVENT_SEND_COMPLETED			0x04
 
void MainThread (void const *argument);                             // thread function
osThreadId tid_MainThread;                                          // thread id
osThreadDef (MainThread, osPriorityNormal, 1, 0);                   // thread object

/* Private functions ---------------------------------------------------------*/
void WIFI_USART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
				/* Success: Wakeup Thread */
				osSignalSet(tid_MainThread, WIFI_EVENT_RECEIVE_COMPLETED);
        break;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
				/* Success: Wakeup Thread */
				break;
    case ARM_USART_EVENT_SEND_COMPLETE:
				/* Success: Wakeup Thread */
				osSignalSet(tid_MainThread, WIFI_EVENT_SEND_COMPLETED);
				break;
    case ARM_USART_EVENT_TX_COMPLETE:
        /* Success: Wakeup Thread */
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
         //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}

//----------------------------- GUI FRAMES ----------------------------------
extern void PasswordFrame_Process(uint16_t pic_id);
extern void  ServiceFrame_Process(uint16_t pic_id);

int Init_Main_Thread (void) {

  tid_MainThread = osThreadCreate (osThread(MainThread), NULL);
  if (!tid_MainThread) return(-1);
	
	/*Initialize the USART driver */
  Driver_USART1.Initialize(DWIN_USART_callback);
  /*Power up the USART peripheral */
  Driver_USART1.PowerControl(ARM_POWER_FULL);
	
  /*Configure the USART to 115200 Bits/sec */
  Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 115200);
	
	/* Enable Receiver and Transmitter lines */
  Driver_USART1.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);
	
	/*
	//Initialize the USART driver 
  Driver_USART3.Initialize(WIFI_USART_callback);
  //Power up the USART peripheral 
  Driver_USART3.PowerControl(ARM_POWER_FULL);
	
  //Configure the USART to 115200 Bits/sec 
  Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_EVEN |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_RTS_CTS, 115200);
	
	// Enable Receiver and Transmitter lines 
  Driver_USART3.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART3.Control (ARM_USART_CONTROL_RX, 1);
  
	char ATCMD[] = "AT+S.SCFG=ipaddr\n\0";
	char ATRCV[64];
	Driver_USART3.Send(ATCMD, strlen(ATCMD));
	Driver_USART3.Receive(ATRCV, 32);
	osSignalWait(WIFI_EVENT_RECEIVE_COMPLETED, 100);*/
	
	/* Set DGUS driver */
	DGUS_USART_Driver = &Driver_USART1;
	
  return(0);
}

void MainThread (void const *argument) {
	uint16_t pic_id;

  while (1) {
    ; // Insert thread code here...
		pic_id = GetPicId(100);
		
		// GUI Frames process
		switch (pic_id)
		{
			case FRAME_PICID_LOGO:
				SetPicId(FRAME_PICID_MAINMENU);
				break;
			
			// Frames process
			case FRAME_PICID_PASSWORD:	PasswordFrame_Process(pic_id);			break;
			case FRAME_PICID_SERVICE:		ServiceFrame_Process(pic_id);				break;
			
			default:
				break;
		}
		
    osThreadYield ();                                           // suspend thread
  }
}
