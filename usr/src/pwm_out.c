#include "pwm_out.h"

#include "ti_msp_dl_config.h"

#define PWM_OUT_CLK_HZ        (PWM_TIMER_INST_CLK_FREQ)
#define PWM_OUT_TIMER_MAX     (65535U)
#define PWM_OUT_CC_FULLSCALE  (32768U)

static volatile bool gPwmOutRunning;
static float gPwmOutDutyPercent;

void PwmOut_Init(void)
{
    gPwmOutRunning = false;
    gPwmOutDutyPercent = 50.0f;
}

bool PwmOut_SetFreqHz(uint32_t frequencyHz)
{
    uint32_t count;

    if ((frequencyHz == 0U) || (frequencyHz > (PWM_OUT_CLK_HZ / 2U))) {
        return false;
    }
    count = PWM_OUT_CLK_HZ / frequencyHz;
    if (count < 2U) {
        count = 2U;
    }
    if (count > PWM_OUT_TIMER_MAX) {
        count = PWM_OUT_TIMER_MAX;
    }

    DL_TimerA_setLoadValue(PWM_TIMER_INST, count - 1U);
    /* Re-apply current duty on the new period. */
    PwmOut_SetDutyPercent(gPwmOutDutyPercent);
    return true;
}

uint32_t PwmOut_GetFreqHz(void)
{
    uint32_t count = DL_TimerA_getLoadValue(PWM_TIMER_INST) + 1U;
    return PWM_OUT_CLK_HZ / count;
}

bool PwmOut_SetDutyPercent(float percent)
{
    uint32_t count;
    uint32_t ccValue;

    if ((percent < 0.0f) || (percent > 100.0f)) {
        return false;
    }
    gPwmOutDutyPercent = percent;
    count = DL_TimerA_getLoadValue(PWM_TIMER_INST) + 1U;
    ccValue = (uint32_t) ((float) count * percent / 100.0f);
    if (ccValue >= count) {
        ccValue = count - 1U;
    }
    DL_Timer_setCaptureCompareValue(PWM_TIMER_INST, ccValue,
        DL_TIMER_CC_1_INDEX);
    return true;
}

float PwmOut_GetDutyPercent(void)
{
    return gPwmOutDutyPercent;
}

void PwmOut_Start(void)
{
    DL_TimerA_startCounter(PWM_TIMER_INST);
    gPwmOutRunning = true;
}

void PwmOut_Stop(void)
{
    DL_TimerA_stopCounter(PWM_TIMER_INST);
    gPwmOutRunning = false;
}

bool PwmOut_IsRunning(void)
{
    return gPwmOutRunning;
}

float PwmOut_SampleToDuty(int16_t sample, uint16_t fullScale)
{
    float normalized;

    if (fullScale == 0U) {
        fullScale = PWM_OUT_CC_FULLSCALE;
    }
    normalized = (float) sample / (float) fullScale;   /* -1 .. +1 */
    if (normalized > 1.0f) {
        normalized = 1.0f;
    }
    if (normalized < -1.0f) {
        normalized = -1.0f;
    }
    /* Map -1..+1 onto 10%..90% duty (stay away from 0/100). */
    return 50.0f + normalized * 40.0f;
}
