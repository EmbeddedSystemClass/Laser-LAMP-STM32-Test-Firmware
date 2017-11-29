#include <string.h>
#include "Driver_USART.h"

#include "DGUS.h"
#include "SDCard.h"
#include "LaserMisc.h"
#include "SolidStateLaser.h"

// Calypso System Global Configuration and Variables
#include "GlobalVariables.h"
#include "IPL_Energy_Table.h"

#include <math.h>
#include "arm_math.h"

// Frequency num
#define IPL_SLOW_MIN_FREQ				1
#define IPL_SLOW_MAX_FREQ				3
#define IPL_DOUBLE_MIN_FREQ			3
#define IPL_DOUBLE_MAX_FREQ			10

// Duration num
#define IPL_SINGLE_MIN_DURATION	0
#define IPL_SINGLE_MAX_DURATION	IPL_DURATIONS_NUM-1
#define IPL_SLOW_MIN_DURATION		0
#define IPL_SLOW_MAX_DURATION		IPL_DURATIONS_NUM-1
#define IPL_DOUBLE_MIN_DURATION	0
#define IPL_DOUBLE_MAX_DURATION	IPL_DURATIONS_NUM-1

// Energy num
#define IPL_SINGLE_MIN_ENERGY		0
#define IPL_SINGLE_MAX_ENERGY		IPL_VOLTAGES_NUM-1
#define IPL_SLOW_MIN_ENERGY			0
#define IPL_SLOW_MAX_ENERGY			IPL_VOLTAGES_NUM-1
#define IPL_DOUBLE_MIN_ENERGY		0
#define IPL_DOUBLE_MAX_ENERGY		IPL_VOLTAGES_NUM-1

static void _StopIPL(void);
static void _PrepareIPL(void);
static void _IgnitionIPL(void);

extern bool IgnitionIPLState(void);
extern void SetDACValue(float32_t value);

DGUS_LASERPROFILE localIPLProfiles[3];

uint16_t Min(uint16_t x, uint16_t y)
{
	if (x <  y) { return x; }
	if (x >  y) { return y; }
	if (x == y) { return x; }
}

uint16_t Max(uint16_t x, uint16_t y)
{
	if (x >  y) { return x; }
	if (x <  y) { return y; }
	if (x == y) { return x; }
}

bool _IPL_freq_correct(uint16_t mode, __packed uint16_t *freq)
{
	bool update = false;
	if ((mode == 0) && ((*freq > 1) || (*freq < 1)))
	{
			// Set frequency for mode 0
			*freq = 1;
			update = true;
	}
	
	if (mode == 1) 
	{
		if (*freq < IPL_SLOW_MIN_FREQ)
		{
			// Set frequency for mode 1
			*freq = IPL_SLOW_MIN_FREQ;
			update = true;
		}
		if (*freq > IPL_SLOW_MAX_FREQ)
		{
			// Set frequency for mode 1
			*freq = IPL_SLOW_MAX_FREQ;
			update = true;
		}
	}
	
	if (mode == 2) 
	{
		if (*freq < IPL_DOUBLE_MIN_FREQ)
		{
			// Set frequency for mode 2
			*freq = IPL_DOUBLE_MIN_FREQ;
			update = true;
		}
		if (*freq > IPL_DOUBLE_MAX_FREQ)
		{
			// Set frequency for mode 2
			*freq = IPL_DOUBLE_MAX_FREQ;
			update = true;
		}
	}
	return update;
}

bool _IPL_duration_correct(uint16_t mode, __packed uint16_t *duration)
{
	uint16_t new_duration;
	
	switch (mode)
	{
		case 0: 
			new_duration = Min(IPL_SINGLE_MAX_DURATION, Max(IPL_SINGLE_MIN_DURATION, *duration)); 
		break;
		case 1: 
			new_duration = Min(  IPL_SLOW_MAX_DURATION, Max(  IPL_SLOW_MIN_DURATION, *duration)); 
		break;
		case 2: 
			new_duration = Min(IPL_DOUBLE_MAX_DURATION, Max(IPL_DOUBLE_MIN_DURATION, *duration)); 
		break;
	}
	
	if (new_duration != *duration)
	{
		*duration = new_duration;
		return true;
	}
	else
		return false;
}

bool _IPL_energy_correct(uint16_t mode, __packed uint16_t *energy)
{
	uint16_t new_energy;
	
	switch (mode)
	{
		case 0: 
			new_energy = Min(IPL_SINGLE_MAX_ENERGY, Max(IPL_SINGLE_MIN_ENERGY, *energy)); 
		break;
		case 1: 
			new_energy = Min(  IPL_SLOW_MAX_ENERGY, Max(  IPL_SLOW_MIN_ENERGY, *energy)); 
		break;
		case 2: 
			new_energy = Min(IPL_DOUBLE_MAX_ENERGY, Max(IPL_DOUBLE_MIN_ENERGY, *energy)); 
		break;
	}
	
	if (new_energy != *energy)
	{
		*energy = new_energy;
		return true;
	}
	else
		return false;
}

void IPLInput_Init(uint16_t pic_id)
{
	frameData_LaserDiode.mode = PROFILE_SINGLE;
	
	memcpy((void*)&frameData_LaserDiode.laserprofile, (void*)&localIPLProfiles[frameData_LaserDiode.mode], sizeof(DGUS_LASERPROFILE));
	
	// Init counters
	frameData_LaserDiode.PulseCounter = FlushesGlobalIPL;
	frameData_LaserDiode.SessionPulseCounter = 0;
	
	// Init phototype indicators
	frameData_LaserDiode.melanin = 0;
	frameData_LaserDiode.phototype = 1;
	
	// Init cooling indicators
	frameData_LaserDiode.temperature = temperature;
	frameData_LaserDiode.cooling = 5;
	frameData_LaserDiode.flow = 0;
	
	// Timer data initialization
	frameData_LaserDiode.timer.timer_seconds = m_wSetSec;
	frameData_LaserDiode.timer.timer_minutes = m_wSetMin;
	
	// Reset buttons state
	frameData_LaserDiode.buttons.onCancelBtn = 0x00;
	frameData_LaserDiode.buttons.onIgnitionBtn = 0x00;
	frameData_LaserDiode.buttons.onInputBtn = 0x00;
	frameData_LaserDiode.buttons.onReadyBtn = 0x00;
	frameData_LaserDiode.buttons.onRestartBtn = 0x00;
	frameData_LaserDiode.buttons.onStartBtn = 0x00;
	frameData_LaserDiode.buttons.onStopBtn = 0x00;
	
	// Init DGUS
	WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	
	__SOLIDSTATELASER_DISCHARGEON();
	__SOLIDSTATELASER_HVOFF();
	__SOLIDSTATELASER_SIMMEROFF();
	__MISC_LASERDIODE_OFF();
	
	DiodeControlPulseStop();
	DiodeLaser_en = false;
}

uint16_t getPhototypeFromMelanin(uint16_t melanin)
{	
	if (melanin < 10)
	{
		frameData_LaserDiode.phototype = 1;
		return 1;
	}
	else
	if (melanin < 20)
	{
		frameData_LaserDiode.phototype = 2;
		return 2;
	}
	else
	if (melanin < 35)
	{
		frameData_LaserDiode.phototype = 3;
		return 3;
	}
	else
	if (melanin < 49)
	{
		frameData_LaserDiode.phototype = 4;
		return 4;
	}
	else
	if (melanin < 72)
	{
		frameData_LaserDiode.phototype = 5;
		return 5;
	}
	else
	{
		frameData_LaserDiode.phototype = 6;
		return 6;
	}
}

void IPLInput_Process(uint16_t pic_id)
{
	volatile bool update = false;
	uint16_t new_pic_id = pic_id;
	
	// Reset session flushes
	SolidStateLaserPulseReset(slot1_id);
	
	// read old previos state.
	uint16_t frequency		= frameData_SolidStateLaser.laserprofile.Frequency;
	uint16_t energyCnt 		= frameData_LaserDiode.laserprofile.EnergyCnt;
	uint16_t durationCnt	= frameData_LaserDiode.laserprofile.DurationCnt;
	uint16_t mode 				= frameData_LaserDiode.mode;
	uint16_t melanin			= frameData_LaserDiode.melanin;
	uint16_t phototype		= frameData_LaserDiode.phototype;
	
	DGUS_LASERDIODE* value;
	ReadVariable(FRAMEDATA_LASERDIODE_BASE, (void**)&value, sizeof(DGUS_LASERDIODE));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata(&frameData_LaserDiode, value);
	else 
		return;
	
	// Reset counters state
	FlushesCount = 1000000;
	if (frameData_LaserDiode.mode == 3)
		subFlushesCount = 2;
	else
		subFlushesCount = 1;
	
	// Update if changed
	if (mode 				!= frameData_LaserDiode.mode)											update = true;
	if (energyCnt 	!= frameData_LaserDiode.laserprofile.EnergyCnt)		update = true;
	if (durationCnt != frameData_LaserDiode.laserprofile.DurationCnt)	update = true;
	if (melanin		  != frameData_LaserDiode.melanin)
	{
		// melanin preset
		frameData_LaserDiode.phototype = getPhototypeFromMelanin(frameData_LaserDiode.melanin);
		switch (frameData_LaserDiode.phototype)
		{
			case 1:
			case 2:
				frameData_LaserDiode.laserprofile.EnergyCnt = 9;
				frameData_LaserDiode.laserprofile.DurationCnt = 1;
				break;
			case 3:
			case 4:
				frameData_LaserDiode.laserprofile.EnergyCnt = 0;
				frameData_LaserDiode.laserprofile.DurationCnt = 3;
				break;
			case 5:
			case 6:
				frameData_LaserDiode.laserprofile.EnergyCnt = 0;
				frameData_LaserDiode.laserprofile.DurationCnt = 4;
				break;
		}
		update = true;
	}
	if (phototype != frameData_LaserDiode.phototype)
	{
		// melanin preset
		switch (frameData_LaserDiode.phototype)
		{
			case 1:
				frameData_LaserDiode.melanin = 5;
			case 2:
				frameData_LaserDiode.melanin = 14;
				frameData_LaserDiode.laserprofile.EnergyCnt = 9;
				frameData_LaserDiode.laserprofile.DurationCnt = 1;
				break;
			case 3:
				frameData_LaserDiode.melanin = 27;
			case 4:
				frameData_LaserDiode.melanin = 41;
				frameData_LaserDiode.laserprofile.EnergyCnt = 0;
				frameData_LaserDiode.laserprofile.DurationCnt = 3;
				break;
			case 5:
				frameData_LaserDiode.melanin = 61;
			case 6:
				frameData_LaserDiode.melanin = 85;
				frameData_LaserDiode.laserprofile.EnergyCnt = 0;
				frameData_LaserDiode.laserprofile.DurationCnt = 4;
				break;
		}
		update = true;
	}
	
	// Set initial state of laser system
	CoolOn();
	
	// Correct
	update |= _IPL_freq_correct(frameData_LaserDiode.mode, &frameData_LaserDiode.laserprofile.Frequency);
	update |= _IPL_duration_correct(frameData_LaserDiode.mode, &frameData_LaserDiode.laserprofile.DurationCnt);
	update |= _IPL_energy_correct(frameData_LaserDiode.mode, &frameData_LaserDiode.laserprofile.EnergyCnt);
	
	// Calculate energy, voltage and duration
	uint16_t duration_id = frameData_LaserDiode.laserprofile.DurationCnt;
	uint16_t voltage_id  = frameData_LaserDiode.laserprofile.EnergyCnt;
	uint16_t energy      = GetIPLEnergy(voltage_id, duration_id);
	uint16_t duration    = global_IPL_Duration_Table[duration_id];
	uint16_t voltage     = global_IPL_Voltage_Table [voltage_id];
	SetDACValue(voltage / 45.0f); // 450 - 0 to 10 - 0 range conversion
	SetPulseDuration_ms(duration/subFlushesCount, duration * 2);
	SetPulseFrequency(frameData_LaserDiode.laserprofile.Frequency);
	
	// calculate duration and energy
	frameData_LaserDiode.lasersettings.Duration = duration;
	frameData_LaserDiode.lasersettings.Energy = energy;
	
	// Publish data to server
	frequency_publish = frameData_LaserDiode.laserprofile.Frequency;
	duration_publish = frameData_LaserDiode.lasersettings.Duration;
	energy_publish = frameData_LaserDiode.lasersettings.Energy;
	
	// check single mode
	if (mode == 0) Profile = PROFILE_SINGLE; // Set single mode
	else 					 Profile = PROFILE_MEDIUM;
	
	// Process buttons
	if (frameData_LaserDiode.buttons.onIgnitionBtn != 0)
	{
		_IgnitionIPL();
		
		frameData_LaserDiode.buttons.onIgnitionBtn = 0;
		new_pic_id = FRAME_PICID_IPL_IGNITION_PROCESS;
		update = true;
	}
	
	// if ignition coplete, jump to FRAME_PICID_IPL_INPUT
	if (pic_id == FRAME_PICID_IPL_IGNITION_PROCESS)
	{
		if (IgnitionIPLState())
		{
			new_pic_id = FRAME_PICID_IPL_INPUT;
			update = true;
		}
	}
	
	// if ignition lost, return to FRAME_PICID_IPL_IGNITION
	if (pic_id == FRAME_PICID_IPL_INPUT)
	{
		if (!IgnitionIPLState())
		{
			new_pic_id = FRAME_PICID_IPL_IGNITION;
			update = true;
		}
	}
	
	if (frameData_LaserDiode.buttons.onInputBtn != 0)
	{					
		// On Input Pressed
		_PrepareIPL();
		
		frameData_LaserDiode.buttons.onInputBtn = 0;
		new_pic_id = FRAME_PICID_IPL_BATTERY_CHARGING;
		update = true;
	}
	
	if (frameData_LaserDiode.buttons.onCancelBtn != 0)
	{
		// On Cancel Pressed
		_StopIPL();
		
		frameData_LaserDiode.buttons.onCancelBtn = 0;
		new_pic_id = FRAME_PICID_IPL_IGNITION;
		update = true;
	}
	
	// Update IPL counter
	if (frameData_LaserDiode.PulseCounter != GetSolidStateGlobalPulse(slot1_id))
	{
		frameData_LaserDiode.PulseCounter = GetSolidStateGlobalPulse(slot1_id);
		frameData_LaserDiode.SessionPulseCounter = GetSolidStateSessionPulse(slot1_id);
		update = true;
	}
	
	// Update IPL interface
	if (update)
	{
		WriteLaserDiodeDataConvert16(FRAMEDATA_LASERDIODE_BASE, &frameData_LaserDiode);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	// Update IPL menu
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}

void _StopIPL(void)
{
	SolidStateLaser_en = false;
	LampControlPulseStop();
		
	__SOLIDSTATELASER_SIMMEROFF();
	__SOLIDSTATELASER_HVOFF();
	__SOLIDSTATELASER_DISCHARGEON();
	SetDACValue(0.0f);
	
	StoreGlobalVariables();
}

void _PrepareIPL(void)
{
	__SOLIDSTATELASER_DISCHARGEOFF();
	__SOLIDSTATELASER_HVON();
	__SOLIDSTATELASER_SIMMERON();
	prepare = true;
}

void _IgnitionIPL(void)
{
	__SOLIDSTATELASER_DISCHARGEON();
	__SOLIDSTATELASER_HVOFF();
	__SOLIDSTATELASER_SIMMERON();
}
