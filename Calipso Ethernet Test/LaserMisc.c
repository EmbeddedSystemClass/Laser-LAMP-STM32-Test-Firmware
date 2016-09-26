#include "LaserMisc.h"
#include "stm32f4xx_hal_tim.h"

TIM_HandleTypeDef htim_flow1 = {0};
TIM_HandleTypeDef htim_flow2 = {0};
TIM_HandleTypeDef htim_cool = {0};

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
	__TIM3_CLK_ENABLE();
	
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

void CoolOn(void)
{
	//HAL_TIM_OC_Start(&htim_cool, TIM_CHANNEL_1);
}

void CoolOff(void)
{
	__HAL_TIM_SetCompare(&htim_cool, TIM_CHANNEL_1, 0);
	//HAL_TIM_OC_Stop(&htim_cool, TIM_CHANNEL_1);
}

void CoolSet(uint16_t cool)
{
	__HAL_TIM_SetCompare(&htim_cool, TIM_CHANNEL_1, cool * 420);
}

LASER_ID GetLaserID()
{
	if (__MISC_LASER_ID0() == GPIO_PIN_SET)
	{
		if (__MISC_LASER_ID1() == GPIO_PIN_SET)
		{			
			return LASER_ID_SOLIDSTATE;
		}
		else
		{
			return LASER_ID_DIODELASER;
		}
	}
	else
	{
		if (__MISC_LASER_ID1() == GPIO_PIN_SET)
		{
			return LASER_ID_LONGPULSE;
		}
		else
		{
			return LASER_ID_FRACTIONAL;
		}
	}
}
