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
#define VOLTAGES_SNUM 8
#define VOLTAGES_NUM 16//8

extern void SetDACValue(float32_t value);

float32_t _programI;

uint16_t duration;
uint16_t voltage;

uint16_t duration_short[SHORT_DURATIONS_NUM]   = {20, 40, 60, 80, 100, 120, 140, 160, 180, 200};
uint16_t duration_stdrt[STDRD_DURATIONS_NUM+1] = { 2,  3,  4,  5,   6,   7,   8,   9,  10,  10};
uint16_t duration_long [LONGP_DURATIONS_NUM]   = {10, 12, 14, 16,  18,  20,  20,  20,  20,  20};


uint16_t energy_tbl[VOLTAGES_SNUM*SHORT_DURATIONS_NUM + VOLTAGES_NUM*SHORT_DURATIONS_NUM*2] = 
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
			
/*	220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 - Voltages	*/
		5		,6		,8		,10		,11		,14		,16		,18		,20		,23		,25		,27		,30		,32		,35  ,38  ,  //	2
		8		,10		,12		,15		,17		,20		,23		,26		,29		,32		,36		,39		,42		,45		,49  ,52  ,  //	3
		10	,13		,16		,19		,22		,26		,29		,33		,37		,41		,45		,48		,51		,55		,59  ,64  ,  //	4
		12	,16		,19		,23		,26		,31		,35		,39		,43		,48		,52		,56		,60		,64		,68  ,73  ,  //	5
		14	,18		,22		,27		,31		,36		,40		,45		,49		,54		,60		,64		,69		,73		,77  ,82  ,  //	6
		13	,17		,20		,24		,28		,35		,42		,45		,49		,57		,65		,79		,93		,102	,110 ,123 ,  //	7
		20	,26		,31		,37		,43		,49		,56		,62		,68		,77		,87		,95		,102	,110	,118 ,133 ,  //	8
		30	,40		,49		,58		,68		,79		,91		,102	,114	,128	,143	,155	,168	,183	,198 ,217 ,  //	9
		41	,54		,67		,80		,93		,109	,126	,143	,160	,179	,198	,216	,235	,256	,278 ,300 ,  //	10
    41	,54		,67		,80		,93		,109	,126	,143	,160	,179	,198	,216	,235	,256	,278 ,300 ,  //	--

/*	240 ,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380 ,390 - Voltages	*/
		41	,54		,67		,80		,93		,109	,126	,143	,160	,179	,198	,216	,235	,256	,278 ,300	,  //	10
		47	,62		,76		,92		,108	,126	,144	,162	,181	,202	,222	,244	,265	,289	,314 ,339	,  //	12
		51	,67		,83		,100	,117	,137	,157	,178	,200	,223	,247	,270	,293	,320	,346 ,375	,  //	14
		56	,74		,91		,109	,128	,149	,171	,195	,218	,243	,268	,295	,322	,346	,371 ,401	,  //	16
		59	,78		,97		,117	,137	,160	,183	,207	,231	,258	,285	,313	,341	,369	,398 ,430	,  //	18
		61	,80		,100	,122	,145	,168	,192	,218	,244	,272	,300	,329	,358	,390	,422 ,455	,  //	20
		61	,80		,100	,122	,145	,168	,192	,218	,244	,272	,300	,329	,358	,390	,422 ,455	,  //	20
		61	,80		,100	,122	,145	,168	,192	,218	,244	,272	,300	,329	,358	,390	,422 ,455	,  //	20
		61	,80		,100	,122	,145	,168	,192	,218	,244	,272	,300	,329	,358	,390	,422 ,455	,  //	20
		61	,80		,100	,122	,145	,168	,192	,218	,244	,272	,300	,329	,358	,390	,422 ,455};	 //	20
	
uint16_t voltage_tbl[VOLTAGES_SNUM*SHORT_DURATIONS_NUM + VOLTAGES_NUM*SHORT_DURATIONS_NUM*2] = 
/*	350, 360,	 370,	 380,	 390,  400,	 410,	 420 - Voltages	*/
	{	350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	200
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	400
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	600
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	800
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	1000
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	1200
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	1400
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	1600
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	1800
		350, 360,	 370,	 380,	 390,  400,	 410,	 420,  //	2000
			
/*	220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 - Voltages	*/
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	2
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	3
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	4
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	5
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	6
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	7
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	8
		220	,230	,240	,250	,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360 ,370 ,  //	9
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400 ,  //	10 -- Calibrated for Long Pulse 10 ms
    250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400 ,  //	--

/*	250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400 - Voltages	*/
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	10
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	12
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	14
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	16
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400 ,  //	18
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	20
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	20
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	20
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400	,  //	20
		250 ,260	,270	,280	,290	,300	,310	,320	,330	,340	,350	,360	,370	,380	,390 ,400};	 //	20

		
/*uint16_t voltage_short[VOLTAGES_SNUM] = {320, 330, 340, 350, 360, 370, 380, 390};
uint16_t voltage_stdrt[VOLTAGES_NUM] = {220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370};
uint16_t voltage_long[VOLTAGES_NUM] = {250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400};*/

void LongPulseLaserInput_Init(uint16_t pic_id)
{
	frameData_SolidStateLaser.laserprofile.DurationCnt = 0;
	frameData_SolidStateLaser.laserprofile.Frequency = 10;
	frameData_SolidStateLaser.laserprofile.EnergyCnt = 0;
	frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
	frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_SNUM];
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
	
	if (frameData_SolidStateLaser.mode == 0)
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > VOLTAGES_SNUM-1) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = VOLTAGES_SNUM-1;
			update = true;
		}
	}
	else
	{
		if (frameData_SolidStateLaser.laserprofile.EnergyCnt > VOLTAGES_NUM-1) 
		{
			frameData_SolidStateLaser.laserprofile.EnergyCnt = VOLTAGES_NUM-1;
			update = true;
		}
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
	frequency_publish = frameData_SolidStateLaser.laserprofile.Frequency * 0.1f;
	
	bool update_val = false;
	if (mode != frameData_SolidStateLaser.mode)		{ update = true; update_val = true; };
	switch (frameData_SolidStateLaser.mode)
	{
		case 0:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_short[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_SNUM];
		
			voltage = voltage_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_SNUM];
			//voltage_short[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			break;
			
		case 1:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_stdrt[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*VOLTAGES_SNUM];
		
			//voltage = voltage_stdrt[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			voltage = voltage_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*VOLTAGES_SNUM];
			break;
			
		case 2:
			frameData_SolidStateLaser.lasersettings.EnergyInt = duration_long[frameData_SolidStateLaser.laserprofile.DurationCnt];
			duration = frameData_SolidStateLaser.lasersettings.EnergyInt;
			frameData_SolidStateLaser.lasersettings.Energy = energy_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*(VOLTAGES_NUM + VOLTAGES_SNUM)];
		
			//voltage = voltage_long[frameData_SolidStateLaser.laserprofile.EnergyCnt];
			voltage = voltage_tbl[frameData_SolidStateLaser.laserprofile.EnergyCnt + frameData_SolidStateLaser.laserprofile.DurationCnt*VOLTAGES_NUM + SHORT_DURATIONS_NUM*(VOLTAGES_NUM + VOLTAGES_SNUM)];
			break;
	}
	
	energy_publish = frameData_SolidStateLaser.lasersettings.Energy * 0.1f;
	
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
	{
		SetPulseDuration_us(duration * 10);
		duration_publish = duration * 10.0f * 0.000001f;
	}
	else
	{
		SetPulseDuration_ms(duration, duration * 2);
		duration_publish = duration * 0.001f;
	}

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
	
	static int16_t simmer_off_cnt = 0;
#ifdef DEBUG_SOLID_STATE_LASER
	if (!(__MISC_GETSIMMERSENSOR()))
	{
		simmer_off_cnt++;
		if (simmer_off_cnt > 10) simmer_off_cnt = 10;
	}
	else
	{
		simmer_off_cnt = 0;
	}
	if ((simmer_off_cnt > 5) && pic_id != 55 && pic_id != 53) 
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
