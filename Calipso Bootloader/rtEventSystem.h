#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

void rtSignalSet(uint32_t signals);
void rtSignalClear(uint32_t signals);
bool rtSignalWait(uint32_t signals, uint32_t millisec);
