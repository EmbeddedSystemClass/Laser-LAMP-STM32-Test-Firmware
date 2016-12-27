#ifndef SDCARD_H
#define SDCARD_H

#include "rl_fs.h"
#include "DGUS.h"

extern bool sdcard_ready;

void init_filesystem (void);
bool start_log(DWIN_TIMEDATE date);
bool log_out(DWIN_TIMEDATE date, char* format, ...);

#endif
