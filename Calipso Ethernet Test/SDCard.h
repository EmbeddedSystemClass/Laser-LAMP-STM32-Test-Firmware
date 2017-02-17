#ifndef SDCARD_H
#define SDCARD_H

#include "rl_fs.h"
#include "DGUS.h"
#include "GlobalVariables.h"

extern bool sdcard_ready;

typedef struct __attribute__((__packed__)) LOG_ELEMENT_STRUCT {
	char log_line[64];
} LOG_ELEMENT, *PLOG_ELEMENT;

extern LOG_ELEMENT log_table[12];

void init_filesystem (void);
bool start_log(DWIN_TIMEDATE date);
bool start_slog(DWIN_TIMEDATE date);
bool LOG(uint16_t id, char* data);
bool LOG_F(uint16_t id, char* format, float32_t value);
bool LOG_I(uint16_t id, char* format, int32_t value);
bool LOG_UPDATE_TABLE(int32_t offset);
bool SLOG(char* data);
bool SLOG_F(char* format, float32_t value);
bool SLOG_I(char* format, int32_t value);
bool log_wifi(DWIN_TIMEDATE date, char* str);

void log_LaserDiodeStart(float32_t freq, float32_t duration, float32_t energy, uint32_t counter);
void log_LaserSSStart(float32_t freq, float32_t duration, float32_t energy, uint32_t counter);
void log_LongPulseStart(float32_t freq, float32_t duration, float32_t energy, uint32_t counter);
void log_FractLaserStart(float32_t freq, float32_t duration, float32_t energy, uint32_t counter);

void log_LaserDiodeStop(uint32_t counter);
void log_LaserSSStop(uint32_t counter);
void log_LongPulseStop(uint32_t counter);
void log_FractLaserStop(uint32_t counter);

#endif
