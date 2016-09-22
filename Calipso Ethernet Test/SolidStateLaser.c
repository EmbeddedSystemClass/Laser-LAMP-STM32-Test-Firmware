#include "SolidStateLaser.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "GlobalVariables.h"

TIM_HandleTypeDef hTIM9;
TIM_HandleTypeDef hTIM10;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
		case GPIO_PIN_LAMP_Ready:	
			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_LAMP_Ready);
			break;
		case GPIO_PIN_LAMP_HVOn:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_HVOn);
			break;
		case GPIO_PIN_LAMP_Fault:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_Fault);
			break;
		case GPIO_PIN_LAMP_OV:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_OV);
			break;
		case GPIO_PIN_LAMP_OT:	
			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_LAMP_OT);
			break;
		case GPIO_PIN_LAMP_SIMMERSEN:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_SIMMERSEN);
			break;
		default:
			break;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//__HAL_TIM_DISABLE(&hTIM9);
	hTIM9.Instance->CR1 &= ~(TIM_CR1_CEN);
	__HAL_TIM_SET_COUNTER(&hTIM9, 0);
	//HAL_TIM_OC_Stop_IT(&hTIM9, TIM_CHANNEL_2);
	//__HAL_TIM_DISABLE_IT(&hTIM9, TIM_IT_UPDATE);
}

void LampControlGPIOInit(void)
{
	__GPIOC_CLK_ENABLE();
	__GPIOF_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio_c = {0};
	gpio_c.Pin   = GPIO_PIN_LAMP_Ready | GPIO_PIN_LAMP_OT;
	gpio_c.Mode  = GPIO_MODE_IT_RISING;
	gpio_c.Pull  = GPIO_NOPULL;
	gpio_c.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOC, &gpio_c);
	
	gpio_c.Pin   = GPIO_PIN_LAMP_HVINH;
	gpio_c.Mode  = GPIO_MODE_OUTPUT_PP;
	
	HAL_GPIO_Init(GPIOC, &gpio_c);
	
	GPIO_InitTypeDef gpio_f = {0};
	gpio_f.Pin   = GPIO_PIN_LAMP_HVOn | GPIO_PIN_LAMP_Fault | GPIO_PIN_LAMP_OV | GPIO_PIN_LAMP_SIMMERSEN;
	gpio_f.Mode  = GPIO_MODE_IT_RISING;
	gpio_f.Pull  = GPIO_NOPULL;
	gpio_f.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOF, &gpio_f);
	
	gpio_f.Pin   = GPIO_PIN_LAMP_SIMMEREN | GPIO_PIN_LAMP_DISCHARGE;
	gpio_f.Mode  = GPIO_MODE_OUTPUT_PP;
	
	HAL_GPIO_Init(GPIOF, &gpio_f);
	
	// HV Off
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_LAMP_HVINH, GPIO_PIN_RESET);
	
	// Enable GPIO interrupts
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	
	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	
	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(EXTI3_IRQn);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	
	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void LampControlTIMInit(void)
{
	__TIM9_CLK_ENABLE();
	__TIM10_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio_e = {0};
	gpio_e.Pin   = GPIO_PIN_6;
	gpio_e.Mode  = GPIO_MODE_AF_PP;
	gpio_e.Pull  = GPIO_NOPULL;
	gpio_e.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_e.Alternate = GPIO_AF3_TIM9;
	
	HAL_GPIO_Init(GPIOE, &gpio_e);
	
	// Master timer
	TIM_Base_InitTypeDef tim10_init = {0};
	tim10_init.Period = 4200; // 0.1s period
	//tim10_init.Period = 42000; // 1s period
	tim10_init.Prescaler = 3999;
	tim10_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim10_init.CounterMode = TIM_COUNTERMODE_UP;
	tim10_init.RepetitionCounter = 0;
	
	TIM_OC_InitTypeDef tim10_oc_init = {0};
	tim10_oc_init.OCMode = TIM_OCMODE_PWM2;
	tim10_oc_init.Pulse = 1000;
	tim10_oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
	tim10_oc_init.OCFastMode = TIM_OCFAST_ENABLE;
	
	hTIM10.Init = tim10_init;
	hTIM10.Instance = TIM10;
	
	TIM_MasterConfigTypeDef tim10_master_init = {0};
	tim10_master_init.MasterOutputTrigger = TIM_TRGO_OC1;
	tim10_master_init.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	
	HAL_TIM_OC_Init(&hTIM10);
	HAL_TIM_OC_ConfigChannel(&hTIM10, &tim10_oc_init, TIM_CHANNEL_1);
	HAL_TIMEx_MasterConfigSynchronization(&hTIM10, &tim10_master_init); 
	
	// One pulse timer
	TIM_Base_InitTypeDef tim9_init = {0};
	tim9_init.Period = 42000; // 1ms period
	tim9_init.Prescaler = 3;
	tim9_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim9_init.CounterMode = TIM_COUNTERMODE_UP;
	tim9_init.RepetitionCounter = 0;
	
	TIM_OC_InitTypeDef tim9_oc_init = {0};
	tim9_oc_init.OCMode = TIM_OCMODE_PWM2;
	tim9_oc_init.Pulse = 42000-10500; // 200us pulse width
	tim9_oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
	tim9_oc_init.OCFastMode = TIM_OCFAST_ENABLE;
	
	TIM_SlaveConfigTypeDef tim9_slave_init = {0};
	tim9_slave_init.SlaveMode = TIM_SLAVEMODE_TRIGGER;
	tim9_slave_init.InputTrigger = TIM_TS_ITR2;
	tim9_slave_init.TriggerFilter = 0;
	tim9_slave_init.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
	tim9_slave_init.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	
	hTIM9.Init = tim9_init;
	hTIM9.Instance = TIM9;
	
	HAL_TIM_OC_Init(&hTIM9);
	HAL_TIM_OC_ConfigChannel(&hTIM9, &tim9_oc_init, TIM_CHANNEL_2);
	HAL_TIM_SlaveConfigSynchronization(&hTIM9, &tim9_slave_init);
	
	HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(TIM1_BRK_TIM9_IRQn);
	HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
}

void LampControlInit(void)
{
	LampControlGPIOInit();
	LampControlTIMInit();
}

void LampControlPulseStart(void)
{
	LaserStarted = true;
	
	//HAL_TIM_OC_Start_IT(&hTIM9, TIM_CHANNEL_2);
	__HAL_TIM_ENABLE_IT(&hTIM9, TIM_IT_UPDATE);
	
	/* Enable the Output compare channel */
  TIM_CCxChannelCmd(hTIM9.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);

  /* Enable the Peripheral */
  __HAL_TIM_ENABLE(&hTIM9);
	
	// Start frequency counter
	HAL_TIM_OC_Start(&hTIM10, TIM_CHANNEL_1);
}

void LampControlPulseStop(void)
{	
	LaserStarted = false;
	
	// Start frequency counter
	HAL_TIM_OC_Stop(&hTIM10, TIM_CHANNEL_1);
}

void LampSetPulseDuration(uint16_t duration)
{	
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM9);
	__HAL_TIM_SET_COUNTER(&hTIM9, 0);
	__HAL_TIM_SET_COMPARE(&hTIM9, TIM_CHANNEL_2, 42000 - duration * 42);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM9);
}

void LampSetPulseFrequency(float32_t frequency)
{
	uint16_t period = 42000.0f / frequency;
	
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM10);
	__HAL_TIM_SET_COUNTER(&hTIM10, 0);
	__HAL_TIM_SET_AUTORELOAD(&hTIM10, period);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM10);
}

