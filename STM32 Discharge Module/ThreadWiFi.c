#include "cmsis_os.h"                   // CMSIS RTOS header file

#include "Driver_USART.h"
#include "cmsis_os.h"                   /* ARM::CMSIS:RTOS:Keil RTX */
#include <stdio.h>
#include <string.h>
 
void myUART_Thread(void const *argument);
osThreadId tid_myUART_Thread;
 
/* USART Driver */
extern ARM_DRIVER_USART Driver_USART3;
 
 
void myUSART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:    
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
    case ARM_USART_EVENT_SEND_COMPLETE:
    case ARM_USART_EVENT_TX_COMPLETE:
        /* Success: Wakeup Thread */
        osSignalSet(tid_myUART_Thread, 0x01);
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}
 
/* CMSIS-RTOS Thread - UART command thread */
void myUART_Thread(const void* args)
{
    static ARM_DRIVER_USART * USARTdrv = &Driver_USART6;
    ARM_DRIVER_VERSION     version;
    ARM_USART_CAPABILITIES drv_capabilities;
    char                   cmd;
 
  #ifdef DEBUG
    version = USARTdrv->GetVersion();
    if (version.api < 0x200)   /* requires at minimum API version 2.00 or higher */
    {                          /* error handling */
        return;
    }
    drv_capabilities = USARTdrv->GetCapabilities();
    if (drv_capabilities.event_tx_complete == 0)
    {                          /* error handling */
        return;
    }
  #endif
 
    /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 4800 Bits/sec */
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 4800);
     
    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
 
    USARTdrv->Send("\nPress Enter to receive a message", 34);
    osSignalWait(0x01, osWaitForever);
     
    while (1)
    {
        USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
        osSignalWait(0x01, osWaitForever);
        if (cmd == 13)                       /* CR, send greeting  */
        {
          USARTdrv->Send("\nHello World!", 12);
          osSignalWait(0x01, osWaitForever);
        }
 
    }
}

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
void Thread (void const *argument);                             // thread function
osThreadId tid_Thread;                                          // thread id
osThreadDef (Thread, osPriorityNormal, 1, 0);                   // thread object

int Init_Thread (void) {

  tid_Thread = osThreadCreate (osThread(Thread), NULL);
  if (!tid_Thread) return(-1);
  
  return(0);
}

void Thread (void const *argument) {

  while (1) {
    ; // Insert thread code here...
    osThreadYield ();                                           // suspend thread
  }
}
