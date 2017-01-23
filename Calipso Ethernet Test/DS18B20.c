#include "ds18b20.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal_uart.h"
#include <stdbool.h>

uint8_t t_buffer[32];
volatile bool recv_complete;

extern ARM_DRIVER_USART Driver_USART6;

UART_InitTypeDef init_uart_9600;
UART_InitTypeDef init_uart_115200;
UART_HandleTypeDef huart ={0};

#define OW_0    0x00
#define OW_1    0xff
#define OW_R    0xff

/* DS18B20 Commands */
const uint8_t convert_T[] = {
                OW_0, OW_0, OW_1, OW_1, OW_0, OW_0, OW_1, OW_1, // 0xcc SKIP ROM
                OW_0, OW_0, OW_1, OW_0, OW_0, OW_0, OW_1, OW_0  // 0x44 CONVERT
};

const uint8_t read_scratch[] = {
                OW_0, OW_0, OW_1, OW_1, OW_0, OW_0, OW_1, OW_1, // 0xcc SKIP ROM
                OW_0, OW_1, OW_1, OW_1, OW_1, OW_1, OW_0, OW_1, // 0xbe READ SCRATCH
                OW_R, OW_R, OW_R, OW_R, OW_R, OW_R, OW_R, OW_R,
                OW_R, OW_R, OW_R, OW_R, OW_R, OW_R, OW_R, OW_R
};

/* Private functions ---------------------------------------------------------*/
void DS18B20_USART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
				/* Success: Wakeup Thread */
				recv_complete = true;
        break;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
				/* Success: Wakeup Thread */
				break;
    case ARM_USART_EVENT_SEND_COMPLETE:
				/* Success: Wakeup Thread */
				break;
    case ARM_USART_EVENT_TX_COMPLETE:
        /* Success: Wakeup Thread */
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
				recv_complete = true;
#ifdef DEBUG_BRK
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
#endif
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
			recv_complete = true;
#ifdef DEBUG_BRK
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
#endif
        break;
    }
}

/*void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	recv_complete = true;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}*/

void Init_DS18B20(void)
{
	/*Initialize the USART driver */
  Driver_USART6.Initialize(DS18B20_USART_callback);
  /*Power up the USART peripheral */
  Driver_USART6.PowerControl(ARM_POWER_FULL);
	
  /*Configure the USART to 9600 Bits/sec */
  Driver_USART6.Control(ARM_USART_MODE_ASYNCHRONOUS |
												ARM_USART_DATA_BITS_8 |
												ARM_USART_PARITY_NONE |
												ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_NONE, 9600);
	
	/* Enable Receiver and Transmitter lines */
  Driver_USART6.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART6.Control (ARM_USART_CONTROL_RX, 1);
	
	/*UART_InitTypeDef init_uart_9600 = {0};
	
	init_uart_9600.BaudRate = 9600;
  init_uart_9600.WordLength = UART_WORDLENGTH_8B;
  init_uart_9600.StopBits = UART_STOPBITS_1;
  init_uart_9600.Parity = UART_PARITY_NONE;
  init_uart_9600.Mode = UART_MODE_TX_RX;
  init_uart_9600.HwFlowCtl = UART_HWCONTROL_NONE;
  init_uart_9600.OverSampling = UART_OVERSAMPLING_16;
	
	huart.Init = init_uart_9600;
	huart.Instance = USART6;
	
	memcpy(&init_uart_115200, &init_uart_9600, sizeof(init_uart_9600));
	init_uart_115200.BaudRate = 115200;
	
	HAL_UART_Init(&huart);
	
	HAL_NVIC_SetPriority(USART6_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(USART6_IRQn);*/
}

bool DS18B20_Reset(void)
{
	uint8_t data_in = 0x00, data_out = 0xf0;
	/*huart.Init = init_uart_9600;
	HAL_UART_Init(&huart);
	
	recv_complete = false;
	HAL_UART_Transmit(&huart, &data_out, 1, 1000);
	HAL_UART_Receive(&huart, &data_in, 1, 1000);
	
	huart.Init = init_uart_115200;
	HAL_UART_Init(&huart);*/
	
	/*Configure the USART to 9600 Bits/sec */
  Driver_USART6.Control(ARM_USART_MODE_ASYNCHRONOUS |
												ARM_USART_DATA_BITS_8 |
												ARM_USART_PARITY_NONE |
												ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_NONE, 9600);
	
	/* Enable Receiver and Transmitter lines */
  Driver_USART6.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART6.Control (ARM_USART_CONTROL_RX, 1);
	
	HAL_Delay(1);
	
	recv_complete = false;
	Driver_USART6.Send((void*)&data_out, 1);
	Driver_USART6.Receive((void*)&data_in, 1);
	while (!recv_complete);
	
	/*Configure the USART to 9600 Bits/sec */
  Driver_USART6.Control(ARM_USART_MODE_ASYNCHRONOUS |
												ARM_USART_DATA_BITS_8 |
												ARM_USART_PARITY_NONE |
												ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_NONE, 115200);
	
	/* Enable Receiver and Transmitter lines */
  Driver_USART6.Control (ARM_USART_CONTROL_TX, 1);
  Driver_USART6.Control (ARM_USART_CONTROL_RX, 1);
	
	return (data_in != 0xf0);
}

void DS18B20_StartConvertion(void)
{
	recv_complete = false;
	Driver_USART6.Send((void*)&convert_T, 16);
	Driver_USART6.Receive((void*)&t_buffer, 16);
	while (!recv_complete);
}

uint16_t DS18B20_ReadData(void)
{	
	recv_complete = false;
	Driver_USART6.Receive((void*)&t_buffer, 32);
	Driver_USART6.Send((void*)&read_scratch, 32);
	while (!recv_complete);
	
	uint16_t tt = 0;

  for (uint8_t i=16; i<32; i++) {
     if (t_buffer[i] == 0xff) {
             tt = (tt>>1) | 0x8000;
     } else {
             tt = tt>>1;
     }
  }
	
	return tt;
}
