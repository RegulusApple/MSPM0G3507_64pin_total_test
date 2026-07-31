#include "dac.h"
#include "usr_uart.h"
#include <math.h>

#define DAC_SINE_AMPLITUDE_CODE (1800U)
#define DAC_SINE_CENTER_CODE    (2048U)
#define DAC_SINE_AMPLITUDE_MAX_MV (DAC12_REF_MV / 2U)
#define DAC_TWO_PI              (6.28318530718f)
#define DAC_SINE_MAX_HZ         (DAC_SINE_FS_MAX_HZ / DAC_SINE_TABLE_MIN)
#define DAC_TIMER_MAX_PRESCALE_PLUS1 (256U)
#define DAC_TIMER_MAX_PERIOD_PLUS1   (65536U)
#define DAC_SINE_SAMPLE_TIMER_HZ     (1000000U)

static uint16_t gDACCode = DAC12_DEFAULT_CODE;
static uint16_t gDACSineAmplitudeCode = DAC_SINE_AMPLITUDE_CODE;
static uint16_t gDACSineBuffer[DAC_SINE_TABLE_MAX];
static uint32_t gDACSineRequestedHz = 0U;
static uint32_t gDACSineActualHz = 0U;
static uint32_t gDACSineSampleRateHz = 0U;
static uint16_t gDACSineTableLen = 0U;
static bool gDACSineRunning = false;

static uint16_t DAC_clampCode(uint16_t code)
{
    if (code > DAC_CODE_MAX) {
        return DAC_CODE_MAX;
    }

    return code;
}

void DAC_Init(void)
{
    DAC_SetCode12(DAC12_DEFAULT_CODE);
}

void DAC_SetCode12(uint16_t code)
{
    DL_TimerA_stopCounter(DAC_TIMER_INST);
    DL_DAC12_disableSampleTimeGenerator(DAC12_INST);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DAC12_disableDMATrigger(DAC12_INST);
    DL_DAC12_disableFIFO(DAC12_INST);
    gDACSineRunning = false;
    gDACCode = DAC_clampCode(code);
    DL_DAC12_output12(DAC12_INST, gDACCode);
}

void DAC_SetVoltageMv(uint16_t millivolt)
{
    uint32_t code;

    if (millivolt > DAC12_REF_MV) {
        millivolt = DAC12_REF_MV;
    }

    code = ((uint32_t) millivolt * DAC_CODE_MAX + (DAC12_REF_MV / 2U)) / DAC12_REF_MV;
    DAC_SetCode12((uint16_t) code);
}

void DAC_SetSineAmplitudeMv(uint16_t millivolt)
{
    uint32_t code;
    uint32_t frequencyHz;

    if (millivolt > DAC_SINE_AMPLITUDE_MAX_MV) {
        millivolt = DAC_SINE_AMPLITUDE_MAX_MV;
    }

    code = ((uint32_t) millivolt * DAC_CODE_MAX + (DAC12_REF_MV / 2U)) /
        DAC12_REF_MV;
    gDACSineAmplitudeCode = (uint16_t) code;

    if (gDACSineRunning) {
        frequencyHz = gDACSineRequestedHz;
        DAC_StopSine();
        (void) DAC_StartSineHz(frequencyHz);
    }
}

uint16_t DAC_GetCode12(void)
{
    return gDACCode;
}

uint16_t DAC_GetVoltageMv(void)
{
    return (uint16_t) (((uint32_t) gDACCode * DAC12_REF_MV + (DAC_CODE_MAX / 2U)) /
                       DAC_CODE_MAX);
}

const char *DAC_GetOutputPinName(void)
{
    return DAC_OUTPUT_PIN_NAME;
}

static uint16_t DAC_selectSineTableLen(uint32_t freqHz)
{
    uint32_t tableLen = (DAC_SINE_SAMPLE_TIMER_HZ + (freqHz / 2U)) / freqHz;

    if (tableLen > DAC_SINE_TABLE_MAX) return DAC_SINE_TABLE_MAX;
    if (tableLen < DAC_SINE_TABLE_MIN) return DAC_SINE_TABLE_MIN;
    return (uint16_t) tableLen;
}

static void DAC_buildSineTable(uint16_t tableLen)
{
    uint16_t i;

    for (i = 0U; i < tableLen; i++) {
        float theta = DAC_TWO_PI * (float) i / (float) tableLen;
        int32_t code = (int32_t) DAC_SINE_CENTER_CODE +
            (int32_t) ((float) gDACSineAmplitudeCode * sin(theta));

        if (code < 0) {
            code = 0;
        }
        if (code > (int32_t) DAC_CODE_MAX) {
            code = DAC_CODE_MAX;
        }
        gDACSineBuffer[i] = (uint16_t) code;
    }
}

static void DAC_setTimerSampleRate(uint32_t sampleRateHz)
{
    uint32_t divider;
    uint32_t prescalePlus1;
    uint32_t periodPlus1;
    DL_TimerA_ClockConfig clockConfig = {
        .clockSel    = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale    = 0U,
    };

    if (sampleRateHz == 0U) {
        sampleRateHz = DAC_SINE_DEFAULT_HZ * DAC_SINE_TABLE_MIN;
    }
    if (sampleRateHz > DAC_SINE_FS_MAX_HZ) {
        sampleRateHz = DAC_SINE_FS_MAX_HZ;
    }

    divider = (CPUCLK_FREQ + (sampleRateHz / 2U)) / sampleRateHz;
    if (divider < 1U) {
        divider = 1U;
    }
    if (divider > (DAC_TIMER_MAX_PRESCALE_PLUS1 * DAC_TIMER_MAX_PERIOD_PLUS1)) {
        divider = DAC_TIMER_MAX_PRESCALE_PLUS1 * DAC_TIMER_MAX_PERIOD_PLUS1;
    }

    prescalePlus1 = (divider + (DAC_TIMER_MAX_PERIOD_PLUS1 - 1U)) /
        DAC_TIMER_MAX_PERIOD_PLUS1;
    if (prescalePlus1 < 1U) {
        prescalePlus1 = 1U;
    }
    if (prescalePlus1 > DAC_TIMER_MAX_PRESCALE_PLUS1) {
        prescalePlus1 = DAC_TIMER_MAX_PRESCALE_PLUS1;
    }

    periodPlus1 = divider / prescalePlus1;
    if (periodPlus1 < 1U) {
        periodPlus1 = 1U;
    }
    if (periodPlus1 > DAC_TIMER_MAX_PERIOD_PLUS1) {
        periodPlus1 = DAC_TIMER_MAX_PERIOD_PLUS1;
    }

    gDACSineSampleRateHz = CPUCLK_FREQ / (prescalePlus1 * periodPlus1);

    DL_TimerA_stopCounter(DAC_TIMER_INST);
    clockConfig.prescale = (uint8_t) (prescalePlus1 - 1U);
    DL_TimerA_setClockConfig(DAC_TIMER_INST, &clockConfig);
    DL_TimerA_setLoadValue(DAC_TIMER_INST, (uint16_t) (periodPlus1 - 1U));
    DL_TimerA_setTimerCount(DAC_TIMER_INST, (uint16_t) (periodPlus1 - 1U));
}

bool DAC_StartSineHz(uint32_t freqHz)
{
    uint32_t sampleRateHz;
    uint16_t i;

    if ((freqHz == 0U) || (freqHz > DAC_SINE_MAX_HZ)) {
        freqHz = DAC_SINE_DEFAULT_HZ;
    }

    gDACSineRequestedHz = freqHz;
    gDACSineTableLen = DAC_selectSineTableLen(freqHz);
    if ((gDACSineTableLen < DAC_SINE_TABLE_MIN) ||
        (gDACSineTableLen > DAC_SINE_TABLE_MAX)) {
        gDACSineTableLen = DAC_SINE_TABLE_MIN;
    }
    sampleRateHz = freqHz * gDACSineTableLen;
    if (sampleRateHz > DAC_SINE_FS_MAX_HZ) {
        sampleRateHz = DAC_SINE_FS_MAX_HZ;
    }

    DAC_buildSineTable(gDACSineTableLen);
    (void) sampleRateHz;
    gDACSineSampleRateHz = DAC_SINE_SAMPLE_TIMER_HZ;
    gDACSineActualHz = gDACSineSampleRateHz / gDACSineTableLen;

    DL_TimerA_stopCounter(DAC_TIMER_INST);
    DL_DAC12_disableSampleTimeGenerator(DAC12_INST);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DMA_setTrigger(DMA, DMA_DAC_CHAN_ID, DMA_DAC0_EVT_BD_1_TRIG,
        DL_DMA_TRIGGER_TYPE_EXTERNAL);
    DL_DMA_setSrcIncrement(DMA, DMA_DAC_CHAN_ID, DL_DMA_ADDR_INCREMENT);
    DL_DMA_setDestIncrement(DMA, DMA_DAC_CHAN_ID, DL_DMA_ADDR_UNCHANGED);
    DL_TimerA_enableEvent(DAC_TIMER_INST, DL_TIMERA_EVENT_ROUTE_1,
        DL_TIMERA_EVENT_ZERO_EVENT);
    DL_TimerA_setPublisherChanID(DAC_TIMER_INST, DL_TIMERA_PUBLISHER_INDEX_0, 3U);
    DL_DAC12_setSubscriberChanID(DAC12_INST, DL_DAC12_SUBSCRIBER_INDEX_0, 3U);
    DL_DAC12_clearInterruptStatus(DAC12_INST, 0xFFFFFFFFU);

    DL_DAC12_enableFIFO(DAC12_INST);
    DL_DAC12_setFIFOTriggerSource(DAC12_INST, DL_DAC12_FIFO_TRIGGER_SAMPLETIMER);
    DL_DAC12_setFIFOThreshold(DAC12_INST, DL_DAC12_FIFO_THRESHOLD_TWO_QTRS_EMPTY);
    DL_DAC12_enable(DAC12_INST);

    /*
     * The DMA request is generated when FIFO empty slots reach the selected
     * threshold.  A newly enabled, empty FIFO does not necessarily create a
     * request, so preload it before enabling DMA requests.  The timer then
     * consumes two samples and reliably reaches the half-empty threshold.
     */
    DL_DAC12_disableDMATrigger(DAC12_INST);
    for (i = 0U; i < 4U; i++) {
        DL_DAC12_output12(DAC12_INST, gDACSineBuffer[i]);
    }

    DL_DMA_setSrcAddr(DMA, DMA_DAC_CHAN_ID, (uint32_t) &gDACSineBuffer[0]);
    DL_DMA_setDestAddr(DMA, DMA_DAC_CHAN_ID, (uint32_t) &DAC12_INST->DATA0);
    DL_DMA_setTransferSize(DMA, DMA_DAC_CHAN_ID, gDACSineTableLen);
    DL_DMA_enableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DAC12_enableDMATrigger(DAC12_INST);
    DL_DAC12_enableSampleTimeGenerator(DAC12_INST);
    gDACSineRunning = true;
    return true;
}

void DAC_StopSine(void)
{
    if (gDACSineRunning == false) {
        return;
    }

    DL_TimerA_stopCounter(DAC_TIMER_INST);
    DL_DAC12_disableSampleTimeGenerator(DAC12_INST);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DAC12_disableDMATrigger(DAC12_INST);
    gDACSineRunning = false;
}

uint32_t DAC_GetSineRequestedHz(void)
{
    return gDACSineRequestedHz;
}

uint32_t DAC_GetSineActualHz(void)
{
    return gDACSineActualHz;
}

uint32_t DAC_GetSineSampleRateHz(void)
{
    return gDACSineSampleRateHz;
}

uint16_t DAC_GetSineTableLen(void)
{
    return gDACSineTableLen;
}

void DAC_PrintDebug(const char *tag)
{
    USR_UART_printf("DBG:%s,dma_en=%u,dma_sz=%u,timer_run=%u,timer_cnt=%u,dac_en=%u,fifo_en=%u,fifo_full=%u,ctl2=0x%08X,ctl3=0x%08X,data0=0x%08X,fsub0=%u,tpub0=%u\r\n",
        tag,
        DL_DMA_isChannelEnabled(DMA, DMA_DAC_CHAN_ID),
        DL_DMA_getTransferSize(DMA, DMA_DAC_CHAN_ID),
        DL_TimerA_isRunning(DAC_TIMER_INST),
        DL_TimerA_getTimerCount(DAC_TIMER_INST),
        DL_DAC12_isEnabled(DAC12_INST),
        DL_DAC12_isFIFOEnabled(DAC12_INST),
        DL_DAC12_isFIFOFull(DAC12_INST),
        DAC12_INST->CTL2,
        DAC12_INST->CTL3,
        DAC12_INST->DATA0,
        DL_DAC12_getSubscriberChanID(DAC12_INST, DL_DAC12_SUBSCRIBER_INDEX_0),
        DL_TimerA_getPublisherChanID(DAC_TIMER_INST, DL_TIMERA_PUBLISHER_INDEX_0));
}
