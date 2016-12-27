#include "SDCard.h"

#include "rl_fs.h"
#include <stdio.h>
#include <stdarg.h>

FILE* flog;
bool sdcard_ready = false;
bool start_logging = false;

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
		flog = fopen("log.txt", "ab");
		if (flog == NULL) return false;
		fprintf(flog, "20%02d:%02d:%02d\t%02d:%02d:%02d\tSystem start\n\r", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		fclose(flog);
		
		start_logging = true;
		
		return true;
	}
	else
		return false;
}

bool log_out(DWIN_TIMEDATE date, char* format, ...)
{
	if (sdcard_ready && start_logging)
	{
		flog = fopen("log.txt", "ab");
		if (flog == NULL) return false;
		fprintf(flog, "20%02d:%02d:%02d\t%02d:%02d:%02d\t", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
		
		va_list argptr;
		va_start(argptr, format);
		fprintf(flog, format, argptr);
		va_end(argptr);
		
		fclose(flog);
		return true;
	}
	else
		return false;
}
