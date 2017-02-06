#ifndef SDCARD_H
#define SDCARD_H

#include "rl_fs.h"
#include "DGUS.h"
#include "GlobalVariables.h"

extern bool sdcard_ready;

void init_filesystem (void);
bool slog_out(DWIN_TIMEDATE date, char* data);
bool slog_out_f(DWIN_TIMEDATE date, char* format, float32_t value);
bool slog_out_i(DWIN_TIMEDATE date, char* format, int32_t value);
bool start_service_log(DWIN_TIMEDATE date);

#endif
