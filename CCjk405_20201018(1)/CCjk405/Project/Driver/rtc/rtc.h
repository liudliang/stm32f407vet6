#ifndef _RTC_H
#define _RTC_H

#ifdef  __cplusplus
extern  "C" {
#endif

#include "config.h"
#include <time.h>

#define  BKP_RTC_CFG_FLAG	 0xA5c5

#define RTC_CLOCK_SOURCE_LSE

typedef struct {
    uint8_t seconds;
    uint16_t milliseconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
    uint32_t unix;
}TM_RTC_Time_t;


extern void sysRtcHwInit(void);
extern struct tm Time_GetSystemCalendarTime(void);
extern ErrorStatus RTC_SetDateTime(struct tm CalTime);
extern time_t Time_GetUnixTime(void);
	
extern TM_RTC_Time_t Time_GetRtcTime(void);
#ifdef  __cplusplus
}
#endif

#endif

