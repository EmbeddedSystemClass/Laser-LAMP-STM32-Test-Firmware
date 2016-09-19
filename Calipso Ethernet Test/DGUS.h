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

/* ************************** LASER DIODE DATA ADDRESS ******************** */

#define FRAMEDATA_VARADDR_STATE									0x0000
#define FRAMEDATA_VARADDR_MODE									0x0001
#define FRAMEDATA_VARADDR_FREQ									0x0002
#define FRAMEDATA_VARADDR_DURATIONSTP						0x0003
#define FRAMEDATA_VARADDR_ENERGYSTP							0x0004
#define FRAMEDATA_VARADDR_FLUSHESLIMIT					0x0005
#define FRAMEDATA_VARADDR_DURATION							0x0006 // Duration
#define FRAMEDATA_VARADDR_ENERGY								0x0007 // Energy
#define FRAMEDATA_VARADDR_MELANIN								0x0008
#define FRAMEDATA_VARADDR_PHOTOTYPE							0x0009
#define FRAMEDATA_VARADDR_COOLING								0x000a
#define FRAMEDATA_VARADDR_TEMPER								0x000b
#define FRAMEDATA_VARADDR_COOLICON							0x000c
#define FRAMEDATA_VARADDR_FLOW									0x000d
#define FRAMEDATA_VARADDR_TIMMIN								0x000e
#define FRAMEDATA_VARADDR_TIMSEC								0x000f
#define FRAMEDATA_VARADDR_LASERCNT							0x0010
#define FRAMEDATA_VARADDR_SESSNCNT							0x0012
// Laser Diode Control Buttons
#define FRAMEDATA_VARADDR_BTNINPUT							0x0014
#define FRAMEDATA_VARADDR_BTNREADY							0x0015
#define FRAMEDATA_VARADDR_BTNSTART							0x0016
#define FRAMEDATA_VARADDR_BTNSTOP 							0x0017
// database control
#define FRAMEDATA_VARADDR_DATAOFFS							0x0018 // Deprecated
#define FRAMEDATA_VARADDR_DATAINDEX							0x0019 // Deprecated

#define FRAMEDATA_LASERDIODE_BASE								0x0000
#define FRAMEDATA_TIMER_BASE										0x000e
#define FRAMEDATA_LASERSTATE_BASE								0x000b

/* ************************** DGUS PIC IDs ******************************** */

#define FRAME_PICID_LOGO												0
#define FRAME_PICID_MAINMENU										1
#define FRAME_PICID_PASSWORD										3 			// Process
#define FRAME_PICID_SERVICE											5				// Process
#define FRAME_PICID_LANGMENU										7				// Process
#define FRAME_PICID_SERVICE_SOLIDSTATELASER			9				// Process
#define FRAME_PICID_SERVICE_LASERDIODE					11			// Process
#define FRAME_PICID_SERVICE_BASICSETTINGS				13			// Process
#define FRAME_PICID_SERVICE_WIFISCANNING				15			// Process
#define FRAME_PICID_SERVICE_WIFIAUTHENTICATION	17			// Process
#define FRAME_PICID_LASERDIODE_INPUT						19			// Process
#define FRAME_PICID_LASERDIODE_NUMPAD						20		
#define FRAME_PICID_LASERDIODE_TEMPERATUREOUT		22			// Process
#define FRAME_PICID_LASERDIODE_PREPARETIMER			23			// Process
#define FRAME_PICID_LASERDIODE_READY						24			// Process
#define FRAME_PICID_LASERDIODE_START						26			// Process
#define FRAME_PICID_LASERDIODE_STARTED					28			// Process
#define FRAME_PICID_LASERDIODE_PHOTOTYPE				30			// Process

/* ************************** LASER DIODE CONTROL ************************* */

typedef struct __attribute__((__packed__)) DGUS_PREPARETIMER_STRUCT
{
	uint16_t timer_minutes;
	uint16_t timer_seconds;
} DGUS_PREPARETIMER, *PDGUS_PREPARETIMER;

typedef struct __attribute__((__packed__)) DGUS_LASERPROFILE_STRUCT
{
	// Basic laser settings
	uint16_t Frequency;		// Frequency of laser pulses
	uint16_t DurationCnt;		// Duration of laser pulse
	uint16_t EnergyCnt; // Energy in percentage of one pulse
} DGUS_LASERPROFILE, *PDGUS_LASERPROFILE;

typedef struct __attribute__((__packed__)) DGUS_LASERSETTINGS_STRUCT
{
	// Service settings
	uint16_t FlushesLimit;	// Power of laser light depricated
	uint16_t Duration;		// Duty cycle	
	uint16_t Energy;		// Energy in J
} DGUS_LASERSETTINGS, *PDGUS_LASERSETTINGS;

typedef struct __attribute__((__packed__)) DGUS_LASERDIODE_CONTROLBTN_STRUCT {
	uint16_t onInputBtn;	// on input pressed (1) else (0)
	uint16_t onReadyBtn;	// on ready pressed (1) else (0)
	uint16_t onStartBtn;	// on start pressed (1) else (0)
	uint16_t onStopBtn;		// on stop  pressed (1) else (0)
} DGUS_LASERDIODE_CONTROLBTN, *PDGUS_LASERDIODE_CONTROLBTN;

typedef struct __attribute__((__packed__)) DGUS_LASERDIODE_STATE_STRUCT {
	uint16_t temperature;								// Dynamic data
	uint16_t coolIcon;									// Dynamic data
	uint16_t flow;											// Dynamic data
} DGUS_LASERDIODE_STATE, *PDGUS_LASERDIODE_STATE;

typedef struct __attribute__((__packed__)) DGUS_LASERDIODE_STRUCT
{
	// Application state
	uint16_t state;
	
	// Laser mode
	uint16_t mode;
	
	// Basic laser settings
	DGUS_LASERPROFILE laserprofile;
	
	// Service settings
	DGUS_LASERSETTINGS lasersettings;
	
	// Phototype
	uint16_t melanin;
	uint16_t phototype;
	
	// Cooling progress bar
	uint16_t cooling;
	
	// Cooling settings
	uint16_t temperature;								// Dynamic data
	uint16_t coolIcon;									// Dynamic data
	uint16_t flow;											// Dynamic data
	
	// Timer
	DGUS_PREPARETIMER timer;						// Dynamic data when prepare
	
	// Pulse counter
	uint32_t PulseCounter;							// Dynamic data when work
	uint32_t SessionPulseCounter;				// Dynamic data when work
	
	// Control buttons
	DGUS_LASERDIODE_CONTROLBTN buttons;	// State control data
	
	// Database variables
	uint16_t DatabasePageOffset;				// database control data
	uint16_t DatabaseSelectionIndex;		// database control data
} DGUS_LASERDIODE, *PDGUS_LASERDIODE;

/* ************************** DGUS CONTROL STRUCT ************************* */

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
void conver_laserdata(DGUS_LASERDIODE* dst, DGUS_LASERDIODE* src);
void convert_array_w(uint16_t* dst, uint16_t* src, uint16_t num);

void WriteLaserDiodeDataConvert16(uint16_t addr, DGUS_LASERDIODE *data);
void WriteRegister(uint8_t  addr, void  *data, uint8_t num);
void WriteVariable(uint16_t addr, void  *data, uint8_t num);
void ReadRegister (uint8_t  addr, void **data, uint8_t num);
void ReadVariable (uint16_t addr, void **data, uint8_t num);

void WriteVariableConvert16(uint16_t addr, void  *data, uint8_t num);

uint32_t GetPicId(uint32_t timeout);
void SetPicId(uint16_t pic_id, uint16_t timeout);

void DWIN_USART_callback(uint32_t event);

#endif
