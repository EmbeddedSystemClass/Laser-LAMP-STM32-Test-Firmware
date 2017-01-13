#include "SolidStateLaser.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "GlobalVariables.h"

TIM_HandleTypeDef hTIM9;
TIM_HandleTypeDef hTIM10;
TIM_HandleTypeDef hTIM11; // Sound timer

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
		case GPIO_PIN_15:	
			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_LAMP_Ready);
			if (footswitch_en)
			{
				if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_FOOTSWITCH) == GPIO_PIN_RESET)
					footswitch_on = true;
				else
				{
					footswitch_on = false;
					__MISC_LASERLED2_OFF();
					//__MISC_LASERLED_OFF();
				}
			}
			break;
		case GPIO_PIN_3:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_HVOn);
			break;
		case GPIO_PIN_1:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_Fault);
			break;
		case GPIO_PIN_0:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_OV);
			break;
		case GPIO_PIN_14:	
			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_LAMP_OT);
			break;
		case GPIO_PIN_4:	
			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_LAMP_SIMMERSEN);
			break;
		default:
			break;
	}
}

void SolidStateLaserPulseReset(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_SOLIDSTATE:
			FlushesSessionSS = 0;
			break;
		case LASER_ID_SOLIDSTATE2:
			FlushesSessionSS2 = 0;
			break;
		case LASER_ID_LONGPULSE:
			FlushesSessionLP = 0;
			break;
		case LASER_ID_DIODELASER:
			break;
	}		
}

void SolidStateLaserPulseInc(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_SOLIDSTATE:
			FlushesSessionSS++;
			FlushesGlobalSS++;
			break;
		case LASER_ID_SOLIDSTATE2:
			FlushesSessionSS2++;
			FlushesGlobalSS2++;
			break;
		case LASER_ID_LONGPULSE:
			FlushesSessionLP++;
			FlushesGlobalLP++;
			break;
		case LASER_ID_DIODELASER:
			break;
	}		
}

uint32_t GetSolidStateGlobalPulse(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_SOLIDSTATE:
			return FlushesGlobalSS;
		case LASER_ID_SOLIDSTATE2:
			return FlushesGlobalSS2;
		case LASER_ID_LONGPULSE:
			return FlushesGlobalLP;
			break;
		case LASER_ID_DIODELASER:
			return FlushesGlobalLD;
	}		
	return 0;
}
uint32_t GetSolidStateSessionPulse(LASER_ID laser_id)
{
	switch (laser_id)
	{
		case LASER_ID_SOLIDSTATE:
			return FlushesSessionSS;
		case LASER_ID_SOLIDSTATE2:
			return FlushesSessionSS2;
		case LASER_ID_LONGPULSE:
			return FlushesSessionLP;
			break;
		case LASER_ID_DIODELASER:
			return FlushesSessionLD;
	}	
	return 0;
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &hTIM9)
	{
		if (DiodeLaser_en) 
		{
			//__MISC_LASERLED_ON();
			__MISC_LASERLED2_ON();
			
			if (Profile == PROFILE_SINGLE)
			{
				FlushesSessionLD++;
				FlushesGlobalLD++;
			}
			
		}
		else 
			__MISC_LASERLED2_OFF();
		
		if (SolidStateLaser_en) 
		{
			/*uint32_t count = convert_d(frameData_SolidStateLaser.PulseCounter);
			count++;
			frameData_SolidStateLaser.PulseCounter = convert_d(count);*/
			SolidStateLaserPulseInc(LaserID);
			
			SoundOn();
			__HAL_TIM_SET_AUTORELOAD(&hTIM11, 2100);
			HAL_TIM_Base_Start_IT(&hTIM11);
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//__HAL_TIM_DISABLE(&hTIM9);
	if (htim == &hTIM9)
	{
		if (DiodeLaser_en) 
		{
			//__MISC_LASERLED_OFF();
			__MISC_LASERLED2_OFF();
		}
		//SoundOff();
		
		subFlushes++;
		if (subFlushes == subFlushesCount)
		{
			hTIM9.Instance->CR1 &= ~(TIM_CR1_CEN);
			__HAL_TIM_SET_COUNTER(&hTIM9, 0);
			subFlushes = 0;
		}
	}
	
	if (htim == &hTIM10)
	{
		Flushes++;
		
		if (DiodeLaser_en) 
		{			
			if (Profile != PROFILE_SINGLE)
			{
				FlushesSessionLD++;
				FlushesGlobalLD++;
			}
			
			if (((FlushesSessionLD % FlushesCount) == 0) && (FlushesSessionLD > 0))
			{
				hTIM10.Instance->CR1 &= ~(TIM_CR1_CEN);
				__HAL_TIM_SET_COUNTER(&hTIM10, 0);
				Flushes = 0;
				if (Profile != PROFILE_SINGLE)
				{
					SoundOn();
					__HAL_TIM_SET_AUTORELOAD(&hTIM11, 42000);
					HAL_TIM_Base_Start_IT(&hTIM11);
				}
				//LaserStarted = false;
			}
			else
			{
				if (Profile != PROFILE_SINGLE)
				{
					SoundOn();
					__HAL_TIM_SET_AUTORELOAD(&hTIM11, 2041);//2100); // 0.05
					HAL_TIM_Base_Start_IT(&hTIM11);
				}
			}
		}
		/*if (SolidStateLaser_en) 
		{
			SolidStateLaserPulseInc(LaserID);
			
			SoundOn();
			__HAL_TIM_SET_AUTORELOAD(&hTIM11, 2100);
			HAL_TIM_Base_Start_IT(&hTIM11);
		}*/
		
		if (Flushes == FlushesCount)
		{
			hTIM10.Instance->CR1 &= ~(TIM_CR1_CEN);
			__HAL_TIM_SET_COUNTER(&hTIM10, 0);
			Flushes = 0;
			//LaserStarted = false;
		}
	}
	//HAL_TIM_OC_Stop_IT(&hTIM9, TIM_CHANNEL_2);
	//__HAL_TIM_DISABLE_IT(&hTIM9, TIM_IT_UPDATE);
	
	if (htim == &hTIM11)
	{
		HAL_TIM_Base_Stop(&hTIM11);
		SoundOff();
	}
}

void LampControlGPIOInit(void)
{
	__GPIOC_CLK_ENABLE();
	__GPIOF_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio_c = {0};
	gpio_c.Pin   = GPIO_PIN_LAMP_Ready | GPIO_PIN_LAMP_OT;
	gpio_c.Mode  = GPIO_MODE_IT_RISING_FALLING;
	gpio_c.Pull  = GPIO_NOPULL;
	gpio_c.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(GPIOC, &gpio_c);
	
	gpio_c.Pin   = GPIO_PIN_LAMP_HVINH;
	gpio_c.Mode  = GPIO_MODE_OUTPUT_PP;
	
	HAL_GPIO_Init(GPIOC, &gpio_c);
	
	GPIO_InitTypeDef gpio_f = {0};
	gpio_f.Pin   = GPIO_PIN_LAMP_HVOn | GPIO_PIN_LAMP_Fault | GPIO_PIN_LAMP_OV | GPIO_PIN_LAMP_SIMMERSEN | GPIO_PIN_FOOTSWITCH;
	gpio_f.Mode  = GPIO_MODE_IT_RISING_FALLING;
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
	__TIM11_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio_e = {0};
	gpio_e.Pin   = GPIO_PIN_5 | GPIO_PIN_6;
	gpio_e.Mode  = GPIO_MODE_AF_PP;
	gpio_e.Pull  = GPIO_NOPULL;
	gpio_e.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_e.Alternate = GPIO_AF3_TIM9;
	
	HAL_GPIO_Init(GPIOE, &gpio_e);
	
	// Sound timer
	TIM_Base_InitTypeDef tim11_init = {0};
	tim11_init.Period = 2100; // 0.05s period
	tim11_init.Prescaler = 3999;
	tim11_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim11_init.CounterMode = TIM_COUNTERMODE_UP;
	tim11_init.RepetitionCounter = 0;
	
	hTIM11.Init = tim11_init;
	hTIM11.Instance = TIM11;
	
	HAL_TIM_Base_Init(&hTIM11);
	
	HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(TIM1_TRG_COM_TIM11_IRQn);
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	
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
	tim10_oc_init.Pulse = 200;
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
	
	HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(TIM1_UP_TIM10_IRQn);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
	
	// One pulse timer
	TIM_Base_InitTypeDef tim9_init = {0};
	tim9_init.Period = 42000; // 1ms period
	tim9_init.Prescaler = 3;
	tim9_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim9_init.CounterMode = TIM_COUNTERMODE_UP;
	tim9_init.RepetitionCounter = 0;
	
	TIM_OC_InitTypeDef tim9_oc_init = {0};
	tim9_oc_init.OCMode = TIM_OCMODE_PWM2;
	tim9_oc_init.Pulse = 42000-10500; // 200us pulse width (both channels)
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
	HAL_TIM_OC_ConfigChannel(&hTIM9, &tim9_oc_init, TIM_CHANNEL_1);
	HAL_TIM_OC_ConfigChannel(&hTIM9, &tim9_oc_init, TIM_CHANNEL_2);
	HAL_TIM_SlaveConfigSynchronization(&hTIM9, &tim9_slave_init);
	
	HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(TIM1_BRK_TIM9_IRQn);
	HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
	
	//TIM1_UP_TIM10_IRQn
}

void LampControlInit(void)
{
	LampControlGPIOInit();
	LampControlTIMInit();
}

void LampControlPulseStart(void)
{
	if (!LaserStarted)
	{
		LaserStarted = true;
		
		//__MISC_LASERLED_ON();
		
		Flushes = 0;
		subFlushes = 0;
		
		__HAL_TIM_SET_COUNTER(&hTIM10, 0);
		__HAL_TIM_SET_COUNTER(&hTIM9, 0);
		
		// Start frequency counter
		HAL_TIM_OC_Start(&hTIM10, TIM_CHANNEL_1);
		__HAL_TIM_ENABLE_IT(&hTIM10, TIM_IT_UPDATE);
		
		//HAL_TIM_OC_Start_IT(&hTIM9, TIM_CHANNEL_2);
		__HAL_TIM_ENABLE_IT(&hTIM9, TIM_IT_UPDATE);
		__HAL_TIM_ENABLE_IT(&hTIM9, TIM_IT_CC2);
	
		/* Enable the Peripheral */
		//__HAL_TIM_ENABLE(&hTIM9);
		
		/* Enable the Output compare channel */
		TIM_CCxChannelCmd(hTIM9.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);
	}
}

void DiodeControlPulseStart(void)
{
	if (!LaserStarted)
	{
		LaserStarted = true;
		
		Flushes = 0;
		subFlushes = 0;
		
		__HAL_TIM_SET_COUNTER(&hTIM10, 0);
		__HAL_TIM_SET_COUNTER(&hTIM9, 0);
		
		// Start frequency counter
		HAL_TIM_OC_Start(&hTIM10, TIM_CHANNEL_1);
		__HAL_TIM_ENABLE_IT(&hTIM10, TIM_IT_UPDATE);
		
		//HAL_TIM_OC_Start_IT(&hTIM9, TIM_CHANNEL_2);
		__HAL_TIM_ENABLE_IT(&hTIM9, TIM_IT_UPDATE);
		__HAL_TIM_ENABLE_IT(&hTIM9, TIM_IT_CC1);
		
		if (Profile == PROFILE_SINGLE)
		{
			SoundOn();
			__HAL_TIM_SET_AUTORELOAD(&hTIM11, 4200);
			HAL_TIM_Base_Start_IT(&hTIM11);
		}
		else
		{
			SoundOn();
			__HAL_TIM_SET_AUTORELOAD(&hTIM11, 2100);
			HAL_TIM_Base_Start_IT(&hTIM11);
		}
	
		/* Enable the Peripheral */
		//__HAL_TIM_ENABLE(&hTIM9);
		
		/* Enable the Output compare channel */
		TIM_CCxChannelCmd(hTIM9.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
	}
}

// Deprecated
void DiodeControlOnePulseStart(void)
{
	if (!LaserStarted)
	{
		LaserStarted = true;
		
		__HAL_TIM_SET_COUNTER(&hTIM9, 0);
		
		//HAL_TIM_OC_Start_IT(&hTIM9, TIM_CHANNEL_2);
		__HAL_TIM_ENABLE_IT(&hTIM9, TIM_IT_UPDATE);
	
		/* Enable the Peripheral */
		//__HAL_TIM_ENABLE(&hTIM9);
		
		/* Enable the Output compare channel */
		TIM_CCxChannelCmd(hTIM9.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
	}
}

void LampControlPulseStop(void)
{	
	if (LaserStarted)
	{
		LaserStarted = false;
		
		// Start frequency counter
		HAL_TIM_OC_Stop(&hTIM10, TIM_CHANNEL_1);
		
		/* Disable the Output compare channel */
		TIM_CCxChannelCmd(hTIM9.Instance, TIM_CHANNEL_2, TIM_CCx_DISABLE);
		__HAL_TIM_DISABLE(&hTIM9);
		
		//__MISC_LASERLED_OFF();
		SoundOff();
	}
}

void DiodeControlPulseStop(void)
{	
	if (LaserStarted)
	{
		LaserStarted = false;
		
		// Start frequency counter
		HAL_TIM_OC_Stop(&hTIM10, TIM_CHANNEL_1);
		
		/* Disable the Output compare channel */
		TIM_CCxChannelCmd(hTIM9.Instance, TIM_CHANNEL_1, TIM_CCx_DISABLE);
		__HAL_TIM_DISABLE(&hTIM9);
		
		__MISC_LASERLED2_OFF();
		SoundOff();
	}
}

void SetPulseDuration_us(uint16_t duration)
{	
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM9);
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM10);
	__HAL_TIM_SET_PRESCALER(&hTIM9, 16-1);
	__HAL_TIM_SET_COUNTER(&hTIM9, 0);
	__HAL_TIM_SET_AUTORELOAD(&hTIM9, duration * 21); // 50% duty cycle
	//__HAL_TIM_URS_ENABLE(&hTIM9);
	hTIM9.Instance->EGR |= TIM_EGR_UG;
	__HAL_TIM_SET_COMPARE(&hTIM9, TIM_CHANNEL_1, (duration * 21)/2);
	__HAL_TIM_SET_COMPARE(&hTIM9, TIM_CHANNEL_2, (duration * 21)/2);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM9);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM10);
}

void SetPulseDuration_ms(uint16_t duration, uint16_t period)
{	
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM9);
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM10);
	__HAL_TIM_SET_PRESCALER(&hTIM9, 4000-1);
	__HAL_TIM_SET_COUNTER(&hTIM9, 0);
	__HAL_TIM_SET_AUTORELOAD(&hTIM9, period * 42); // 50% duty cycle
	//__HAL_TIM_URS_ENABLE(&hTIM9);
	hTIM9.Instance->EGR |= TIM_EGR_UG;
	__HAL_TIM_SET_COMPARE(&hTIM9, TIM_CHANNEL_1, (period-duration) * 42);
	__HAL_TIM_SET_COMPARE(&hTIM9, TIM_CHANNEL_2, (period-duration) * 42);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM9);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM10);
}

void SetPulseFrequency(float32_t frequency)
{
	uint16_t period = 42000.0f / frequency;
	
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM10);
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM9);
	__HAL_TIM_SET_PRESCALER(&hTIM10, 3999);
	__HAL_TIM_SET_COUNTER(&hTIM10, 0);
	__HAL_TIM_SET_AUTORELOAD(&hTIM10, period);
	hTIM10.Instance->EGR |= TIM_EGR_UG;
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM10);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM9);
}

void SetPulseFrequency_(float32_t frequency)
{
	uint16_t period = 42000.0f / frequency; // 10s period
	
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM10);
	if (LaserStarted) __HAL_TIM_DISABLE(&hTIM10);
	__HAL_TIM_SET_PRESCALER(&hTIM10, 39999);
	__HAL_TIM_SET_COUNTER(&hTIM10, 0);
	__HAL_TIM_SET_AUTORELOAD(&hTIM10, period);
	hTIM10.Instance->EGR |= TIM_EGR_UG;
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM10);
	if (LaserStarted) __HAL_TIM_ENABLE(&hTIM9);
}

