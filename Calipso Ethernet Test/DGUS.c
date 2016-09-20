#include "DGUS.h"
#include <string.h>
#include "Driver_USART.h"

ARM_DRIVER_USART* DGUS_USART_Driver;
extern osThreadId tid_MainThread;

uint8_t dgus_buffer_tx[BUFFER_NUM];
uint8_t dgus_buffer_rx[BUFFER_NUM];

uint16_t convert_w(uint16_t value)
{
	return (value >> 8) | (value << 8);
}

uint32_t convert_d(uint32_t value)
{
	return (value >> 24) || ((value & 0xff0000) >> 8) || ((value & 0xff00) << 8) || ((value & 0xff) << 24);
}

void convert_array_w(uint16_t* dst, uint16_t* src, uint16_t num)
{
	for (uint16_t i = 0; i < num; i++)
		dst[i] = convert_w(src[i]);
}

void WriteRegister(uint8_t addr, void *data, uint8_t num)
{
	DWIN_HEADERREG* header = (DWIN_HEADERREG*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 2;
	header->cmd    = 0x80;
	header->addr   = addr;
	
	memcpy(dgus_buffer_tx + 5, data, num);
	
	osSignalClear(tid_MainThread, 0);
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 5);
}

void WriteVariable(uint16_t addr, void *data, uint8_t num)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
	memcpy(dgus_buffer_tx + 6, data, num);
	
	osSignalClear(tid_MainThread, 0);
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
}

void WriteVariableConvert16(uint16_t addr, void *data, uint8_t num)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
	convert_array_w((uint16_t*)(dgus_buffer_tx + 6), (uint16_t*)data, num / 2);
	
	osSignalClear(tid_MainThread, 0);
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
}

void ReadRegister(uint8_t  addr, void **data, uint8_t num)
{
	DWIN_HEADERREG_REQ* header = (DWIN_HEADERREG_REQ*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = 3;
	header->cmd    = 0x81;
	header->addr   = addr;
	header->num    = num;
	
	osSignalClear(tid_MainThread, 0);
	DGUS_USART_Driver->Send(dgus_buffer_tx, 6);
	DGUS_USART_Driver->Receive(dgus_buffer_rx, num + 6);
	
	*data = dgus_buffer_rx + 6;
}

void ReadVariable(uint16_t  addr, void **data, uint8_t num)
{
	DWIN_HEADERDATA_REQ* header = (DWIN_HEADERDATA_REQ*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = 4;
	header->cmd    = 0x83;
	header->addr   = convert_w(addr);
	header->num    = num;
	
	osSignalClear(tid_MainThread, 0);
	DGUS_USART_Driver->Send(dgus_buffer_tx, 7);
	DGUS_USART_Driver->Receive(dgus_buffer_rx, num*2 + 7);
	
	*data = dgus_buffer_rx + 7;
}

uint32_t GetPicId(uint32_t timeout)
{
	uint16_t* pvalue;
	ReadRegister(REGISTER_ADDR_PICID, (void**)&pvalue, 2);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, timeout);
	osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, timeout);
	return convert_w(*pvalue);
}

void SetPicId(uint16_t pic_id, uint16_t timeout)
{
	uint16_t value = convert_w(pic_id);
	WriteRegister(REGISTER_ADDR_PICID, &value, sizeof(value));
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, timeout);
}

/* Private functions ---------------------------------------------------------*/
void DWIN_USART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
				/* Success: Wakeup Thread */
				osSignalSet(tid_MainThread, DGUS_EVENT_RECEIVE_COMPLETED);
        break;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
				/* Success: Wakeup Thread */
				break;
    case ARM_USART_EVENT_SEND_COMPLETE:
				/* Success: Wakeup Thread */
				osSignalSet(tid_MainThread, DGUS_EVENT_SEND_COMPLETED);
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

/* *************************************** Helper Laser Diode Data Functions ************************** */

void convert_laserdata(DGUS_LASERDIODE* dst, DGUS_LASERDIODE* src)
{
	dst->state								  = convert_w(src->state);
	dst->mode                   = convert_w(src->mode);
	convert_array_w((uint16_t*)&dst->laserprofile, (uint16_t*)&src->laserprofile, sizeof(DGUS_LASERPROFILE));
	convert_array_w((uint16_t*)&dst->lasersettings, (uint16_t*)&src->lasersettings, sizeof(PDGUS_LASERSETTINGS));
	dst->PulseCounter           = convert_d(src->PulseCounter);
	dst->melanin                = convert_w(src->melanin);
	dst->phototype              = convert_w(src->phototype);
	dst->temperature            = convert_w(src->temperature);
	dst->cooling                = convert_w(src->cooling);
	dst->flow                   = convert_w(src->flow);
	convert_array_w((uint16_t*)&dst->timer, (uint16_t*)&src->timer, sizeof(DGUS_PREPARETIMER));
	dst->coolIcon								= convert_w(src->coolIcon);
	dst->SessionPulseCounter    = convert_d(src->SessionPulseCounter);
	convert_array_w((uint16_t*)&dst->buttons, (uint16_t*)&src->buttons, sizeof(DGUS_LASERDIODE_CONTROLBTN));
}

void convert_laserdata_ss(DGUS_SOLIDSTATELASER* dst, DGUS_SOLIDSTATELASER* src)
{
	dst->state								  = convert_w(src->state);
	dst->mode                   = convert_w(src->mode);
	convert_array_w((uint16_t*)&dst->laserprofile, (uint16_t*)&src->laserprofile, sizeof(DGUS_LASERPROFILE));
	convert_array_w((uint16_t*)&dst->lasersettings, (uint16_t*)&src->lasersettings, sizeof(PDGUS_LASERSETTINGS));
	dst->PulseCounter           = convert_d(src->PulseCounter);
	dst->SessionPulseCounter    = convert_d(src->SessionPulseCounter);
	convert_array_w((uint16_t*)&dst->buttons, (uint16_t*)&src->buttons, sizeof(DGUS_SOLIDSTATELASER_CONTROLBTN));
	dst->connector              = convert_w(src->connector);
}

void WriteLaserDiodeDataConvert16(uint16_t addr, DGUS_LASERDIODE *data)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	uint16_t num = sizeof(DGUS_LASERDIODE);
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
	convert_laserdata((DGUS_LASERDIODE*)(dgus_buffer_tx + 6), data);
	
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
}

void WriteSolidStateLaserDataConvert16(uint16_t addr, DGUS_SOLIDSTATELASER *data)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	uint16_t num = sizeof(DGUS_LASERDIODE);
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
	convert_laserdata_ss((DGUS_SOLIDSTATELASER*)(dgus_buffer_tx + 6), data);
	
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
}
