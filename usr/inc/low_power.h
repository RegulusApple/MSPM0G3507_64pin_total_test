#ifndef LOW_POWER_H_
#define LOW_POWER_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    WAKE_REASON_NONE = 0,
    WAKE_REASON_TIMER,
    WAKE_REASON_EXTERNAL
} WakeReason;

typedef struct {
    uint32_t sequence;
    uint8_t addrCode;
    uint16_t sampleCount;
    float rmsMv;
    float currentMa;
    bool valid;
} LowPowerMeasurement;

void LowPower_Init(void);
WakeReason LowPower_GetWakeReason(void);
void LowPower_ClearWakeReason(void);
uint32_t LowPower_GetWakeTimerTicks(void);
void LowPower_RunTimerMeasurementFlow(void);
void LowPower_RunTimerWakeFlow(void);
void LowPower_RunExternalWakeFlow(void);
void LowPower_EnterStandby1(void);

const LowPowerMeasurement *LowPower_GetLatestMeasurement(void);

#endif
