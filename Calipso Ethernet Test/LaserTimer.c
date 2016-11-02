
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include <stdbool.h>
#include "GlobalVariables.h"
#include "SolidStateLaser.h"

/*----------------------------------------------------------------------------
 *      Timer: Sample timer functions
 *---------------------------------------------------------------------------*/
 
 extern TIM_HandleTypeDef htim_flow1;
 extern TIM_HandleTypeDef htim_flow2;


/*----- Periodic Timer Example -----*/
static void LaserTimer_Callback (void const *arg);              // prototype for timer callback function

static osTimerId idLaserTimer;                                  // timer id
static osTimerDef (Timer2, LaserTimer_Callback);
 
// Periodic Timer Example
static void LaserTimer_Callback(void const *arg) {
	if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_FOOTSWITCH) == GPIO_PIN_RESET)
		footswitch_on = true;
	else
		footswitch_on = false;
				
	if (footswitch_on /*&& footswitch_en*/)
	{
		switch_filter++;
		if (switch_filter > switch_filter_threshold * 2)
			switch_filter = switch_filter_threshold * 2;
	}
	else
	{
		if (switch_filter > 0)
			switch_filter--;
	}
	
	if (footswitch_en)
	{
		if (SolidStateLaser_en)
		{
			if (switch_filter > switch_filter_threshold)
				LampControlPulseStart();
			else
				LampControlPulseStop();
		}
		
		if (DiodeLaser_en)
		{
			if (switch_filter > switch_filter_threshold)
				DiodeControlPulseStart();
			else
				DiodeControlPulseStop();
		}
		
		if (DiodeLaserOnePulse_en)
			if (switch_filter > switch_filter_threshold)
				DiodeControlOnePulseStart();
	}
	
		static int cnt = 0;
	cnt++;
	
	// Flow measure
	if (cnt > 10)
	{
		HAL_TIM_Base_Stop(&htim_flow1);
		HAL_TIM_Base_Stop(&htim_flow2);
		flow1 = (float32_t)(__HAL_TIM_GET_COUNTER(&htim_flow1)) * 600.0f / 1500.0f;
		flow2 = (float32_t)(__HAL_TIM_GET_COUNTER(&htim_flow2)) * 600.0f / 1500.0f;
		__HAL_TIM_SET_COUNTER(&htim_flow1, 0);
		__HAL_TIM_SET_COUNTER(&htim_flow2, 0);
		HAL_TIM_Base_Start(&htim_flow1);
		HAL_TIM_Base_Start(&htim_flow2);
		cnt = 0;
	}
	
  // add user code here
	if (g_peltier_en)
	{
		if (m_wMillSec == 0)
		{			
			if (m_wSeconds == 0)
			{
				if (m_wMinutes == 0)
				{
					//OnTimeout();
					prepare = false;
					return;
				}
				m_wSeconds = 60;
				if (m_wMinutes > 0)	m_wMinutes--;
			}
			m_wMillSec = 1000; // Every 10 ms
			if (m_wSeconds > 0)	m_wSeconds--;
		}
		if (m_wMillSec > 0)	m_wMillSec-=10;
	}
	
	if (!g_peltier_en)
	{
		if (m_wMillSec >= 700)
		{
			if ((m_wSeconds >= m_wSetSec) && (m_wMinutes >= m_wSetMin))
			{
				m_wSeconds = m_wSetSec;
				m_wMinutes = m_wSetMin;
				m_wMillSec = 0;
			}
			else
			{
				if (m_wSeconds == 60)
				{
					if (m_wMinutes >= m_wSetMin)
						m_wMinutes = m_wSetMin;
					m_wSeconds = 0;
					if (m_wMinutes < 60)	m_wMinutes++;
				}
				if (m_wSeconds < 60) m_wSeconds++;
			}
			m_wMillSec = 0; // Every 10 ms
		}
		if (m_wMillSec < 1000)	m_wMillSec += 10;
	}
}


// Example: Create and Start timers
void Init_Timers (void) {
  osStatus status;                                              // function return status
	
	/*peltier_en = false;
	m_wMillSec = 0;
	m_wSeconds = 0;
	m_wMinutes = 0;
	m_wSetSec  = 0;
	m_wSetMin  = 0;*/
 
  // Create periodic timer
  idLaserTimer = osTimerCreate (osTimer(Timer2), osTimerPeriodic, NULL);
  if (idLaserTimer != NULL) {    // Periodic timer created
    // start timer with periodic 10ms interval
    status = osTimerStart (idLaserTimer, 10);            
    if (status != osOK) {
      // Timer could not be started
    }
  }
}
