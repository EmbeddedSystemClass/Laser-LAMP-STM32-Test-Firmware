#include "rtEventSystem.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

#include "stm32f4xx_hal.h"

uint32_t eventTimes[32];
uint32_t eventFlags;

void rtSignalSet(uint32_t signals)
{
	int i = 0;
	int p = 1;
	
	for (i = 0; i < 32; i++, p<<=1)
		if ((p & signals) != 0)
		{
			eventTimes[i] = HAL_GetTick();
			eventFlags |= p;
		}
}

void rtSignalClear(uint32_t signals)
{
	int i = 0;
	int p = 1;
	
	for (i = 0; i < 32; i++, p<<=1)
		if ((p & signals) != 0)
		{
			eventTimes[i] = HAL_GetTick();
			eventFlags &= (~p);
		}
}

bool rtSignalWait(uint32_t signals, uint32_t millisec)
{
	bool timeout;
	bool stop;
	uint32_t timout_start = HAL_GetTick();
	
	do
	{
		int i = 0;
		int p = 1;
		timeout = false;
		stop = true;
	
		for (i = 0; i < 32; i++, p<<=1)
			if ((p & signals) == 0)
				stop = false;
			
		timeout = ((HAL_GetTick() - timout_start) > millisec);
	}
	while ((!timeout) & (!stop));
	
	return timeout;
}
