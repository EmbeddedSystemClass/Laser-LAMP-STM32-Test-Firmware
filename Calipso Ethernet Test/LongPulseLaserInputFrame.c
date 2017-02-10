#include <string.h>
#include "DGUS.h"
#include "Driver_USART.h"
#include "stm32f4xx_hal.h"
#include "SolidStateLaser.h"
#include "GlobalVariables.h"
#include "LaserMisc.h"

#include <math.h>
#include "arm_math.h"

#define SHORT_DURATIONS_NUM	10
#define STDRD_DURATIONS_NUM	9
#define LONGP_DURATIONS_NUM	10
#define VOLTAGES_NUM 8

extern void SetDACValue(float32_t value);

float32_t _programI;

uint16_t duration;
uint16_t voltage;

uint16_t duration_short[SHORT_DURATIONS_NUM]   = {20, 40, 60, 80, 100, 120, 140, 160, 180, 200};
uint16_t duration_stdrt[STDRD_DURATIONS_NUM+1] = { 2,  3,  4,  5,   6,   7,   8,   9,  10,  10};
uint16_t duration_long [LONGP_DURATIONS_NUM]   = {10, 12, 14, 16,  18,  20,  22,  24,  26,  28};


uint16_t energy_tbl[VOLTAGES_NUM*SHORT_DURATIONS_NUM*3] = 
/*	350,	360,	370,	380,	390, 	400,	410,	420 - Voltages	*/
	{	1		,2		,3		,4		,5		,6		,7		,8	,  //	200
		7		,8		,9		,10		,11		,13		,14		,15	,  //	400
		11	,14		,17		,20		,23		,26		,28		,31 ,  //	600
		15	,19		,24		,28		,31		,35		,38		,41 ,  //	800
		20	,25		,31		,36		,39		,43		,46		,49 ,  //	1000
		27	,32		,36		,41		,48		,54		,61		,67 ,  //	1200
		35	,39		,43		,47		,54		,61		,67		,74 ,  //	1400
		42	,45		,49		,52		,58		,65		,71		,77 ,  //	1600
		47	,52		,57		,62		,69		,76		,83		,90 ,  //	1800
		50	,59		,68		,77		,84		,91		,98		,105,  //	2000
			
/*	330,	340,	350,	360,	370, 	380,	390,	400 - Voltages	*/
		20		,30		,40		,50		,60		,70		,80		,90  ,  //	2
		50		,60		,70		,80		,90		,100		,110		,120 ,  //	3
		90		,100		,110		,120		,130		,140		,150		,160 ,  //	4
		110	,120		,130		,140		,150		,160		,170		,180 ,  //	5
		120	,140		,160		,180		,190		,200		,210		,220 ,  //	6
		130	,150		,180		,200		,210		,230		,240		,250 ,  //	7
		140	,170		,200		,230		,250		,270		,280		,300 ,  //	8
		180	,200		,230		,250		,280		,310		,330		,360 ,  //	9
		200	,230		,250		,280		,310		,340		,370		,400 ,  //	10
    200	,230		,250		,280		,310		,340		,370		,400	,  //	--
		
		210	,230		,260		,280		,310		,330		,360		,380 ,  //	10
		240	,260		,270		,290		,340		,380		,430		,470 ,  //	12
		270	,280		,290		,300		,350		,410		,460		,510 ,  //	14
		290	,300		,320		,330		,390		,440		,500		,550 ,  //	16
		330	,340		,340		,350		,410		,480		,540		,600 ,  //	18
		350	,360		,370		,380		,450		,510		,580		,640 ,  //	20
		370	,390		,410		,430		,500		,570		,630		,700 ,  //	22
		400	,420		,450		,470		,530		,590		,640		,700 ,  //	24
		400	,430		,470		,500		,550		,600		,650		,700 ,  //	26
		400	,430		,470		,500		,550		,600		,650		,700};  //	28
		

/*uint16_t voltage_short[VOLTAGES_NUM] = {350, 360, 370, 380, 390, 400, 410, 420};
uint16_t voltage_other[VOLTAGES_NUM] = {330, 340, 350, 360, 370, 380, 390, 400};*/
		
uint16_t voltage_short[VOLTAGES_NUM] = {320, 330, 340, 350, 360, 370, 380, 390};
uint16_t voltage_other[VOLTAGES_NUM] = {300, 310, 320, 330, 340, 350, 360, 370};

void LongPulseLaserInput_Init(uint16_t pic_id)
{
	frameData_SolidStateLaser.laserprofile.Frequency = 10;
	frameData_SolidStateLaser.laserprofile.EnergyCnt = 0;
	frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
	frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM];
	frameData_SolidStateLaser.mode = 0;
	frameData_SolidStateLaser.state = 0;
	frameData_SolidStateLaser.connector = 0;
	frameData_SolidStateLaser.PulseCounter = GetSolidStateGlobalPulse(LaserID);
	frameData_SolidStateLaser.SessionPulseCounter = GetSolidStateSessionPulse(LaserID);;
	
	// Reset button states
	frameData_SolidStateLaser.buttons.onInputBtn = 0;
	frameData_SolidStateLaser.buttons.onSimmerBtn = 0;
	frameData_SolidStateLaser.buttons.onStartBtn = 0;
	frameData_SolidStateLaser.buttons.onStopBtn = 0;
	
	WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
	osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
}

void LongPulseLaserInput_Process(uint16_t pic_id)
{
	bool update = false;
	uint16_t new_pic_id = pic_id;
	
	// Reset session flushes
	SolidStateLaserPulseReset(LaserID);
	
	//uint16_t frequency = frameData_SolidStateLaser.laserprofile.Frequency;
	uint16_t durationCnt = frameData_SolidStateLaser.laserprofile.DurationCnt;
	uint16_t energyCnt = frameData_SolidStateLaser.laserprofile.EnergyCnt;
	uint16_t connector = frameData_SolidStateLaser.connector;
	uint16_t mode = frameData_SolidStateLaser.mode;
	
	DGUS_SOLIDSTATELASER* value;
	ReadVariable(FRAMEDATA_SOLIDSTATELASER_BASE, (void**)&value, sizeof(frameData_SolidStateLaser));
	if ((osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout).status != osEventTimeout) && (osSignalWait(DGUS_EVENT_RECEIVE_COMPLETED, g_wDGUSTimeout).status != osEventTimeout))
		convert_laserdata_ss(&frameData_SolidStateLaser, value);
	else 
		return;
	
	uint16_t state = frameData_SolidStateLaser.state;
	
	FlushesCount = 1000000;
	subFlushesCount = 1;
	
	if (frameData_SolidStateLaser.laserprofile.EnergyCnt > VOLTAGES_NUM-1) 
	{
		frameData_SolidStateLaser.laserprofile.EnergyCnt = VOLTAGES_NUM-1;
		update = true;
	}
	
	switch (frameData_SolidStateLaser.mode)
	{
		case 0:
			if (frameData_SolidStateLaser.laserprofile.DurationCnt > SHORT_DURATIONS_NUM-1)
			{
				frameData_SolidStateLaser.laserprofile.DurationCnt = SHORT_DURATIONS_NUM-1;
				update = true;
			}
			break;
		case 1:
			if (frameData_SolidStateLaser.laserprofile.DurationCnt > STDRD_DURATIONS_NUM-1)
			{
				frameData_SolidStateLaser.laserprofile.DurationCnt = STDRD_DURATIONS_NUM-1;
				update = true;
			}
			break;
		case 2:
			if (frameData_SolidStateLaser.laserprofile.DurationCnt > LONGP_DURATIONS_NUM-1)
			{
				frameData_SolidStateLaser.laserprofile.DurationCnt = LONGP_DURATIONS_NUM-1;
				update = true;
			}
			break;
	}
	
	if (frameData_SolidStateLaser.mode == 0)
		frameData_SolidStateLaser.connector = 1;
	else
		frameData_SolidStateLaser.connector = 0;
	
	if (connector != frameData_SolidStateLaser.connector) update = true;
	
	if (frameData_SolidStateLaser.mode == 0) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 100)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 100;
			update = true;
		}
	}
	else
	if (frameData_SolidStateLaser.mode == 1) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 10)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 10;
			update = true;
		}
	}
	else
	if (frameData_SolidStateLaser.mode == 2) 
	{
		if (frameData_SolidStateLaser.laserprofile.Frequency > 4)
		{
			frameData_SolidStateLaser.laserprofile.Frequency = 4;
			update = true;
		}
	}
	
	SetPulseFrequency_(frameData_SolidStateLaser.laserprofile.Frequency);
	
	bool update_val = false;
	if (mode != frameData_SolidStateLaser.mode)		{ update = true; update_val = true; };
	switch (frameData_SolidStateLaser.mode)
	{
		case 0:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM];
		
			voltage = voltage_short[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
			
		case 1:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_stdrt[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*VOLTAGES_NUM];
		
			voltage = voltage_other[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
			
		case 2:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_long[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*VOLTAGES_NUM*2];
		
			voltage = voltage_other[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
	}
	
	if ((durationCnt != frameData_SolidStateLaser.laserprofile.DurationCnt) || update_val)
	{
			update = true;
	}
			
	if ((energyCnt != frameData_SolidStateLaser.laserprofile.EnergyCnt) || update_val)
	{
			update = true;
	}
	
	// ******************************************* Start working with module ************************************************
	
	_programI = ((float32_t)(voltage) / 450.0f) * 10.0f;
	
	if (frameData_SolidStateLaser.mode == 0)
		SetPulseDuration_us(duration * 10);
	else
		SetPulseDuration_ms(duration, duration * 2);

	__SOLIDSTATELASER_DISCHARGEON();
	
	frameData_SolidStateLaser.state = 0;
	
	CoolOn();
	
	if (frameData_SolidStateLaser.buttons.onInputBtn != 0)
	{
		__SOLIDSTATELASER_DISCHARGEOFF();
		__SOLIDSTATELASER_HVON();
		
		frameData_SolidStateLaser.state = 1; // Battery charging
		
#ifdef DEBUG_SOLID_STATE_LASER
		if (__MISC_GETCHARGEMODULEREADYSTATE())
#endif
		{
			frameData_SolidStateLaser.state = 3; // Ready
			
			// On Input Pressed
			frameData_SolidStateLaser.buttons.onInputBtn = 0;
		
			new_pic_id = FRAME_PICID_LONGPULSE_START;
		
			SetDACValue(_programI);
		}
#ifdef DEBUG_SOLID_STATE_LASER
		else
		{
			frameData_SolidStateLaser.mode = mode;
		}
#endif
		update = true;
	}
	
	if (frameData_SolidStateLaser.buttons.onCancelBtn != 0)
	{
		// On Input Pressed
		frameData_SolidStateLaser.buttons.onCancelBtn = 0;
		
		SolidStateLaser_en = false;
		LampControlPulseStop();
		__SOLIDSTATELASER_HVOFF();
		//__SOLIDSTATELASER_SIMMEROFF();
		SetDACValue(0.0f);
		frameData_SolidStateLaser.state = 0;
		new_pic_id = FRAME_PICID_LONGPULSE_INPUT;
		update = true;
		StoreGlobalVariables();
	}
	
#ifdef DEBUG_SOLID_STATE_LASER
	if (!(__MISC_GETSIMMERSENSOR()) && pic_id != 55 && pic_id != 53) 
	{
		new_pic_id = FRAME_PICID_LONGPULSE_SIMMERSTART;
		
		// Solid State Laser Off
		footswitch_en = false;
		SolidStateLaser_en = false;
		LampControlPulseStop();
		osDelay(100);
		__SOLIDSTATELASER_SIMMEROFF();
		osDelay(100);
		__SOLIDSTATELASER_HVOFF();
		osDelay(100);
		__SOLIDSTATELASER_DISCHARGEON();
		
		update = true;
	}
#endif
	
	if (state != frameData_SolidStateLaser.state)
		update = true;
	
	if (frameData_SolidStateLaser.PulseCounter != GetSolidStateGlobalPulse(LaserID))
	{
		frameData_SolidStateLaser.PulseCounter = GetSolidStateGlobalPulse(LaserID);
		frameData_SolidStateLaser.SessionPulseCounter = GetSolidStateSessionPulse(LaserID);;
		update = true;
	}
	
	if (update)
	{
		WriteSolidStateLaserDataConvert16(FRAMEDATA_SOLIDSTATELASER_BASE, &frameData_SolidStateLaser);
		osSignalWait(DGUS_EVENT_SEND_COMPLETED, g_wDGUSTimeout);
	}
	
	if (pic_id != new_pic_id && update)
		SetPicId(new_pic_id, g_wDGUSTimeout);
}
