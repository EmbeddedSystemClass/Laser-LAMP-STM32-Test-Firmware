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
uint16_t duration_long [LONGP_DURATIONS_NUM]   = {10, 12, 14, 16,  18,  20,  20,  20,  20,  20};


uint16_t energy_tbl[VOLTAGES_NUM*SHORT_DURATIONS_NUM*3] = 
/*	350, 360,	 370,	 380,	 390,  400,	 410,	 420 - Voltages	*/
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
			
/*	220, 240,	 260,	 280,	 300,  320,	 340,	 360 - Voltages	*/
		5		,8		,11		,16		,20		,25		,30		,35  ,  //	2
		8		,12		,17		,23		,29		,36		,42		,49  ,  //	3
		10	,16		,22		,29		,37		,45		,51		,59  ,  //	4
		12	,19		,26		,35		,43		,52		,60		,68  ,  //	5
		14	,22		,31		,40		,49		,60		,69		,77  ,  //	6
		13	,20		,28		,42		,49		,65		,93		,110 ,  //	7
		20	,31		,43		,56		,68		,87		,102	,118 ,  //	8
		30	,49		,68		,91		,114	,143	,168	,198 ,  //	9
		41	,67		,93		,126	,160	,198	,235	,278 ,  //	10
    41	,67		,93		,126	,160	,198	,235	,278 ,  //	--
		
		41	,67		,93		,126	,160	,198	,235	,278 ,  //	10
		47	,76		,108	,144	,181	,222	,265	,314 ,  //	12
		51	,83		,117	,157	,200	,247	,293	,346 ,  //	14
		56	,91		,128	,171	,218	,268	,322	,371 ,  //	16
		59	,97		,137	,183	,231	,285	,341	,398 ,  //	18
		61	,100	,145	,192	,244	,300	,358	,422 ,  //	20
		61	,100	,145	,192	,244	,300	,358	,422 ,  //	20
		61	,100	,145	,192	,244	,300	,358	,422 ,  //	20
		61	,100	,145	,192	,244	,300	,358	,422 ,  //	20
		61	,100	,145	,192	,244	,300	,358	,422};	//	20
		

/*uint16_t voltage_short[VOLTAGES_NUM] = {350, 360, 370, 380, 390, 400, 410, 420};
uint16_t voltage_other[VOLTAGES_NUM] = {330, 340, 350, 360, 370, 380, 390, 400};*/
		
uint16_t voltage_short[VOLTAGES_NUM] = {320, 330, 340, 350, 360, 370, 380, 390};
uint16_t voltage_other[VOLTAGES_NUM] = {220, 240, 260, 280, 300, 320, 340, 360};

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
