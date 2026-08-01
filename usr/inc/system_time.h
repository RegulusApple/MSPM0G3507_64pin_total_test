#ifndef SYSTEM_TIME_H_
#define SYSTEM_TIME_H_

#include <stdint.h>
#include <limits.h>

#include "ti_msp_dl_config.h"

static inline uint32_t SystemTime_CyclesFromUs(uint32_t microseconds)
{
    uint64_t cycles = ((uint64_t) CPUCLK_FREQ * microseconds + 999999ULL) /
        1000000ULL;

    if (cycles > UINT32_MAX) {
        return UINT32_MAX;
    }
    if ((microseconds != 0U) && (cycles == 0U)) {
        return 1U;
    }
    return (uint32_t) cycles;
}

static inline uint32_t SystemTime_CyclesFromMs(uint32_t milliseconds)
{
    uint64_t cycles = ((uint64_t) CPUCLK_FREQ * milliseconds + 999ULL) /
        1000ULL;

    return (cycles > UINT32_MAX) ? UINT32_MAX : (uint32_t) cycles;
}

static inline void SystemTime_DelayUs(uint32_t microseconds)
{
    delay_cycles(SystemTime_CyclesFromUs(microseconds));
}

static inline void SystemTime_DelayMs(uint32_t milliseconds)
{
    delay_cycles(SystemTime_CyclesFromMs(milliseconds));
}

#endif
