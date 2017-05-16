#ifndef __CANBUS_H
#define __CANBUS_H

#include "stm32f4xx_hal_can.h"
#include <stdbool.h>

#define SLOT_ID_1															1 << 7
#define SLOT_ID_0															0 << 7

// Device id groups
#define CAN_RECEIVER_DEVICE_ID_SLOT_mask			0x8000
#define CAN_RECEIVER_DEVICE_ID_PWM_mask				0x4000
#define CAN_RECEIVER_DEVICE_ID_PORT_mask			0x2000
#define CAN_RECEIVER_DEVICE_ID_SENS_mask			0x1000
#define CAN_RECEIVER_DEVICE_ID_GROUP_mask			0x7000

#define CAN_SENDER_DEVICE_ID_SLOT_mask				0x800000
#define CAN_SENDER_DEVICE_ID_PWM_mask					0x400000
#define CAN_SENDER_DEVICE_ID_PORT_mask				0x200000
#define CAN_SENDER_DEVICE_ID_SENS_mask				0x100000
#define CAN_SENDER_DEVICE_ID_GROUP_mask				0x700000

// CMD options
#define CAN_MESSAGE_TYPE_RW_mask							0x80 	// "0" - read, "1" - write
#define CAN_MESSAGE_TYPE_CMD_mask							0x40 	// "0" - register, "1" - command

// CMD types
#define CAN_MESSAGE_TYPE_CMD_RESETCNT					0x01
#define CAN_MESSAGE_TYPE_CMD_TAU							0x02
#define CAN_MESSAGE_TYPE_CMD_ENERGY						0x03
#define CAN_MESSAGE_TYPE_CMD_REQUESTID				0x04

// Registers
#define CAN_MESSAGE_TYPE_REGISTER_ID					0x01
#define CAN_MESSAGE_TYPE_REGISTER_CNT					0x02
#define CAN_MESSAGE_TYPE_REGISTER_TEMPERATURE	0x03
#define CAN_MESSAGE_TYPE_REGISTER_FLOW				0x04
#define CAN_MESSAGE_TYPE_REGISTER_LED					0x05
#define CAN_MESSAGE_TYPE_REGISTER_SWITCH			0x06

#define CAN_MESSAGE_TYPE_REGISTERID_mask			0x3f
#define CAN_MESSAGE_TYPE_COMMANDID_mask				0x3f

extern CAN_HandleTypeDef hcan1;

void Init_CAN(void);
bool CANSendCommand(uint8_t dev_addr, uint8_t cmd, uint8_t *data, uint8_t length);
bool CANWriteRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t length);
bool CANReadRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t *length);
bool CANReadRegisterMultiply(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t *length, uint8_t dev_num);

#endif
