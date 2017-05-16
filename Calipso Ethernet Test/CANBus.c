#include "CANBus.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_gpio.h"
#include "GlobalVariables.h"
#include <string.h>

CAN_HandleTypeDef hcan1;
static CanTxMsgTypeDef        TxMessage;
static CanRxMsgTypeDef        RxMessage;

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
	
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
	
	hcan1.pTxMsg = &TxMessage;
	hcan1.pRxMsg = &RxMessage;
	
	TxMessage.StdId = 0x00;
	TxMessage.ExtId = 0x12345678U;
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.DLC = 2;
	TxMessage.Data[0] = 0x01;
	TxMessage.Data[1] = 0x00;
	
	CAN_FilterConfTypeDef canFilter;
	canFilter.FilterNumber = 0;
	canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
	canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
	canFilter.FilterIdHigh = 0x00000000 << 13;
	canFilter.FilterIdLow = (0x00000000 << 3) | CAN_ID_EXT;
	canFilter.FilterMaskIdHigh = 0x00000000 << 13;
	canFilter.FilterMaskIdLow = (0x00000000 << 3) | CAN_ID_EXT;
	canFilter.FilterFIFOAssignment = CAN_FIFO0;
	canFilter.FilterActivation = ENABLE;
	canFilter.BankNumber = 0;
	
	HAL_CAN_ConfigFilter(&hcan1, &canFilter);
}

void Init_CAN(void)
{
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 105;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SJW = CAN_SJW_1TQ;
  hcan1.Init.BS1 = CAN_BS1_13TQ;
  hcan1.Init.BS2 = CAN_BS2_2TQ;
  hcan1.Init.TTCM = DISABLE;
  hcan1.Init.ABOM = DISABLE;
  hcan1.Init.AWUM = DISABLE;
  hcan1.Init.NART = ENABLE;
  hcan1.Init.RFLM = DISABLE;
  hcan1.Init.TXFP = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    //Error_Handler();
		printf("CAN1 Initialization failed.\n");
  }
}

bool CANSendCommand(uint8_t dev_addr, uint8_t cmd, uint8_t *data, uint8_t length)
{
	TxMessage.StdId = 0x00;
	TxMessage.ExtId = CAN_MESSAGE_TYPE_CMD_mask | (cmd & CAN_MESSAGE_TYPE_COMMANDID_mask) | (dev_addr << 8);
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.RTR = CAN_RTR_DATA;
	
	memcpy((void*)TxMessage.Data, (void*)data, length);
	TxMessage.DLC = length;
	
	if (HAL_CAN_Transmit(&hcan1, g_wCANTimeout) == HAL_OK)
		return true;
	return false;
}

bool CANWriteRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t length)
{
	TxMessage.StdId = 0x00;
	TxMessage.ExtId = CAN_MESSAGE_TYPE_RW_mask | (reg_addr & CAN_MESSAGE_TYPE_REGISTERID_mask) | (dev_addr << 8);
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.RTR = CAN_RTR_DATA;
	
	memcpy((void*)TxMessage.Data, (void*)data, length);
	TxMessage.DLC = length;
	
	if (HAL_CAN_Transmit(&hcan1, g_wCANTimeout) == HAL_OK)
		return true;
	return false;
}

bool CANReadRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t *length)
{
	TxMessage.StdId = 0x00;
	TxMessage.ExtId = (reg_addr & CAN_MESSAGE_TYPE_REGISTERID_mask) | (dev_addr << 8);
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.DLC = 0;
	
	memcpy((void*)TxMessage.Data, (void*)data, *length);
	
	if (HAL_CAN_Transmit(&hcan1, g_wCANTimeout) == HAL_OK)
	{
		//HAL_Delay(100);
		if (HAL_CAN_Receive(&hcan1, CAN_FIFO0, g_wCANTimeout) == HAL_OK)
		{
			*length = RxMessage.DLC;
			memcpy((void*)data, (void*)RxMessage.Data, *length);
			return true;
		}
	}
	return false;
}

bool CANReadRegisterMultiply(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t *length, uint8_t dev_num)
{
	uint8_t i = 0;
	TxMessage.StdId = 0x00;
	TxMessage.ExtId = (reg_addr & CAN_MESSAGE_TYPE_REGISTERID_mask) | (dev_addr << 8);
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.DLC = 0;
	
	memcpy((void*)TxMessage.Data, (void*)data, *length);
	
	if (HAL_CAN_Transmit(&hcan1, g_wCANTimeout) == HAL_OK)
	{
		//HAL_Delay(100);
		for (i = 0; i < dev_num; i++)
			if (HAL_CAN_Receive(&hcan1, CAN_FIFO0, g_wCANTimeout) == HAL_OK)
			{
				*length = RxMessage.DLC;
				memcpy((void*)&data[i], (void*)RxMessage.Data, *length);
			}
			else 
				data[i] = -1;
	}
	return false;
}
