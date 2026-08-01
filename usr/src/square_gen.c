#include "square_gen.h"

#include "ti_msp_dl_config.h"

#define SQUARE_GEN_CLK_HZ      (SQUARE_TIMER_INST_CLK_FREQ)
#define SQUARE_GEN_TIMER_MAX   (65535U)

static volatile bool gSquareGenRunning;

void SquareGen_Init(void)
{
    gSquareGenRunning = false;
}

bool SquareGen_SetFreqHz(uint32_t frequencyHz)
{
    uint32_t count;

    if ((frequencyHz == 0U) || (frequencyHz > (SQUARE_GEN_CLK_HZ / 2U))) {
        return false;
    }
    count = SQUARE_GEN_CLK_HZ / frequencyHz;
    if (count < 2U) {
        count = 2U;
    }
    if (count > SQUARE_GEN_TIMER_MAX) {
        count = SQUARE_GEN_TIMER_MAX;
    }

    DL_TimerG_setLoadValue(SQUARE_TIMER_INST, count - 1U);
    /* Keep 50% duty: CC0 = half of load. */
    DL_Timer_setCaptureCompareValue(SQUARE_TIMER_INST,
        (count / 2U) - 1U, DL_TIMER_CC_0_INDEX);
    return true;
}

uint32_t SquareGen_GetFreqHz(void)
{
    uint32_t count = DL_TimerG_getLoadValue(SQUARE_TIMER_INST) + 1U;
    return SQUARE_GEN_CLK_HZ / count;
}

void SquareGen_Start(void)
{
    DL_TimerG_startCounter(SQUARE_TIMER_INST);
    gSquareGenRunning = true;
}

void SquareGen_Stop(void)
{
    DL_TimerG_stopCounter(SQUARE_TIMER_INST);
    gSquareGenRunning = false;
}

bool SquareGen_IsRunning(void)
{
    return gSquareGenRunning;
}
