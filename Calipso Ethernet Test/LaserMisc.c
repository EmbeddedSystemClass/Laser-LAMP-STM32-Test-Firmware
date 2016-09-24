#include "LaserMisc.h"
#include "stm32f4xx_hal_tim.h"

TIM_HandleTypeDef htim_flow1 = {0};
TIM_HandleTypeDef htim_flow2 = {0};

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
