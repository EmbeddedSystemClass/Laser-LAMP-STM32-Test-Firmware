#include "DGUS.h"
#include <string.h>
#include "Driver_USART.h"

extern ARM_DRIVER_USART Driver_USART1;

uint8_t dgus_buffer_tx[BUFFER_NUM];

uint16_t convert_w(uint16_t value)
{
	return (value >> 8) | (value << 8);
}

uint32_t convert_d(uint32_t value)
{
	return value;
}

void WriteRegister(uint8_t addr, void *data, uint8_t num)
{
	DWIN_HEADERREG* header = (DWIN_HEADERREG*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 2;
	header->cmd    = 0x80;
	header->addr   = addr;
	
	memcpy(dgus_buffer_tx + 5, data, num);
	
	Driver_USART1.Send(dgus_buffer_tx, num + 5);
}

void WriteVariable(uint16_t addr, void *data, uint8_t num)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
	memcpy(dgus_buffer_tx + 6, data, num);
	
	Driver_USART1.Send(dgus_buffer_tx, num + 6);
}

void ReadRegister(uint8_t  addr, void *data, uint8_t num)
{
	DWIN_HEADERREG_REQ* header = (DWIN_HEADERREG_REQ*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = 3;
	header->cmd    = 0x81;
	header->addr   = addr;
	header->num    = num;
	
	Driver_USART1.Send(dgus_buffer_tx, 6);
	Driver_USART1.Receive(data, num + 6);
}

void ReadVariable(uint16_t  addr, void *data, uint8_t num)
{
	DWIN_HEADERDATA_REQ* header = (DWIN_HEADERDATA_REQ*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = 4;
	header->cmd    = 0x83;
	header->addr   = convert_w(addr);
	header->num    = num;
	
	Driver_USART1.Send(dgus_buffer_tx, 7);
	Driver_USART1.Receive(data, num + 7);
}

/* Private functions ---------------------------------------------------------*/
void DWIN_USART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
				/* Success: Wakeup Thread */
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
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}
