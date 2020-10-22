#ifndef _RTC_H
#define _RTC_H

#ifdef  __cplusplus
extern  "C" {
#endif

#include "config.h"
#include <time.h>

#define  BKP_RTC_CFG_FLAG	 0xA5c5

#define RTC_CLOCK_SOURCE_LSE
//#define RTC_CLOCK_SOURCE_HSE
//#define RTC_CLOCK_SOURCE_LSI

extern void sysRtcHwInit(void);
extern struct tm Time_ConvUnixToCalendar(time_t t);
extern time_t Time_ConvCalendarToUnix(struct tm t);
extern time_t Time_GetUnixTime(void);
extern struct tm Time_GetCalendarTime(void);
	
extern struct tm Time_GetSystemCalendarTime(void);
	
extern void Time_SetUnixTime(time_t);
extern void Time_SetCalendarTime(struct tm t);
extern void Time_Adjust(struct tm CalTime);
#ifdef  __cplusplus
}
#endif

#endif

