#include "LaserMisc.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_dma.h"
#include "GlobalVariables.h"
#include "SoundLib.h"

TIM_HandleTypeDef htim_flow1 = {0};
TIM_HandleTypeDef htim_flow2 = {0};
TIM_HandleTypeDef htim_cool = {0};
TIM_HandleTypeDef htim_speaker = {0};
DMA_HandleTypeDef hdma_speaker = {0};

// 1600 Hz
uint16_t sine_sound_256[256] = {
	128,	131,	134,	137,	140,	144,	147,	150,	
	153,	156,	159,	162,	165,	168,	171,	174,
	177,	179,	182,	185,	188,	191,	193,	196,
	199,	201,	204,	206,	209,	211,	213,	216,
	218,	220,	222,	224,	226,	228,	230,	232,
	234,	235,	237,	239,	240,	241,	243,	244,
	245,	246,	248,	249,	250,	250,	251,	252,
	253,	253,	254,	254,	254,	255,	255,	255,
	255,	255,	255,	255,	254,	254,	254,	253,
	253,	252,	251,	250,	250,	249,	248,	246,
	245,	244,	243,	241,	240,	239,	237,	235,
	234,	232,	230,	228,	226,	224,	222,	220,
	218,	216,	213,	211,	209,	206,	204,	201,
	199,	196,	193,	191,	188,	185,	182,	179,
	177,	174,	171,	168,	165,	162,	159,	156,
	153,	150,	147,	144,	140,	137,	134,	131,
	128,	125,	122,	119,	116,	112,	109,	106,
	103,	100,	97	,	94	,	91	,	88	,	85	,	82 ,
	79	,	77	,	74	,	71	,	68	,	65	,	63	,	60 ,
	57	,	55	,	52	,	50	,	47	,	45	,	43	,	40 ,
	38	,	36	,	34	,	32	,	30	,	28	,	26	,	24 ,
	22	,	21	,	19	,	17	,	16	,	15	,	13	,	12 ,
	11	,	10	,	8		,	7		,	6		,	6		,	5		,	4	 ,
	3		,	3		,	2		,	2		,	2		,	1		,	1		,	1	 ,
	1		,	1		,	1		,	1		,	2		,	2		,	2		,	3	 ,
	3		,	4		,	5		,	6		,	6		,	7		,	8		,	10 ,
	11	,	12	,	13	,	15	,	16	,	17	,	19	,	21 ,	
	22	,	24	,	26	,	28	,	30	,	32	,	34	,	36 ,	
	38	,	40	,	43	,	45	,	47	,	50	,	52	,	55 ,	
	57	,	60	,	63	,	65	,	68	,	71	,	74	,	77 ,	
	79	,	82	,	85	,	88	,	91	,	94	,	97	,	100,
	103,	106,	109,	112,	116,	119,	122,	125, };

/*uint16_t sine_sound_64[64] = 
	{	128,	140,	153,	165,	177,	188,	199,	209,	218,	226,	234,	240,	245,	250,	253,	254,
		255,	254,	253,	250,	245,	240,	234,	226,	218,	209,	199,	188,	177,	165,	153,	140,
		128,	116,	103,	91,		79,		68,		57,		47,		38,		30,		22,		16,		11,		6,		3,		2,	
		1,		2,		3,		6,		11,		16,		22,		30,		38,		47,		57,		68,		79,		91,		103,	116};*/
	
const uint16_t sine_sound_128[128] = {
	256,	268,	280,	293,	305,	317,	329,	341,	353,	364,	376,	387,	397,	407,	417,	427,
	436,	444,	453,	460,	467,	474,	480,	486,	491,	496,	499,	503,	506,	508,	509,	510,
	510,	510,	509,	508,	506,	503,	500,	496,	491,	486,	481,	474,	468,	460,	453,	445,
	436,	427,	418,	408,	397,	387,	376,	365,	353,	342,	330,	318,	306,	293,	281,	268,
	256,	243,	231,	219,	206,	194,	182,	170,	158,	147,	136,	125,	114,	104,	94,	85,
	76,	67,	59,	51,	44,	37,	31,	25,	20,	16,	12,	8,	6,	3,	2,	1,
	1,	1,	2,	3,	5,	8,	11,	15,	20,	25,	30,	36,	43,	50,	58,	66,
	75,	84,	93,	103,	113,	124,	135,	146,	157,	169,	181,	193,	205,	217,	230,	242,
};

const uint16_t sine_sound_64[64] = {
	256,	280,	305,	329,	353,	376,	397,	417,	436,	453,	467,	480,	491,	499,	506,	509,
	510,	509,	506,	500,	491,	481,	468,	453,	436,	418,	397,	376,	353,	330,	306,	281,
	256,	231,	206,	182,	158,	136,	114,	94,	76,	59,	44,	31,	20,	12,	6,	2,
	1,	2,	5,	11,	20,	30,	43,	58,	75,	93,	113,	135,	157,	181,	205,	230,
};


void FlowInit(void)
{
	__TIM4_CLK_ENABLE();
	__TIM8_CLK_ENABLE();
	
	TIM_Base_InitTypeDef tim8_init = {0};
	tim8_init.Period = 10000;
	tim8_init.Prescaler = 0;
	tim8_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim8_init.CounterMode = TIM_COUNTERMODE_UP;
	tim8_init.RepetitionCounter = 0;
	
	TIM_ClockConfigTypeDef tim8_clock = {0};
	tim8_clock.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	tim8_clock.ClockPolarity = TIM_CLOCKPOLARITY_RISING;
	tim8_clock.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	tim8_clock.ClockFilter = 7; // optional
	
	htim_flow1.Init = tim8_init;
	htim_flow1.Instance = TIM8;
	
	HAL_TIM_Base_Init(&htim_flow1);
	HAL_TIM_ConfigClockSource(&htim_flow1, &tim8_clock);
	
	HAL_TIM_Base_Start(&htim_flow1);
	
	htim_flow2.Init = tim8_init;
	htim_flow2.Instance = TIM4;
	
	HAL_TIM_Base_Init(&htim_flow2);
	HAL_TIM_ConfigClockSource(&htim_flow2, &tim8_clock);
	
	HAL_TIM_Base_Start(&htim_flow2);
}

void CoolInit(void)
{
	__TIM2_CLK_ENABLE();
	
	GPIO_InitTypeDef cool = {0};
	
	cool.Pin   = GPIO_PIN_5; // PWM2
	cool.Mode  = GPIO_MODE_AF_PP;
	cool.Pull  = GPIO_NOPULL;
	cool.Speed = GPIO_SPEED_FREQ_LOW;
	cool.Alternate = GPIO_AF1_TIM2;
	
	HAL_GPIO_Init(GPIOA, &cool);
	
	TIM_Base_InitTypeDef tim2_init = {0};
	tim2_init.Period = 42000; // 1ms period
	tim2_init.Prescaler = 3;
	tim2_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim2_init.CounterMode = TIM_COUNTERMODE_UP;
	tim2_init.RepetitionCounter = 0;
	
	TIM_OC_InitTypeDef tim2_oc_init = {0};
	tim2_oc_init.OCMode = TIM_OCMODE_PWM2; //TIM_OCMODE_PWM1;
	tim2_oc_init.Pulse = 0; // 50% duty cycle
	tim2_oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
	tim2_oc_init.OCFastMode = TIM_OCFAST_ENABLE;
	
	htim_cool.Init = tim2_init;
	htim_cool.Instance = TIM2;
	
	HAL_TIM_OC_Init(&htim_cool);
	HAL_TIM_OC_ConfigChannel(&htim_cool, &tim2_oc_init, TIM_CHANNEL_1);
	
	HAL_TIM_OC_Start(&htim_cool, TIM_CHANNEL_1);
}

void SpeakerDMA(void)
{
	__DMA1_CLK_ENABLE();
	
	DMA_InitTypeDef speaker_dma = {0};
	
	speaker_dma.Channel = DMA_CHANNEL_5;
	speaker_dma.Direction = DMA_MEMORY_TO_PERIPH;
	speaker_dma.PeriphInc = DMA_PINC_DISABLE;
	speaker_dma.MemInc = DMA_MINC_ENABLE;
	speaker_dma.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	speaker_dma.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	speaker_dma.Mode = DMA_CIRCULAR;
	speaker_dma.Priority = DMA_PRIORITY_LOW;
	speaker_dma.FIFOMode = DMA_FIFOMODE_DISABLE;
	speaker_dma.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	speaker_dma.MemBurst = DMA_MBURST_SINGLE;
	speaker_dma.PeriphBurst = DMA_PBURST_SINGLE;
	
	hdma_speaker.Instance = DMA1_Stream4;
	hdma_speaker.Init = speaker_dma;

	HAL_DMA_Init(&hdma_speaker);
	
	//HAL_DMA_Start(&hdma_speaker, (uint32_t)&sine_sound[0], (uint32_t) &TIM3->CCR1, SOUND_BUFFER);
}

void NewSpeakerDMA(void)
{
	__DMA2_CLK_ENABLE();
	
	DMA_InitTypeDef speaker_dma = {0};
	
	speaker_dma.Channel = DMA_CHANNEL_6;
	speaker_dma.Direction = DMA_MEMORY_TO_PERIPH;
	speaker_dma.PeriphInc = DMA_PINC_DISABLE;
	speaker_dma.MemInc = DMA_MINC_ENABLE;
	speaker_dma.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	speaker_dma.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	speaker_dma.Mode = DMA_CIRCULAR;
	speaker_dma.Priority = DMA_PRIORITY_LOW;
	speaker_dma.FIFOMode = DMA_FIFOMODE_DISABLE;
	speaker_dma.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	speaker_dma.MemBurst = DMA_MBURST_SINGLE;
	speaker_dma.PeriphBurst = DMA_PBURST_SINGLE;
	
	hdma_speaker.Instance = DMA2_Stream1;
	hdma_speaker.Init = speaker_dma;

	HAL_DMA_Init(&hdma_speaker);
	
	//HAL_DMA_Start(&hdma_speaker, (uint32_t)&sine_sound[0], (uint32_t) &TIM3->CCR1, SOUND_BUFFER);
}

void SoundGhostOn(void);

void NewSpeakerInit(void)
{
	__TIM1_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	
	GPIO_InitTypeDef speaker = {0};
	
	speaker.Pin   = GPIO_PIN_8 | GPIO_PIN_9; // PWM1
	speaker.Mode  = GPIO_MODE_AF_PP;
	speaker.Pull  = GPIO_NOPULL;
	speaker.Speed = GPIO_SPEED_FREQ_HIGH;
	speaker.Alternate = GPIO_AF1_TIM1;
	
	HAL_GPIO_Init(GPIOE, &speaker);
	
	speaker.Pin   = GPIO_PIN_14 | GPIO_PIN_11; //SD IR2104 control
	speaker.Mode  = GPIO_MODE_OUTPUT_PP;
	speaker.Alternate = 0;
	
	HAL_GPIO_Init(GPIOE, &speaker);
	
	TIM_Base_InitTypeDef tim1_init = {0};
	tim1_init.Period = 1904; // 44100 kHz PWM
	tim1_init.Prescaler = 0;
	tim1_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim1_init.CounterMode = TIM_COUNTERMODE_UP;
	tim1_init.RepetitionCounter = 0;
	
	TIM_OC_InitTypeDef tim1_oc_init = {0};
	tim1_oc_init.OCMode = TIM_OCMODE_PWM1; //TIM_OCMODE_PWM1;
	tim1_oc_init.Pulse = 0; // 0% duty cycle
	tim1_oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
	tim1_oc_init.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	tim1_oc_init.OCFastMode = TIM_OCFAST_ENABLE;
	tim1_oc_init.OCIdleState = TIM_OCIDLESTATE_RESET;
	tim1_oc_init.OCNIdleState = TIM_OCIDLESTATE_RESET;
	
	/*TIM_BreakDeadTimeConfigTypeDef tim1_brkdeadtime_init = {0};
	tim1_brkdeadtime_init.OffStateRunMode = TIM_OSSR_ENABLE;
	tim1_brkdeadtime_init.OffStateIDLEMode = TIM_OSSI_ENABLE;
	tim1_brkdeadtime_init.LockLevel = TIM_LOCKLEVEL_1;
	tim1_brkdeadtime_init.DeadTime = 117;
	tim1_brkdeadtime_init.BreakState = TIM_BREAK_ENABLE;
	tim1_brkdeadtime_init.BreakPolarity = TIM_BREAKPOLARITY_LOW;
	tim1_brkdeadtime_init.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
	
	HAL_TIMEx_ConfigBreakDeadTime(&htim_speaker, &tim1_brkdeadtime_init);*/
	
	htim_speaker.Init = tim1_init;
	htim_speaker.Instance = TIM1;
	
	HAL_TIM_OC_Init(&htim_speaker);
	HAL_TIM_OC_ConfigChannel(&htim_speaker, &tim1_oc_init, TIM_CHANNEL_1);
	
	NewSpeakerDMA();
	htim_speaker.hdma[TIM_DMA_ID_CC1] = &hdma_speaker;
	
	__MISC_SOUND_OFF(); // Sound Off
	
	/*SoundGhostOn();
	
	HAL_Delay(3000);*/
	
	SoundOff();
}

void SpeakerInit(void)
{
	__TIM3_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	
	GPIO_InitTypeDef speaker = {0};
	
	speaker.Pin   = GPIO_PIN_6; // PWM1
	speaker.Mode  = GPIO_MODE_AF_PP;
	speaker.Pull  = GPIO_NOPULL;
	speaker.Speed = GPIO_SPEED_FREQ_HIGH;
	speaker.Alternate = GPIO_AF2_TIM3;
	
	HAL_GPIO_Init(GPIOA, &speaker);
	
	TIM_Base_InitTypeDef tim3_init = {0};
	tim3_init.Period = 1904; // 44100 kHz PWM
	tim3_init.Prescaler = 0;
	tim3_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim3_init.CounterMode = TIM_COUNTERMODE_UP;
	tim3_init.RepetitionCounter = 0;
	
	TIM_OC_InitTypeDef tim3_oc_init = {0};
	tim3_oc_init.OCMode = TIM_OCMODE_PWM2; //TIM_OCMODE_PWM1;
	tim3_oc_init.Pulse = 0; // 0% duty cycle
	tim3_oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
	tim3_oc_init.OCFastMode = TIM_OCFAST_ENABLE;
	
	htim_speaker.Init = tim3_init;
	htim_speaker.Instance = TIM3;
	
	HAL_TIM_OC_Init(&htim_speaker);
	HAL_TIM_OC_ConfigChannel(&htim_speaker, &tim3_oc_init, TIM_CHANNEL_1);
	
	SpeakerDMA();
	htim_speaker.hdma[TIM_DMA_ID_CC1] = &hdma_speaker;
	
	/*SoundGhostOn();
	
	HAL_Delay(3000);*/
	
	SoundOff();
}

void SoundGhostOn(void)
{
	GPIO_InitTypeDef speaker = {0};
	
	speaker.Pin   = GPIO_PIN_6; // PWM1
	speaker.Mode  = GPIO_MODE_AF_PP;
	speaker.Pull  = GPIO_NOPULL;
	speaker.Speed = GPIO_SPEED_FREQ_HIGH;
	speaker.Alternate = GPIO_AF2_TIM3;
	
	HAL_GPIO_Init(GPIOA, &speaker);
	
	__HAL_DMA_SET_COUNTER(&hdma_speaker, 0);
	
	HAL_TIM_OC_Start_DMA(&htim_speaker, TIM_CHANNEL_1, (uint32_t*)&sound_seample[0], 64000);
}

void SoundOn(void)
{
	GPIO_InitTypeDef speaker = {0};
	
	speaker.Pin   = GPIO_PIN_6; // PWM1
	speaker.Mode  = GPIO_MODE_AF_PP;
	speaker.Pull  = GPIO_NOPULL;
	speaker.Speed = GPIO_SPEED_FREQ_HIGH;
	speaker.Alternate = GPIO_AF2_TIM3;
	
	HAL_GPIO_Init(GPIOA, &speaker);
	
	__HAL_DMA_SET_COUNTER(&hdma_speaker, 0);
	
	HAL_TIM_OC_Start_DMA(&htim_speaker, TIM_CHANNEL_1, (uint32_t*)&sine_sound_64[0], 64);
}

void NewSoundOn(void)
{
	__MISC_SOUND_ON();
	
	__HAL_DMA_SET_COUNTER(&hdma_speaker, 0);
	
	//HAL_TIM_OC_Start_DMA(&htim_speaker, TIM_CHANNEL_1, (uint32_t*)&sine_sound_64[0], 64);
	TIM_CCxChannelCmd(htim_speaker.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
	HAL_TIMEx_OCN_Start_DMA(&htim_speaker, TIM_CHANNEL_1, (uint32_t*)&sine_sound_64[0], 64);
}

void SoundOff(void)
{
	GPIO_InitTypeDef speaker = {0};
	
	speaker.Pin   = GPIO_PIN_6; // PWM1
	speaker.Mode  = GPIO_MODE_OUTPUT_PP;
	speaker.Pull  = GPIO_PULLUP;
	speaker.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_TIM_OC_Stop_DMA(&htim_speaker, TIM_CHANNEL_1);
	
	HAL_GPIO_Init(GPIOA, &speaker);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}

void NewSoundOff(void)
{
	__MISC_SOUND_OFF();
	
	//HAL_TIM_OC_Stop_DMA(&htim_speaker, TIM_CHANNEL_1);
	HAL_TIMEx_OCN_Stop_DMA(&htim_speaker, TIM_CHANNEL_1);
}

void CoolOn(void)
{
	g_peltier_en = true;
	//HAL_TIM_OC_Start(&htim_cool, TIM_CHANNEL_1);
}

void CoolOff(void)
{
	g_peltier_en = false;
	__HAL_TIM_SetCompare(&htim_cool, TIM_CHANNEL_1, 42000);
	//HAL_TIM_OC_Stop(&htim_cool, TIM_CHANNEL_1);
}

void CoolSet(uint16_t cool)
{
	uint16_t duty_cycle = cool * 420;
	if (duty_cycle > 42000) duty_cycle = 42000;
	__HAL_TIM_SetCompare(&htim_cool, TIM_CHANNEL_1, 42000 - duty_cycle);
}

LASER_ID GetLaserID()
{
	if (__MISC_LASER_ID0() == GPIO_PIN_SET)
	{
		if (__MISC_LASER_ID1() == GPIO_PIN_SET)
		{			
			return LASER_ID_DIODELASER;
		}
		else
		{
			return LASER_ID_SOLIDSTATE;
		}
	}
	else
	{
		if (__MISC_LASER_ID1() == GPIO_PIN_SET)
		{
			return LASER_ID_SOLIDSTATE2;
		}
		else
		{
			return LASER_ID_LONGPULSE;
		}
	}
}
