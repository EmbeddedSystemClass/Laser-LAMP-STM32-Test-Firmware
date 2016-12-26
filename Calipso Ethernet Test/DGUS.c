#include "DGUS.h"
#include <string.h>
#include "Driver_USART.h"
#include "GlobalVariables.h"

#include "stm32f4xx_hal_uart.h"
#include <stdbool.h>

#ifdef USE_DGUS_DRIVER
ARM_DRIVER_USART* DGUS_USART_Driver;
#endif
extern osThreadId tid_MainThread;

UART_HandleTypeDef huart1;
uint8_t dgus_buffer_tx[BUFFER_NUM];
uint8_t dgus_buffer_rx[BUFFER_NUM];

HAL_StatusTypeDef HAL_UART_Transmit_ITMY(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  if((pData == NULL ) || (Size == 0U)) 
  {
    return HAL_ERROR;
  }
  
  /* Process Locked */
  __HAL_LOCK(huart);
  
  huart->pTxBuffPtr = pData;
  huart->TxXferSize = Size;
  huart->TxXferCount = Size;

  huart->ErrorCode = HAL_UART_ERROR_NONE;
  huart->gState = HAL_UART_STATE_BUSY_TX;

  /* Process Unlocked */
  __HAL_UNLOCK(huart);

  /* Enable the UART Transmit data register empty Interrupt */
  SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
  
  return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_ITMY(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  if((pData == NULL ) || (Size == 0U)) 
  {
    return HAL_ERROR;
  }
  
  /* Process Locked */
  __HAL_LOCK(huart);
  
  huart->pRxBuffPtr = pData;
  huart->RxXferSize = Size;
  huart->RxXferCount = Size;
  
  huart->ErrorCode = HAL_UART_ERROR_NONE;
  huart->RxState = HAL_UART_STATE_BUSY_RX;
  
  /* Process Unlocked */
  __HAL_UNLOCK(huart);
      
  /* Enable the UART Parity Error Interrupt */
  SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
  
  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* Enable the UART Data Register not empty Interrupt */
  SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);
  
  return HAL_OK;
}

uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    int i;

    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }

    return crc;
}

uint16_t convert_w(uint16_t value)
{
	return (value >> 8) | (value << 8);
}

uint32_t convert_d(uint32_t value)
{
	return (value >> 24) | ((value & 0xff0000) >> 8) | ((value & 0xff00) << 8) | ((value & 0xff) << 24);
}

void convert_array_w(uint16_t* dst, uint16_t* src, uint16_t num)
{
	for (uint16_t i = 0; i < num/2; i++)
		dst[i] = convert_w(src[i]);
}

void WriteRegister(uint8_t addr, void *data, uint8_t num)
{
	DWIN_HEADERREG* header = (DWIN_HEADERREG*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 2;
	header->cmd    = 0x80;
	header->addr   = addr;
	
#ifdef CRC_CHECK
	header->length = num + 4;
#endif
	
	memcpy(dgus_buffer_tx + 5, data, num);
	
#ifdef CRC_CHECK
	header->length = num + 4;
	uint16_t crc = 0xffff;
	for (uint16_t i = 3; i < num + 5; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[num + 5] = crc & 0xff;
	dgus_buffer_tx[num + 6] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 7);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 7);
#endif
#else
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 5);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 5);
#endif
#endif
}

void WriteVariable(uint16_t addr, void *data, uint8_t num)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
#ifdef CRC_CHECK
	header->length = num + 5;
#endif
	
	memcpy(dgus_buffer_tx + 6, data, num);
	
#ifdef CRC_CHECK
	uint16_t crc = 0xffff;
	for (uint16_t i = 3; i < num + 6; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[num + 6] = crc & 0xff;
	dgus_buffer_tx[num + 7] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 8);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 8);
#endif
#else
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 6);
#endif
#endif
}

void WriteVariableConvert16(uint16_t addr, void *data, uint8_t num)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);

#ifdef CRC_CHECK
	header->length = num + 5;
#endif
	
	convert_array_w((uint16_t*)(dgus_buffer_tx + 6), (uint16_t*)data, num);
	
#ifdef CRC_CHECK
	uint16_t crc = 0xffff;
	for (uint16_t i = 3; i < num + 6; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[num + 6] = crc & 0xff;
	dgus_buffer_tx[num + 7] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 8);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 8);
#endif
#else
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 6);
#endif
#endif
}

void ReadRegister(uint8_t  addr, void **data, uint8_t num)
{
	DWIN_HEADERREG_REQ* header = (DWIN_HEADERREG_REQ*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = 3;
	header->cmd    = 0x81;
	header->addr   = addr;
	header->num    = num;
	
#ifdef CRC_CHECK
	header->length = 5;
	uint16_t crc = 0xFFFF;
	for (uint16_t i = 3; i < 6; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[6] = crc & 0xff;
	dgus_buffer_tx[7] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, 8);
	DGUS_USART_Driver->Receive(dgus_buffer_rx, num + 8);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, 8);
	HAL_UART_Receive_ITMY(&huart1, dgus_buffer_rx, num + 8);
#endif

#else
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, 6);
	DGUS_USART_Driver->Receive(dgus_buffer_rx, num + 6);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, 6);
	HAL_UART_Receive_ITMY(&huart1, dgus_buffer_rx, num + 6);
#endif
#endif
	*data = dgus_buffer_rx + 6;
}

void ReadVariable(uint16_t  addr, void **data, uint8_t num)
{
	DWIN_HEADERDATA_REQ* header = (DWIN_HEADERDATA_REQ*)dgus_buffer_tx;
	
	header->header = convert_w(HEADER_WORD);
	header->length = 4;
	header->cmd    = 0x83;
	header->addr   = convert_w(addr);
	header->num    = num/2;
	
#ifdef CRC_CHECK
	header->length = 6;
	uint16_t crc = 0xFFFF;
	for (uint16_t i = 3; i < 7; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[7] = crc & 0xff;
	dgus_buffer_tx[8] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, 9);
	DGUS_USART_Driver->Receive(dgus_buffer_rx, num + 9);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, 9);
	HAL_UART_Receive_ITMY(&huart1, dgus_buffer_rx, num + 9);
#endif
	
	*data = dgus_buffer_rx + 7;
#else
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, 7);
	DGUS_USART_Driver->Receive(dgus_buffer_rx, num + 7);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, 7);
	HAL_UART_Receive_ITMY(&huart1, dgus_buffer_rx, num + 7);
#endif
	
	*data = dgus_buffer_rx + 7;
#endif
}

static inline int bcd_decimal(uint8_t hex)
{
    int dec = ((hex & 0xF0) >> 4) * 10 + (hex & 0x0F);
    return dec;
}  

static inline int bcd_decimal16(uint16_t hex)
{
    int dec = ((hex & 0xF000) >> 12) * 1000 + ((hex & 0xF00) >> 8) * 100 + ((hex & 0xF0) >> 4) * 10 + (hex & 0x0F);
    return dec;
}  

void GetDateTime(uint32_t timeout, DWIN_TIMEDATE* datetime)
{
	DWIN_TIMEDATE* pvalue;
	ReadRegister(0x20, (void**)&pvalue, 16);
	
	if (osSignalWait(DGUS_EVENT_SEND_COMPLETED, timeout).status != osEventTimeout)
		if (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, timeout).status != osEventTimeout)
		{
			//convert_array_w((uint16_t*)datetime, (uint16_t*)pvalue, 16);
			datetime->seconds = bcd_decimal(pvalue->seconds);			
			datetime->minutes = bcd_decimal(pvalue->minutes);
			datetime->hours   = bcd_decimal(pvalue->hours);
			datetime->day     = bcd_decimal(pvalue->day);
			datetime->week    = bcd_decimal(pvalue->week);
			datetime->year    = bcd_decimal(pvalue->year);
		}
}

uint16_t GetPicId(uint32_t timeout, uint16_t pic_id)
{
	uint16_t* pvalue;
	ReadRegister(REGISTER_ADDR_PICID, (void**)&pvalue, 2);
	
	if (osSignalWait(DGUS_EVENT_SEND_COMPLETED, timeout).status != osEventTimeout)
		if (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, timeout).status != osEventTimeout)
			return convert_w(*pvalue);
		
	return pic_id;
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
#ifdef DEBUG_BRK
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
#endif
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
#ifdef DEBUG_BRK
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
#endif
        break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	osSignalSet(tid_MainThread, DGUS_EVENT_RECEIVE_COMPLETED);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	osSignalSet(tid_MainThread, DGUS_EVENT_SEND_COMPLETED);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	huart->ErrorCode = HAL_UART_ERROR_NONE;
}

/* *************************************** Helper Laser Diode Data Functions ************************** */

void convert_wifinetdata(DGUS_WIFISCANNINGLINE* dst, DGUS_WIFISCANNINGLINE* src)
{
	dst->channel							  = convert_w(src->channel);
	dst->RSSI                   = convert_w(src->RSSI);
	memcpy((uint16_t*)&dst->SSID, (uint16_t*)&src->SSID, strlen(src->SSID)+1);
	dst->WPA					          = convert_d(src->WPA);
	dst->WPA2		                = convert_w(src->WPA2);
}

void convert_laserdata(DGUS_LASERDIODE* dst, DGUS_LASERDIODE* src)
{
	dst->state								  = convert_w(src->state);
	dst->mode                   = convert_w(src->mode);
	convert_array_w((uint16_t*)&dst->laserprofile, (uint16_t*)&src->laserprofile, sizeof(DGUS_LASERPROFILE));
	convert_array_w((uint16_t*)&dst->lasersettings, (uint16_t*)&src->lasersettings, sizeof(DGUS_LASERSETTINGS));
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
	convert_array_w((uint16_t*)&dst->lasersettings, (uint16_t*)&src->lasersettings, sizeof(DGUS_LASERSETTINGS));
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
	
#ifdef CRC_CHECK
	header->length = num + 5;
#endif
	
	convert_laserdata((DGUS_LASERDIODE*)(dgus_buffer_tx + 6), data);
	
#ifdef CRC_CHECK
	uint16_t crc = 0xffff;
	for (uint16_t i = 3; i < num + 6; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[num + 6] = crc & 0xff;
	dgus_buffer_tx[num + 7] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 8);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 8);
#endif
#else
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 6);
#endif
#endif
}

void WriteSolidStateLaserDataConvert16(uint16_t addr, DGUS_SOLIDSTATELASER *data)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	uint16_t num = sizeof(DGUS_SOLIDSTATELASER);
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
#ifdef CRC_CHECK
	header->length = num + 5;
#endif
	
	convert_laserdata_ss((DGUS_SOLIDSTATELASER*)(dgus_buffer_tx + 6), data);
	
#ifdef CRC_CHECK
	uint16_t crc = 0xffff;
	for (uint16_t i = 3; i < num + 6; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[num + 6] = crc & 0xff;
	dgus_buffer_tx[num + 7] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 8);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 8);
#endif
#else	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 6);
#endif
#endif
}

void WriteWifiNetDataConvert16(uint16_t addr, DGUS_WIFISCANNINGLINE *data)
{
	DWIN_HEADERDATA* header = (DWIN_HEADERDATA*)dgus_buffer_tx;
	
	uint16_t num = sizeof(DGUS_WIFISCANNINGLINE);
	
	header->header = convert_w(HEADER_WORD);
	header->length = num + 3;
	header->cmd    = 0x82;
	header->addr   = convert_w(addr);
	
#ifdef CRC_CHECK
	header->length = num + 5;
#endif
	
	convert_wifinetdata((DGUS_WIFISCANNINGLINE*)(dgus_buffer_tx + 6), data);
	
#ifdef CRC_CHECK
	uint16_t crc = 0xffff;
	for (uint16_t i = 3; i < num + 6; i++)
		crc = crc16_update(crc, dgus_buffer_tx[i]); //_crc16_update
	dgus_buffer_tx[num + 6] = crc & 0xff;
	dgus_buffer_tx[num + 7] = crc >> 8;
	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 8);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 8);
#endif
#else	
	osSignalClear(tid_MainThread, 0);
#ifdef USE_DGUS_DRIVER
	DGUS_USART_Driver->Send(dgus_buffer_tx, num + 6);
#else
	HAL_UART_Transmit_ITMY(&huart1, dgus_buffer_tx, num + 6);
#endif
#endif
}


void Initialize_DGUS()
{
	__USART1_CLK_ENABLE();
	
	UART_InitTypeDef init_uart = {0};
	
	init_uart.BaudRate = DGUS_BAUDRATE;
  init_uart.WordLength = UART_WORDLENGTH_8B;
  init_uart.StopBits = UART_STOPBITS_1;
  init_uart.Parity = UART_PARITY_NONE;
  init_uart.Mode = UART_MODE_TX_RX;
  init_uart.HwFlowCtl = UART_HWCONTROL_NONE;
  init_uart.OverSampling = UART_OVERSAMPLING_16;
	
	huart1.Init = init_uart;
	huart1.Instance = USART1;
	
	HAL_UART_Init(&huart1);
	
	HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}
