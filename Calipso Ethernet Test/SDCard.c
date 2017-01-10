#include "SDCard.h"

#include "rl_fs.h"
#include <stdio.h>
#include <stdarg.h>

#include "GlobalVariables.h"

FILE* flog;
FILE* fwifilog;
bool sdcard_ready = false;
bool start_logging = false;
bool start_wifilogging = false;

/* Log os objects */
void LogThread (void const *argument);

// Log
osPoolDef(Log_Events_Pool, 16, LOG_EVENT);
osPoolId pid_Log_Events_Pool;

osMessageQDef(Log_Queue, 16, uint32_t);
osMessageQId qid_Log_Queue;

osThreadId tid_LogThread;
osThreadDef (LogThread, osPriorityNormal, 1, 0);

bool log_out(DWIN_TIMEDATE date, char* data);
bool log_out_f(DWIN_TIMEDATE date, char* format, float32_t value);
bool log_out_i(DWIN_TIMEDATE date, char* format, int32_t value);

/*-----------------------------------------------------------------------------
 *        Main LOG thread
 *----------------------------------------------------------------------------*/

void LogThread (void const *argument)
{
	LOG_EVENT* rptr;
	osEvent evt;
	
	while (start_logging)
	{
		evt = osMessageGet(qid_Log_Queue, osWaitForever);	// wait for message
    if (evt.status == osEventMessage) {
      rptr = evt.value.p;
			
			switch (rptr->cmd_type)
			{
				case 0:	log_out(rptr->time, rptr->data);								// Write event to file
				break;
				case 1:	log_out_f(rptr->time, rptr->data, rptr->fvalue);	// Write event to file
				break;
				case 2:	log_out_i(rptr->time, rptr->data, rptr->ivalue);	// Write event to file								
				break;
			}
			
      osPoolFree(pid_Log_Events_Pool, rptr);					// free memory allocated for message
		}
		
		osThreadYield ();
	}
}

bool QueuePutLog(LOG_EVENT event)
{
	LOG_EVENT* pevent;
	
	pevent = osPoolAlloc(pid_Log_Events_Pool);
	
	if (pevent != NULL)
	{
		memcpy(pevent, &event, sizeof(event));
		osMessagePut(qid_Log_Queue, (uint32_t)pevent, osWaitForever);
		
		return true;
	}
	else
		return true;
}

bool LOG(uint16_t id, char* data)
{
	LOG_EVENT event;
	
	if (sdcard_ready)
	{
		event.time = datetime;
		event.cmd_type = 0;
		event.cmd_id = id;
		memcpy(event.data, data, strlen(data)+1);
		return QueuePutLog(event);
	}
	else
		return false;
}

bool LOG_F(uint16_t id, char* format, float32_t value)
{
	LOG_EVENT event;
	
	if (LOGHASH[id] == (int32_t)(value)) return false;
	LOGHASH[id] = (int32_t)(value);
	
	if (sdcard_ready)
	{
		event.time = datetime;
		event.cmd_type = 1;
		event.cmd_id = id;
		event.fvalue = value;
		memcpy(event.data, format, strlen(format)+1);
		return QueuePutLog(event);
	}
	else
		return false;
}

bool LOG_I(uint16_t id, char* format, int32_t value)
{
	LOG_EVENT event;
	
	if (LOGHASH[id] == (int32_t)(value)) return false;
	LOGHASH[id] = (int32_t)(value);
	
	if (sdcard_ready)
	{
		event.time = datetime;
		event.cmd_type = 2;
		event.cmd_id = id;
		event.ivalue = value;
		memcpy(event.data, format, strlen(format)+1);
		return QueuePutLog(event);
	}
	else
		return false;
}

/*-----------------------------------------------------------------------------
 *        Extract drive specification from the input string
 *----------------------------------------------------------------------------*/
static char *get_drive (char *src, char *dst, uint32_t dst_sz) {
  uint32_t i, n;

  i = 0;
  n = 0;
  while (!n && src && src[i] && (i < dst_sz)) {
    dst[i] = src[i];

    if (dst[i] == ':') {
      n = i + 1;
    }
    i++;
  }
  if (n == dst_sz) {
    n = 0;
  }
  dst[n] = '\0';

  return (src + n);
}

/*-----------------------------------------------------------------------------
 *        Format Device
 *----------------------------------------------------------------------------*/
static void cmd_format (char *par) {
  char  label[12];
  char  drive[4];
  int   retv;

  par = get_drive (par, drive, 4);

  printf ("\nProceed with Format [Y/N]\n");
  retv = getchar();
  if (retv == 'y' || retv == 'Y') {
    /* Format the drive */
    if (fformat (drive, par) == fsOK) {
      printf ("Format completed.\n");
      if (fvol (drive, label, NULL) == 0) {
        if (label[0] != '\0') {
          printf ("Volume label is \"%s\"\n", label);
        }
      }
    }
    else {
      printf ("Formatting failed.\n");
			sdcard_ready = false;
    }
  }
  else {
    printf ("Formatting canceled.\n");
		sdcard_ready = false;
  }
}

/*-----------------------------------------------------------------------------
 *        Initialize a Flash Memory Card
 *----------------------------------------------------------------------------*/
void init_filesystem (void)
{
	fsStatus stat;
	
	stat = finit ("M0:");
  if (stat == fsOK) {
    stat = fmount ("M0:");
    if (stat == fsOK) {
      printf ("Drive M0 ready!\n");
			sdcard_ready = true;
    }
    else if (stat == fsNoFileSystem) {
      /* Format the drive */
      printf ("Drive M0 not formatted!\n");
      cmd_format ("M0:");
    }
    else {
      printf ("Drive M0 mount failed with error code %d\n", stat);
			sdcard_ready = false;
    }
  }
  else {
    printf ("Drive M0 initialization failed!\n");
		sdcard_ready = false;
  }
}

bool start_log(DWIN_TIMEDATE date)
{
	if (sdcard_ready)
	{
		// Add event to log file
		flog = fopen("log.txt", "ab");
		if (flog == NULL) return false;
		fprintf(flog, "20%02d:%02d:%02d\t%02d:%02d:%02d\tSystem start\n\r", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		fclose(flog);
		
		// Start logging
		start_logging = true;
		
		// Create Log queue
		qid_Log_Queue = osMessageCreate(osMessageQ(Log_Queue), NULL);
		if (!qid_Log_Queue) return(-1);
			
		// Create Log Pool
		pid_Log_Events_Pool = osPoolCreate(osPool(Log_Events_Pool));
		
		// Start logging thread
		tid_LogThread = osThreadCreate (osThread(LogThread), NULL);
		if (!tid_LogThread) return(-1);
		
		return true;
	}
	
	return false;
}

bool log_out(DWIN_TIMEDATE date, char* data)
{
	if (sdcard_ready && start_logging)
	{
		flog = fopen("log.txt", "ab");
		if (flog == NULL) return false;
		fprintf(flog, "20%02d:%02d:%02d\t%02d:%02d:%02d\t", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		
		fprintf(flog, data);
		
		fclose(flog);
		return true;
	}
	else
		return false;
}

bool log_out_f(DWIN_TIMEDATE date, char* format, float32_t value)
{
	if (sdcard_ready && start_logging)
	{
		flog = fopen("log.txt", "ab");
		if (flog == NULL) return false;
		fprintf(flog, "20%02d:%02d:%02d\t%02d:%02d:%02d\t", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		
		fprintf(flog, format, value);
		
		fclose(flog);
		return true;
	}
	else
		return false;
}

bool log_out_i(DWIN_TIMEDATE date, char* format, int32_t value)
{
	if (sdcard_ready && start_logging)
	{
		flog = fopen("log.txt", "ab");
		if (flog == NULL) return false;
		fprintf(flog, "20%02d:%02d:%02d\t%02d:%02d:%02d\t", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		
		fprintf(flog, format, value);
		
		fclose(flog);
		return true;
	}
	else
		return false;
}

bool start_wifi(DWIN_TIMEDATE date)
{
	if (sdcard_ready)
	{
		fwifilog = fopen("wifi_log.txt", "ab");
		if (fwifilog == NULL) return false;
		fprintf(fwifilog, "\r20%02d:%02d:%02d\t%02d:%02d:%02d\tWifi start\n\r", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		fclose(fwifilog);
		
		start_wifilogging = true;
		
		return true;
	}
	else
		return false;
}

bool log_wifi(DWIN_TIMEDATE date, char* str)
{
	if (sdcard_ready && start_wifilogging)
	{
		fwifilog = fopen("wifi_log.txt", "ab");
		if (fwifilog == NULL) return false;
		fprintf(fwifilog, "20%02d:%02d:%02d\t%02d:%02d:%02d\t%s\n", date.year, date.month, date.day, date.hours, date.minutes, date.seconds, str);
		
		fclose(fwifilog);
		return true;
	}
	else
		return false;
}
