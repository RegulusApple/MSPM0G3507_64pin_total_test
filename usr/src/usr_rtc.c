#include "usr_rtc.h"

#include "ti_msp_dl_config.h"

#define USR_RTC_INST (RTC)

void UsrRtc_Init(void)
{
    /* Generated SYSCFG_DL_RTC_init() already ran inside Sysclk_Init(). */
}

bool UsrRtc_IsClockReady(void)
{
    /* LFXT must be stable before the RTC calendar advances. */
    return ((DL_SYSCTL_getClockStatus() & DL_SYSCTL_CLK_STATUS_LFXT_GOOD) != 0U);
}

bool UsrRtc_GetTime(UsrRtcTime *time)
{
    DL_RTC_Calendar calendar;

    if ((time == NULL) || (!UsrRtc_IsClockReady())) {
        return false;
    }
    calendar = DL_RTC_getCalendarTime(USR_RTC_INST);

    time->seconds = calendar.seconds;
    time->minutes = calendar.minutes;
    time->hours = calendar.hours;
    time->dayOfWeek = calendar.dayOfWeek;
    time->dayOfMonth = calendar.dayOfMonth;
    time->month = calendar.month;
    time->year = calendar.year;
    return true;
}

bool UsrRtc_SetTime(const UsrRtcTime *time)
{
    DL_RTC_Calendar calendar;

    if ((time == NULL) ||
        (time->seconds > 59U) || (time->minutes > 59U) ||
        (time->hours > 23U) || (time->dayOfWeek > 6U) ||
        (time->dayOfMonth < 1U) || (time->dayOfMonth > 31U) ||
        (time->month < 1U) || (time->month > 12U)) {
        return false;
    }

    calendar.seconds = time->seconds;
    calendar.minutes = time->minutes;
    calendar.hours = time->hours;
    calendar.dayOfWeek = time->dayOfWeek;
    calendar.dayOfMonth = time->dayOfMonth;
    calendar.month = time->month;
    calendar.year = time->year;
    DL_RTC_Common_initCalendar(USR_RTC_INST, calendar, DL_RTC_FORMAT_BINARY);
    return true;
}

static uint16_t UsrRtc_DaysInMonth(uint8_t month, uint16_t year)
{
    static const uint16_t days[] = {31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U};
    uint16_t d;

    if ((month < 1U) || (month > 12U)) {
        return 0U;
    }
    d = days[month - 1U];
    if ((month == 2U) &&
        (((year % 4U) == 0U && (year % 100U) != 0U) ||
         ((year % 400U) == 0U))) {
        d = 29U;
    }
    return d;
}

uint32_t UsrRtc_GetUnixSeconds2000(void)
{
    UsrRtcTime t;
    uint32_t days = 0U;
    uint16_t y;
    uint8_t m;

    if (!UsrRtc_GetTime(&t)) {
        return 0U;
    }
    /* Whole years between 2000 and t.year. */
    for (y = 2000U; y < t.year; y++) {
        days += 365U;
        if (((y % 4U) == 0U && (y % 100U) != 0U) ||
            ((y % 400U) == 0U)) {
            days += 1U;
        }
    }
    /* Whole months of the current year. */
    for (m = 1U; m < t.month; m++) {
        days += UsrRtc_DaysInMonth(m, t.year);
    }
    days += (uint32_t) (t.dayOfMonth - 1U);

    return days * 86400U +
        (uint32_t) t.hours * 3600U +
        (uint32_t) t.minutes * 60U +
        (uint32_t) t.seconds;
}
