#include "freq_meter.h"

#include "ti_msp_dl_config.h"

#define FREQ_METER_CLK_HZ      (CPUCLK_FREQ)
#define FREQ_METER_LOAD_VALUE  (FREQ_CAPTURE_INST_LOAD_VALUE)

/*
 * PERIOD_CAPTURE (down counting): the timer counts down from LOAD at
 * 32 MHz.  On every input edge the current counter value is latched into
 * CC0 (CC0_DN interrupt).  The period between two consecutive edges equals
 * the unsigned difference of the two captured values (hardware measures
 * edge-to-edge, so ISR latency does not affect the result).
 */

static volatile uint32_t gFreqMeterLastCapture;
static volatile bool gFreqMeterSynced;
static volatile uint32_t gFreqMeterEdgeCount;
static volatile bool gFreqMeterValid;
static volatile uint32_t gFreqMeterPeriodTicks;
static volatile bool gFreqMeterLost;

void FreqMeter_Init(void)
{
    gFreqMeterLastCapture = 0U;
    gFreqMeterSynced = false;
    gFreqMeterEdgeCount = 0U;
    gFreqMeterValid = false;
    gFreqMeterPeriodTicks = 0U;
    gFreqMeterLost = false;

    /* Capture peripheral is configured by SysConfig; keep the counter and
     * its interrupt OFF until FreqMeter_Start().  This prevents a floating
     * input from generating an interrupt storm that stalls the main loop. */
    NVIC_DisableIRQ(FREQ_CAPTURE_INST_INT_IRQN);
    DL_TimerG_stopCounter(FREQ_CAPTURE_INST);
}

void FreqMeter_Start(void)
{
    NVIC_ClearPendingIRQ(FREQ_CAPTURE_INST_INT_IRQN);
    NVIC_EnableIRQ(FREQ_CAPTURE_INST_INT_IRQN);
    DL_TimerG_startCounter(FREQ_CAPTURE_INST);
}

void FreqMeter_Stop(void)
{
    DL_TimerG_stopCounter(FREQ_CAPTURE_INST);
    NVIC_DisableIRQ(FREQ_CAPTURE_INST_INT_IRQN);
}

void FreqMeter_Reset(void)
{
    gFreqMeterSynced = false;
    gFreqMeterEdgeCount = 0U;
    gFreqMeterValid = false;
    gFreqMeterPeriodTicks = 0U;
    gFreqMeterLost = false;
}

bool FreqMeter_Read(FreqMeterResult *result)
{
    uint32_t period;

    if (result == NULL) {
        return false;
    }
    result->valid = gFreqMeterValid;
    result->lastPeriodTicks = gFreqMeterPeriodTicks;
    result->edgeCount = gFreqMeterEdgeCount;
    period = gFreqMeterPeriodTicks;
    if ((period == 0U) || (!gFreqMeterValid)) {
        result->frequencyHz = 0.0f;
        return false;
    }
    result->frequencyHz = (float) FREQ_METER_CLK_HZ / (float) period;
    return true;
}

bool FreqMeter_WasLost(void)
{
    return gFreqMeterLost;
}

void FREQ_CAPTURE_INST_IRQHandler(void)
{
    uint32_t captureValue;
    uint32_t delta;

    switch (DL_TimerG_getPendingInterrupt(FREQ_CAPTURE_INST)) {
        case DL_TIMERG_IIDX_CC0_DN:
            captureValue = DL_Timer_getCaptureCompareValue(FREQ_CAPTURE_INST,
                DL_TIMER_CC_0_INDEX);
            if (gFreqMeterSynced) {
                /* Edge-to-edge period with 32-bit wraparound handled by
                 * unsigned subtraction. */
                delta = (gFreqMeterLastCapture - captureValue);
                gFreqMeterPeriodTicks = delta;
                gFreqMeterValid = (delta > 0U);
            }
            gFreqMeterLastCapture = captureValue;
            gFreqMeterSynced = true;
            gFreqMeterEdgeCount++;
            break;
        case DL_TIMERG_IIDX_ZERO:
            /* Counter wrapped with no edge: input lost or too slow.
             * Re-synchronize on the next edge. */
            gFreqMeterSynced = false;
            gFreqMeterValid = false;
            gFreqMeterLost = true;
            break;
        default:
            break;
    }
}
