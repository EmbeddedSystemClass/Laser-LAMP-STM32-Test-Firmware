#ifndef SDCARD_H
#define SDCARD_H

#include "rl_fs.h"
#include "DGUS.h"
#include "GlobalVariables.h"

extern bool sdcard_ready;

void init_filesystem (void);
bool start_log(DWIN_TIMEDATE date);
bool LOG(uint16_t id, char* data);
bool LOG_F(uint16_t id, char* format, float32_t value);
bool LOG_I(uint16_t id, char* format, int32_t value);

bool start_wifi(DWIN_TIMEDATE date);
bool log_wifi(DWIN_TIMEDATE date, char* str);

#endif
