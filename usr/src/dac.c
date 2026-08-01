#include "dac.h"
#include "board_config.h"
#include "system_time.h"
#include "usr_uart.h"
#include <math.h>

#define DAC_SINE_AMPLITUDE_CODE (1800U)
#define DAC_SINE_CENTER_CODE    (2048U)
#define DAC_SINE_AMPLITUDE_MAX_MV (BOARD_DAC_REFERENCE_MV / 2U)
#define DAC_TWO_PI              (6.28318530718f)
#define DAC_SINE_MAX_HZ         (DAC_SINE_FS_MAX_HZ / DAC_SINE_TABLE_MIN)
#define DAC_SINE_SAMPLE_TIMER_HZ     (1000000U)

static uint16_t gDACCode = BOARD_DAC_DEFAULT_CODE;
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
    DAC_SetCode12(BOARD_DAC_DEFAULT_CODE);
}

void DAC_SetCode12(uint16_t code)
{
    DL_DAC12_disableSampleTimeGenerator(BOARD_DAC_INST);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DAC12_disableDMATrigger(BOARD_DAC_INST);
    DL_DAC12_disableFIFO(BOARD_DAC_INST);
    gDACSineRunning = false;
    gDACCode = DAC_clampCode(code);
    DL_DAC12_output12(BOARD_DAC_INST, gDACCode);
}

void DAC_SetVoltageMv(uint16_t millivolt)
{
    uint32_t code;

    if (millivolt > BOARD_DAC_REFERENCE_MV) {
        millivolt = BOARD_DAC_REFERENCE_MV;
    }

    code = ((uint32_t) millivolt * DAC_CODE_MAX +
        (BOARD_DAC_REFERENCE_MV / 2U)) / BOARD_DAC_REFERENCE_MV;
    DAC_SetCode12((uint16_t) code);
}

void DAC_SetSineAmplitudeMv(uint16_t millivolt)
{
    uint32_t code;
    uint32_t frequencyHz;

    if (millivolt > DAC_SINE_AMPLITUDE_MAX_MV) {
        millivolt = DAC_SINE_AMPLITUDE_MAX_MV;
    }

    code = ((uint32_t) millivolt * DAC_CODE_MAX +
        (BOARD_DAC_REFERENCE_MV / 2U)) / BOARD_DAC_REFERENCE_MV;
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
    return (uint16_t) (((uint32_t) gDACCode * BOARD_DAC_REFERENCE_MV +
                       (DAC_CODE_MAX / 2U)) /
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

    DL_DAC12_disableSampleTimeGenerator(BOARD_DAC_INST);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DMA_setTrigger(DMA, DMA_DAC_CHAN_ID, DMA_DAC0_EVT_BD_1_TRIG,
        DL_DMA_TRIGGER_TYPE_EXTERNAL);
    DL_DMA_setSrcIncrement(DMA, DMA_DAC_CHAN_ID, DL_DMA_ADDR_INCREMENT);
    DL_DMA_setDestIncrement(DMA, DMA_DAC_CHAN_ID, DL_DMA_ADDR_UNCHANGED);
    DL_DAC12_clearInterruptStatus(BOARD_DAC_INST, 0xFFFFFFFFU);

    DL_DAC12_enableFIFO(BOARD_DAC_INST);
    DL_DAC12_setFIFOTriggerSource(BOARD_DAC_INST,
        DL_DAC12_FIFO_TRIGGER_SAMPLETIMER);
    DL_DAC12_setFIFOThreshold(BOARD_DAC_INST,
        DL_DAC12_FIFO_THRESHOLD_TWO_QTRS_EMPTY);
    DL_DAC12_enable(BOARD_DAC_INST);

    /*
     * The DMA request is generated when FIFO empty slots reach the selected
     * threshold.  A newly enabled, empty FIFO does not necessarily create a
     * request, so preload it before enabling DMA requests.  The timer then
     * consumes two samples and reliably reaches the half-empty threshold.
     */
    DL_DAC12_disableDMATrigger(BOARD_DAC_INST);
    for (i = 0U; i < 4U; i++) {
        DL_DAC12_output12(BOARD_DAC_INST, gDACSineBuffer[i]);
    }

    DL_DMA_setSrcAddr(DMA, DMA_DAC_CHAN_ID, (uint32_t) &gDACSineBuffer[0]);
    DL_DMA_setDestAddr(DMA, DMA_DAC_CHAN_ID,
        (uint32_t) &BOARD_DAC_INST->DATA0);
    DL_DMA_setTransferSize(DMA, DMA_DAC_CHAN_ID, gDACSineTableLen);
    DL_DMA_enableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DAC12_enableDMATrigger(BOARD_DAC_INST);
    DL_DAC12_enableSampleTimeGenerator(BOARD_DAC_INST);
    gDACSineRunning = true;
    return true;
}

void DAC_StopSine(void)
{
    if (gDACSineRunning == false) {
        return;
    }

    DL_DAC12_disableSampleTimeGenerator(BOARD_DAC_INST);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_DAC12_disableDMATrigger(BOARD_DAC_INST);
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
    USR_UART_printf("DBG:%s,dma_en=%u,dma_sz=%u,dac_en=%u,fifo_en=%u,fifo_full=%u,ctl2=0x%08X,ctl3=0x%08X,data0=0x%08X\r\n",
        tag,
        DL_DMA_isChannelEnabled(DMA, DMA_DAC_CHAN_ID),
        DL_DMA_getTransferSize(DMA, DMA_DAC_CHAN_ID),
        DL_DAC12_isEnabled(BOARD_DAC_INST),
        DL_DAC12_isFIFOEnabled(BOARD_DAC_INST),
        DL_DAC12_isFIFOFull(BOARD_DAC_INST),
        BOARD_DAC_INST->CTL2,
        BOARD_DAC_INST->CTL3,
        BOARD_DAC_INST->DATA0);
}

uint32_t DAC_SweepSine(uint32_t startHz, uint32_t stopHz, float stepRatio,
    uint32_t dwellMs, DacSweepPointFn pointFn)
{
    uint32_t freqHz;
    uint32_t points = 0U;

    if ((startHz == 0U) || (stopHz == 0U) ||
        (stepRatio <= 1.0f) || (stepRatio > 2.0f)) {
        return 0U;
    }
    if (startHz > stopHz) {
        return 0U;
    }

    freqHz = startHz;
    while (freqHz <= stopHz) {
        if (!DAC_StartSineHz(freqHz)) {
            break;
        }
        if (pointFn != NULL) {
            pointFn(freqHz, points);
        }
        if (dwellMs > 0U) {
            SystemTime_DelayMs(dwellMs);
        }
        points++;

        {
            uint32_t next = (uint32_t) ((float) freqHz * stepRatio);
            if (next <= freqHz) {
                break;
            }
            freqHz = next;
        }
    }

    DAC_StopSine();
    return points;
}
