#ifndef __DGUS_H
#define __DGUS_H

#include <stdint.h>

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif
#ifdef RTE_CMSIS_RTOS                   // when RTE component CMSIS RTOS is used
#include "cmsis_os.h"                   // CMSIS RTOS header file
#endif

#define HEADER_WORD 0xAACC
#define BUFFER_NUM	1024

#define DGUS_EVENT_RECEIVE_COMPLETED	0x01
#define DGUS_EVENT_SEND_COMPLETED			0x02

#define REGISTER_ADDR_PICID	0x03

/* ******************** DGUS PIC IDs ************************ */
#define FRAME_PICID_LOGO				0
#define FRAME_PICID_MAINMENU		1
#define FRAME_PICID_PASSWORD		3 // Process
#define FRAME_PICID_SERVICE			5	// Process

typedef struct __attribute__((__packed__)) DWIN_HEADERDATA_REQ_STRUCT
{
	uint16_t	header;
	uint8_t	  length;
	uint8_t		cmd;
	uint16_t	addr;
	uint8_t		num;
} DWIN_HEADERDATA_REQ;

typedef struct __attribute__((__packed__)) DWIN_HEADERREG_REQ_STRUCT
{
	uint16_t	header;
	uint8_t	  length;
	uint8_t		cmd;
	uint8_t		addr;
	uint8_t		num;
} DWIN_HEADERREG_REQ;

typedef struct __attribute__((__packed__)) DWIN_HEADERDATA_STRUCT
{
	uint16_t	header;
	uint8_t	  length;
	uint8_t		cmd;
	uint16_t	addr;
} DWIN_HEADERDATA;

typedef struct __attribute__((__packed__)) DWIN_HEADERREG_STRUCT
{
	uint16_t	header;
	uint8_t	  length;
	uint8_t		cmd;
	uint8_t		addr;
} DWIN_HEADERREG;

uint16_t convert_w(uint16_t value);
uint32_t convert_d(uint32_t value);
void convert_array_w(uint16_t* dst, uint16_t* src, uint16_t num);

void WriteRegister(uint8_t  addr, void  *data, uint8_t num);
void WriteVariable(uint16_t addr, void  *data, uint8_t num);
void ReadRegister (uint8_t  addr, void **data, uint8_t num);
void ReadVariable (uint16_t addr, void **data, uint8_t num);

void WriteVariableConvert16(uint16_t addr, void  *data, uint8_t num);

uint32_t GetPicId(uint32_t timeout);
void SetPicId(uint16_t pic_id);

void DWIN_USART_callback(uint32_t event);

#endif
