#ifndef USR_RTC_H_
#define USR_RTC_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * RTC calendar wrapper for MeterRecord timestamps.
 * The SysConfig RTC block runs on the default LFCLK; this library only adds
 * a small read/write convenience on top of the generated init.
 */

typedef struct {
    uint8_t seconds;    /* 0-59 */
    uint8_t minutes;    /* 0-59 */
    uint8_t hours;      /* 0-23 */
    uint8_t dayOfWeek;  /* 0-6 */
    uint8_t dayOfMonth; /* 1-31 */
    uint8_t month;      /* 1-12 */
    uint16_t year;      /* 0-4095 */
} UsrRtcTime;

void UsrRtc_Init(void);
/* True when the LFXT clock is stable (RTC calendar advances). */
bool UsrRtc_IsClockReady(void);
bool UsrRtc_GetTime(UsrRtcTime *time);
bool UsrRtc_SetTime(const UsrRtcTime *time);
/* Convenience: seconds since 2000-01-01 00:00:00 (local, no DST). */
uint32_t UsrRtc_GetUnixSeconds2000(void);

#endif /* USR_RTC_H_ */
