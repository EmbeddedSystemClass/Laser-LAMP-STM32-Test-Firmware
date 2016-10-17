#ifndef __DGUS_H
#define __DGUS_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif
#ifdef RTE_CMSIS_RTOS                   // when RTE component CMSIS RTOS is used
#include "cmsis_os.h"                   // CMSIS RTOS header file
#endif

//#define OLD_STYLE_LASER_SW
//#define DEBUG_BRK
#define CRC_CHECK
//#define USE_DGUS_DRIVER
#define DGUS_BAUDRATE	9600

#define HEADER_WORD 0xAACC
#define BUFFER_NUM	1024

extern UART_HandleTypeDef huart1;

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
#define FRAMEDATA_VARADDR_BTNCANCEL							0x0018
// database control
/*
#define FRAMEDATA_VARADDR_DATAOFFS							0x0018 // Deprecated
#define FRAMEDATA_VARADDR_DATAINDEX							0x0019 // Deprecated*/

/***************************************************************/
#define FRAMEDATA_SSVARADDR_STATE								0x0100
#define FRAMEDATA_SSVARADDR_MODE								0x0101
#define FRAMEDATA_SSVARADDR_FREQ								0x0102
#define FRAMEDATA_SSVARADDR_DURATION						0x0103 // Reserved
#define FRAMEDATA_SSVARADDR_ENERGYSTP						0x0104
#define FRAMEDATA_SSVARADDR_RESERVED2						0x0105 // Reserved
#define FRAMEDATA_SSVARADDR_ENERGYINT						0x0106 // Energy int
#define FRAMEDATA_SSVARADDR_ENERGY							0x0107 // Energy
#define FRAMEDATA_SSVARADDR_LASERCNT						0x0108
#define FRAMEDATA_SSVARADDR_SESSNCNT						0x010a
// Laser Diode Control Buttons
#define FRAMEDATA_SSVARADDR_BTNINPUT						0x010c
#define FRAMEDATA_SSVARADDR_BTNSIMMER						0x010d
#define FRAMEDATA_SSVARADDR_BTNSTART						0x010e
#define FRAMEDATA_SSVARADDR_BTNSTOP 						0x010f
#define FRAMEDATA_SSVARADDR_BTNCANCEL						0x0110

#define FRAMEDATA_SSVARADDR_CONNECTOR 					0x0111

/* ************************** DGUS DATA STRUCT **************************** */
#define FRAMEDATA_LASERDIODE_BASE								0x0000
#define FRAMEDATA_SOLIDSTATELASER_BASE					0x0100
#define FRAMEDATA_TIMER_BASE										0x000e
#define FRAMEDATA_LASERSTATE_BASE								0x000b

/* ************************** WIFI SCANNING DATA STRUCT ******************* */
#define FRAMEDATA_WIFISCANNING_LINE0_BASE				0x0200						
#define FRAMEDATA_WIFISCANNING_LINE1_BASE				0x0300
#define FRAMEDATA_WIFISCANNING_LINE2_BASE				0x0400
#define FRAMEDATA_WIFISCANNING_LINE3_BASE				0x0500
#define FRAMEDATA_WIFISCANNING_LINE4_BASE				0x0600
#define FRAMEDATA_WIFISCANNING_LINE5_BASE				0x0700
#define FRAMEDATA_WIFISCANNING_LINE6_BASE				0x0800
#define FRAMEDATA_WIFISCANNING_LINE7_BASE				0x0900
#define FRAMEDATA_WIFISCANNING_LINE8_BASE				0x0A00
#define FRAMEDATA_WIFISCANNING_LINE9_BASE				0x0B00
#define FRAMEDATA_WIFISCANNING_LINE10_BASE			0x0C00
#define FRAMEDATA_WIFISCANNING_LINE11_BASE			0x0D00

// WIFI Authentication
#define FRAMEDATA_WIFIAUTHENTICATION_SSID				0x0E00
#define FRAMEDATA_WIFIAUTHENTICATION_PASSWORD		0x0E10
#define FRAMEDATA_WIFISCANNINGSSID_INDEX				0x0E20

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
#define FRAME_PICID_LASERDIODE_TEMPERATUREOUT		23			// Process
#define FRAME_PICID_LASERDIODE_PREPARETIMER			24			// Process
#define FRAME_PICID_LASERDIODE_FLOWERROR				25			// Process
#define FRAME_PICID_LASERDIODE_READY						26			// Process
#define FRAME_PICID_LASERDIODE_START						28			// Process
#define FRAME_PICID_LASERDIODE_STARTED					30			// Process
#define FRAME_PICID_LASERDIODE_PHOTOTYPE				32			// Process
#define FRAME_PICID_REMOTECONTROL								34			// Process

#define FRAME_PICID_SOLIDSTATE_INPUT						35			// Process
#define FRAME_PICID_SOLIDSTATE_SIMMERSTART			37
#define FRAME_PICID_SOLIDSTATE_SIMMER						39			// Process
#define FRAME_PICID_SOLIDSTATE_START						40			// Process
#define FRAME_PICID_SOLIDSTATE_WORK							42			// Process
#define FRAME_PICID_SOLIDSTATE_FLOWERROR				44			// Process
#define FRAME_PICID_SOLIDSTATE_OVERHEATING			45			// Process
#define FRAME_PICID_SOLIDSTATE_FAULT						46			// Process

#define FRAME_PICID_WRONG_EMMITER								47			// Process

#define FRAME_PICID_WIFI_LINKING								50			// Process

#define FRAME_PICID_LONGPULSE_INPUT						  51			// Process
#define FRAME_PICID_LONGPULSE_SIMMERSTART			  53
#define FRAME_PICID_LONGPULSE_SIMMER					  55			// Process
#define FRAME_PICID_LONGPULSE_START						  56			// Process
#define FRAME_PICID_LONGPULSE_WORK						  58			// Process
#define FRAME_PICID_LONGPULSE_FLOWERROR				  60			// Process
#define FRAME_PICID_LONGPULSE_OVERHEATING			  61			// Process
#define FRAME_PICID_LONGPULSE_FAULT						  62			// Process

#define IS_LASERDIODE(pic_id) (pic_id >= 19 && pic_id <= 32)
#define IS_SOLIDSTATE(pic_id) (pic_id >= 37 && pic_id <= 42)
#define IS_LONGPULSE (pic_id) (pic_id >= 53 && pic_id <= 62)

/* ************************** WIFI SCANNING DATA ************************** */

typedef struct __attribute__((__packed__)) DGUS_WIFISCANNINGLINE_STRUCT {
	uint16_t channel;
	char SSID[32];
	uint16_t RSSI;
	uint16_t WPA;
	uint16_t WPA2;
} DGUS_WIFISCANNINGLINE, *PDGUS_WIFISCANNINGLINE;

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

typedef struct __attribute__((__packed__)) DGUS_SSLASERSETTINGS_STRUCT
{
	// Service settings
	uint16_t FlushesLimit;	// Power of laser light depricated
	uint16_t EnergyInt;		// Duty cycle	
	uint16_t Energy;		// Energy in J
} DGUS_SSLASERSETTINGS, *PDGUS_SSLASERSETTINGS;

typedef struct __attribute__((__packed__)) DGUS_LASERDIODE_CONTROLBTN_STRUCT {
	uint16_t onInputBtn;	// on input  pressed (1) else (0)
	uint16_t onReadyBtn;	// on ready  pressed (1) else (0)
	uint16_t onStartBtn;	// on start  pressed (1) else (0)
	uint16_t onStopBtn;		// on stop   pressed (1) else (0)
	uint16_t onCancelBtn;	// on cancel pressed (1) else (0)
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
	
	/*
	// Database variables
	uint16_t DatabasePageOffset;				// database control data
	uint16_t DatabaseSelectionIndex;		// database control data
	*/
	//uint16_t FlushesLimitIcon;
} DGUS_LASERDIODE, *PDGUS_LASERDIODE;

/* ************************** SOLID STATE LASER CONTROL ******************* */

typedef struct __attribute__((__packed__)) DGUS_SOLIDSTATELASER_CONTROLBTN_STRUCT {
	uint16_t onInputBtn;	// on input  pressed (1) else (0)
	uint16_t onSimmerBtn;	// on simmer pressed (1) else (0)
	uint16_t onStartBtn;	// on start  pressed (1) else (0)
	uint16_t onStopBtn;		// on stop   pressed (1) else (0)
	uint16_t onCancelBtn;	// on cancel pressed (1) else (0)
} DGUS_SOLIDSTATELASER_CONTROLBTN, *PDGUS_SOLIDSTATELASER_CONTROLBTN;

typedef struct __attribute__((__packed__)) DGUS_SOLIDSTATELASER_STRUCT
{
	// Application state
	uint16_t state;
	
	// Laser mode
	uint16_t mode;
	
	// Basic laser settings
	DGUS_LASERPROFILE laserprofile;
	
	// Service settings
	DGUS_SSLASERSETTINGS lasersettings;
	
	// Pulse counter
	uint32_t PulseCounter;							// Dynamic data when work
	uint32_t SessionPulseCounter;				// Dynamic data when work
	
	// Control buttons
	DGUS_SOLIDSTATELASER_CONTROLBTN buttons;	// State control data
	
	uint16_t connector;
} DGUS_SOLIDSTATELASER, *PDGUS_SOLIDSTATELASER;

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

void convert_wifinetdata(DGUS_WIFISCANNINGLINE* dst, DGUS_WIFISCANNINGLINE* src);
void convert_laserdata_ss(DGUS_SOLIDSTATELASER* dst, DGUS_SOLIDSTATELASER* src);
void convert_laserdata(DGUS_LASERDIODE* dst, DGUS_LASERDIODE* src);
void convert_array_w(uint16_t* dst, uint16_t* src, uint16_t num);

void WriteWifiNetDataConvert16(uint16_t addr, DGUS_WIFISCANNINGLINE *data);
void WriteLaserDiodeDataConvert16(uint16_t addr, DGUS_LASERDIODE *data);
void WriteSolidStateLaserDataConvert16(uint16_t addr, DGUS_SOLIDSTATELASER *data);
void WriteRegister(uint8_t  addr, void  *data, uint8_t num);
void WriteVariable(uint16_t addr, void  *data, uint8_t num);
void ReadRegister (uint8_t  addr, void **data, uint8_t num);
void ReadVariable (uint16_t addr, void **data, uint8_t num);

void WriteVariableConvert16(uint16_t addr, void  *data, uint8_t num);

uint16_t GetPicId(uint32_t timeout, uint16_t pic_id);
void SetPicId(uint16_t pic_id, uint16_t timeout);

void DWIN_USART_callback(uint32_t event);

void Initialize_DGUS(void);

#endif
