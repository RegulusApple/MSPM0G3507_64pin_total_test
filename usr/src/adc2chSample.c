#include "adc2chSample.h"
#include "signalProcess.h"
#include "usr_uart.h"
#include <math.h>

#define ADC12_FRAME_TIMEOUT_MARGIN_MS  (100U)
#define ADC_TIMER_MAX_PRESCALE_PLUS1   (256U)
#define ADC_TIMER_MAX_PERIOD_PLUS1     (65536U)

#define ADC12_DBG_STAGE_IDLE           (0U)
#define ADC12_DBG_STAGE_DMA_ARMED      (1U)
#define ADC12_DBG_STAGE_TIMER_STARTED  (2U)
#define ADC12_DBG_STAGE_WAIT_DMA       (3U)
#define ADC12_DBG_STAGE_FRAME_DONE     (4U)
#define ADC12_DBG_STAGE_TIMEOUT        (5U)

#define ADC12_MASK_ADC0                (0x01U)
#define ADC12_MASK_ADC1                (0x02U)
#define ADC12_MASK_BOTH                (ADC12_MASK_ADC0 | ADC12_MASK_ADC1)

#define ADC12_FFT_ADC_VREF             (2.5f)
#define ADC12_ADC_CODE_MAX             (4095.0f)
#define ADC12_FFT_HARMONIC_COUNT       (5U)
#define ADC12_FFT_SEARCH_MIN_BIN       (1U)
#define ADC12_FFT_SEARCH_MAX_BIN       (FFT_LEN / 2U)

static uint16_t gADC_Buffer0[ADC_BUFFER_LEN];
static uint16_t gADC_Buffer1[ADC_BUFFER_LEN];
static float gADC12FFTData[FFT_LEN * 2U];
static uint16_t gADC12FFTBins[ADC12_FFT_HARMONIC_COUNT];
static float gADC12FFTMag[ADC12_FFT_HARMONIC_COUNT];
static float gADC12FFTAngle[ADC12_FFT_HARMONIC_COUNT];

uint16_t *pADC_Buffer0 = gADC_Buffer0;
uint16_t *pADC_Buffer1 = gADC_Buffer1;
volatile bool ADC_Finished = false;
uint16_t per = 15625U;

static volatile bool gADC0DMADone;
static volatile bool gADC1DMADone;
static uint32_t gADCSampleRateHz = ADC_SAMPLE_RATE_DEFAULT_HZ;
static uint16_t gADCSampleTimerPeriod = ADC_SAMPLE_TIMER_DEFAULT_LOAD_VALUE;
static uint8_t gADCSampleTimerPrescaler = ADC_SAMPLE_TIMER_DEFAULT_PRESCALE;
static ADC12_DebugInfo gADC12DebugInfo;

static void ADC12_printNode(const char *node)
{
    (void) node;
}

static ADC12_Regs *ADC12_getRegs(uint8_t channel)
{
    return (channel == ADC12_CHANNEL_1) ? ADC12_1_INST : ADC12_0_INST;
}

static uint16_t *ADC12_getBuffer(uint8_t channel)
{
    return (channel == ADC12_CHANNEL_1) ? pADC_Buffer1 : pADC_Buffer0;
}

static const char *ADC12_getChannelName(uint8_t channel)
{
    return (channel == ADC12_CHANNEL_1) ? "PA21_ADC1" : "PA22_ADC0";
}

static void ADC12_setDebugInfo(uint16_t sampleIndex, uint8_t adcId, uint8_t stage,
    ADC12_Regs *adc)
{
    gADC12DebugInfo.sampleIndex        = sampleIndex;
    gADC12DebugInfo.adcId              = adcId;
    gADC12DebugInfo.stage              = stage;
    gADC12DebugInfo.dma0Done           = (uint8_t) gADC0DMADone;
    gADC12DebugInfo.dma1Done           = (uint8_t) gADC1DMADone;
    gADC12DebugInfo.timerRunning       = (uint8_t) DL_TimerG_isRunning(ADC_SAMPLE_TIMER_INST);
    gADC12DebugInfo.timerCount         = DL_TimerG_getTimerCount(ADC_SAMPLE_TIMER_INST);
    gADC12DebugInfo.rawInterruptStatus = DL_ADC12_getRawInterruptStatus(adc, 0xFFFFFFFFU);
    gADC12DebugInfo.control1           = adc->ULLMEM.CTL1;
}

void ADC12_SetSampleRateHz(uint32_t sampleRateHz)
{
    uint32_t divider;
    uint32_t prescalePlus1;
    uint32_t periodPlus1;
    DL_TimerG_ClockConfig clockConfig = {
        .clockSel    = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale    = 0U,
    };

    if (sampleRateHz == 0U) {
        sampleRateHz = ADC_SAMPLE_RATE_DEFAULT_HZ;
    }
    if (sampleRateHz > ADC_SAMPLE_RATE_MAX_HZ) {
        sampleRateHz = ADC_SAMPLE_RATE_MAX_HZ;
    }

    divider = (CPUCLK_FREQ + (sampleRateHz / 2U)) / sampleRateHz;
    if (divider < 1U) {
        divider = 1U;
    }
    if (divider > (ADC_TIMER_MAX_PRESCALE_PLUS1 * ADC_TIMER_MAX_PERIOD_PLUS1)) {
        divider = ADC_TIMER_MAX_PRESCALE_PLUS1 * ADC_TIMER_MAX_PERIOD_PLUS1;
    }

    prescalePlus1 = (divider + (ADC_TIMER_MAX_PERIOD_PLUS1 - 1U)) / ADC_TIMER_MAX_PERIOD_PLUS1;
    if (prescalePlus1 < 1U) {
        prescalePlus1 = 1U;
    }
    if (prescalePlus1 > ADC_TIMER_MAX_PRESCALE_PLUS1) {
        prescalePlus1 = ADC_TIMER_MAX_PRESCALE_PLUS1;
    }

    periodPlus1 = divider / prescalePlus1;
    if (periodPlus1 < 1U) {
        periodPlus1 = 1U;
    }
    if (periodPlus1 > ADC_TIMER_MAX_PERIOD_PLUS1) {
        periodPlus1 = ADC_TIMER_MAX_PERIOD_PLUS1;
    }

    gADCSampleTimerPrescaler = (uint8_t) (prescalePlus1 - 1U);
    gADCSampleTimerPeriod = (uint16_t) (periodPlus1 - 1U);
    gADCSampleRateHz = CPUCLK_FREQ / (prescalePlus1 * periodPlus1);
    per = gADCSampleTimerPeriod;

    DL_TimerG_stopCounter(ADC_SAMPLE_TIMER_INST);
    clockConfig.prescale = gADCSampleTimerPrescaler;
    DL_TimerG_setClockConfig(ADC_SAMPLE_TIMER_INST, &clockConfig);
    DL_TimerG_setLoadValue(ADC_SAMPLE_TIMER_INST, gADCSampleTimerPeriod);
    DL_TimerG_setTimerCount(ADC_SAMPLE_TIMER_INST, gADCSampleTimerPeriod);

    /* Mirror to stagger timer for ADC1 — same period, counter offset by delay */
    DL_TimerG_stopCounter(ADC1_STAGGER_TIMER_INST);
    clockConfig.prescale = gADCSampleTimerPrescaler;
    DL_TimerG_setClockConfig(ADC1_STAGGER_TIMER_INST, &clockConfig);
    DL_TimerG_setLoadValue(ADC1_STAGGER_TIMER_INST, gADCSampleTimerPeriod);
    /* Counter set higher so ADC1 fires ADC_STAGGER_DELAY_TICKS after ADC0 */
    DL_TimerG_setTimerCount(ADC1_STAGGER_TIMER_INST,
        gADCSampleTimerPeriod + ADC_STAGGER_DELAY_TICKS);
}

uint32_t ADC12_GetSampleRateHz(void)
{
    return gADCSampleRateHz;
}

uint16_t ADC12_GetSampleTimerPeriod(void)
{
    return gADCSampleTimerPeriod;
}

uint8_t ADC12_GetSampleTimerPrescaler(void)
{
    return gADCSampleTimerPrescaler;
}

ADC12_DebugInfo ADC12_GetDebugInfo(void)
{
    return gADC12DebugInfo;
}

void ADC12_PrintDebug(const char *tag)
{
    ADC12_DebugInfo info = ADC12_GetDebugInfo();

    USR_UART_printf("DBG:%s,stage=%u,index=%u,adc=%u,dma0=%u,dma1=%u,trun=%u,tcnt=%u,ris=0x%08X,ctl1=0x%08X\r\n",
        tag,
        info.stage,
        info.sampleIndex,
        info.adcId,
        info.dma0Done,
        info.dma1Done,
        info.timerRunning,
        info.timerCount,
        info.rawInterruptStatus,
        info.control1);
}

void ADC12_Init(void)
{
    ADC12_SetSampleRateHz(ADC_SAMPLE_RATE_DEFAULT_HZ);
    SP_myFFTInit();
    ADC12_setDebugInfo(0U, 0U, ADC12_DBG_STAGE_IDLE, ADC12_0_INST);
    USR_UART_printf("MSPM0G3507 ADC ready, BUF_TOTAL=%u, DISCARD=%u, VALID=%u, FS_HZ=%u, TIMER_PERIOD=%u, TIMER_PRESCALE=%u, MODE=TIMER_EVENT_SINGLE_DMA\r\n",
        ADC_BUFFER_LEN,
        ADC_DISCARD_LEN,
        ADC_VALID_LEN,
        ADC12_GetSampleRateHz(),
        ADC12_GetSampleTimerPeriod(),
        ADC12_GetSampleTimerPrescaler());
}

static uint32_t ADC12_getFrameTimeoutLoops(void)
{
    uint32_t frameMs;

    if (gADCSampleRateHz == 0U) {
        return CPUCLK_FREQ;
    }

    frameMs = (ADC_BUFFER_LEN * 1000U) / gADCSampleRateHz;
    return (CPUCLK_FREQ / 1000U) * (frameMs + ADC12_FRAME_TIMEOUT_MARGIN_MS);
}

static void ADC12_prepareFrame(uint8_t channelMask)
{
    volatile uint16_t dummy;
    uint8_t i;

    gADC0DMADone = false;
    gADC1DMADone = false;
    ADC_Finished = false;

    /* --- 1. Stop timers --- */
    DL_TimerG_stopCounter(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_stopCounter(ADC1_STAGGER_TIMER_INST);

    /* --- 2. Disable ADC triggers --- */
    DL_ADC12_stopConversion(ADC12_0_INST);
    DL_ADC12_stopConversion(ADC12_1_INST);
    DL_ADC12_disableConversions(ADC12_0_INST);
    DL_ADC12_disableConversions(ADC12_1_INST);

    /* --- 3. Disable DMA --- */
    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
    DL_DMA_disableChannel(DMA, DMA_CH1_CHAN_ID);

    /* --- 4. Clear all interrupt flags --- */
    DL_ADC12_clearInterruptStatus(ADC12_0_INST, 0xFFFFFFFFU);
    DL_ADC12_clearInterruptStatus(ADC12_1_INST, 0xFFFFFFFFU);
    DL_DMA_clearInterruptStatus(DMA,
        DL_DMA_INTERRUPT_CHANNEL0 | DL_DMA_INTERRUPT_CHANNEL1 |
        DL_DMA_INTERRUPT_ADDR_ERROR | DL_DMA_INTERRUPT_DATA_ERROR);
    NVIC_DisableIRQ(ADC12_0_INST_INT_IRQN);
    NVIC_DisableIRQ(ADC12_1_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(ADC12_0_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(ADC12_1_INST_INT_IRQN);

    /* --- 5. Drain ADC DMA FIFO — read stale results --- */
    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        for (i = 0U; i < 8U; i++) {
            dummy = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_0);
            (void)dummy;
        }
    }
    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        for (i = 0U; i < 8U; i++) {
            dummy = DL_ADC12_getMemResult(ADC12_1_INST, ADC12_1_ADCMEM_0);
            (void)dummy;
        }
    }

    /* --- 6. Reset timer counters --- */
    DL_TimerG_setTimerCount(ADC_SAMPLE_TIMER_INST, gADCSampleTimerPeriod);
    DL_TimerG_setTimerCount(ADC1_STAGGER_TIMER_INST,
        gADCSampleTimerPeriod + ADC_STAGGER_DELAY_TICKS);

    /* --- 7. Enable ADC + stabilization delay --- */
    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        DL_ADC12_enableConversions(ADC12_0_INST);
    }
    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        DL_ADC12_enableConversions(ADC12_1_INST);
    }
    delay_cycles(8U);  /* approx 1 us stabilization at 8 MHz MCLK */

    /* --- 8. Arm DMA (AFTER ADC ready, no stale triggers) --- */
    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID,
            (uint32_t) DL_ADC12_getMemResultAddress(ADC12_0_INST, ADC12_0_ADCMEM_0));
        DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &gADC_Buffer0[0]);
        DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, ADC_BUFFER_LEN);
        DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
        DL_ADC12_enableDMA(ADC12_0_INST);
        DL_ADC12_enableDMATrigger(ADC12_0_INST, DL_ADC12_DMA_MEM0_RESULT_LOADED);
    }

    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        DL_DMA_setSrcAddr(DMA, DMA_CH1_CHAN_ID,
            (uint32_t) DL_ADC12_getMemResultAddress(ADC12_1_INST, ADC12_1_ADCMEM_0));
        DL_DMA_setDestAddr(DMA, DMA_CH1_CHAN_ID, (uint32_t) &gADC_Buffer1[0]);
        DL_DMA_setTransferSize(DMA, DMA_CH1_CHAN_ID, ADC_BUFFER_LEN);
        DL_DMA_enableChannel(DMA, DMA_CH1_CHAN_ID);
        DL_ADC12_enableDMA(ADC12_1_INST);
        DL_ADC12_enableDMATrigger(ADC12_1_INST, DL_ADC12_DMA_MEM0_RESULT_LOADED);
    }

    ADC12_setDebugInfo(0U, 0U, ADC12_DBG_STAGE_DMA_ARMED, ADC12_getRegs(ADC12_CHANNEL_0));
}

static void ADC12_startFrame(uint8_t channelMask)
{
    DL_GPIO_clearPins(ADC_SAMPLE_PROBE_PORT, ADC_SAMPLE_PROBE_PIN);
    DL_TimerG_clearInterruptStatus(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);

    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        DL_ADC12_clearInterruptStatus(ADC12_0_INST, DL_ADC12_INTERRUPT_DMA_DONE);
        DL_ADC12_enableInterrupt(ADC12_0_INST, DL_ADC12_INTERRUPT_DMA_DONE);
        NVIC_ClearPendingIRQ(ADC12_0_INST_INT_IRQN);
        NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
    }

    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        DL_ADC12_clearInterruptStatus(ADC12_1_INST, DL_ADC12_INTERRUPT_DMA_DONE);
        DL_ADC12_enableInterrupt(ADC12_1_INST, DL_ADC12_INTERRUPT_DMA_DONE);
        NVIC_ClearPendingIRQ(ADC12_1_INST_INT_IRQN);
        NVIC_EnableIRQ(ADC12_1_INST_INT_IRQN);
    }

    /* --- 9. Start timer triggers (last, after everything is armed) --- */
    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        DL_TimerG_startCounter(ADC_SAMPLE_TIMER_INST);
    }
    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        DL_TimerG_startCounter(ADC1_STAGGER_TIMER_INST);
    }
    ADC12_setDebugInfo(0U, 0U, ADC12_DBG_STAGE_TIMER_STARTED, ADC12_getRegs(ADC12_CHANNEL_0));
}

static bool ADC12_isFrameDone(uint8_t channelMask)
{
    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        if (DL_DMA_getTransferSize(DMA, DMA_CH0_CHAN_ID) == 0U) {
            gADC0DMADone = true;
        }
        if (gADC0DMADone == false) {
            return false;
        }
    }

    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        if (DL_DMA_getTransferSize(DMA, DMA_CH1_CHAN_ID) == 0U) {
            gADC1DMADone = true;
        }
        if (gADC1DMADone == false) {
            return false;
        }
    }

    return true;
}

static void ADC12_stopFrame(uint8_t channelMask)
{
    DL_TimerG_stopCounter(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_stopCounter(ADC1_STAGGER_TIMER_INST);
    DL_TimerG_clearInterruptStatus(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_GPIO_clearPins(ADC_SAMPLE_PROBE_PORT, ADC_SAMPLE_PROBE_PIN);

    if ((channelMask & ADC12_MASK_ADC0) != 0U) {
        DL_ADC12_stopConversion(ADC12_0_INST);
        DL_ADC12_disableConversions(ADC12_0_INST);
        DL_ADC12_clearInterruptStatus(ADC12_0_INST, 0xFFFFFFFFU);
        NVIC_DisableIRQ(ADC12_0_INST_INT_IRQN);
        NVIC_ClearPendingIRQ(ADC12_0_INST_INT_IRQN);
    }

    if ((channelMask & ADC12_MASK_ADC1) != 0U) {
        DL_ADC12_stopConversion(ADC12_1_INST);
        DL_ADC12_disableConversions(ADC12_1_INST);
        DL_ADC12_clearInterruptStatus(ADC12_1_INST, 0xFFFFFFFFU);
        NVIC_DisableIRQ(ADC12_1_INST_INT_IRQN);
        NVIC_ClearPendingIRQ(ADC12_1_INST_INT_IRQN);
    }
}

static bool ADC12_sample(uint8_t channelMask, const char *tag)
{
    uint32_t timeout = ADC12_getFrameTimeoutLoops();

    ADC12_printNode(tag);
    ADC12_prepareFrame(channelMask);
    ADC12_printNode("PREPARED");
    ADC12_printNode("TIMER_STARTED");
    ADC12_startFrame(channelMask);

    while (ADC12_isFrameDone(channelMask) == false) {
        if (timeout == 0U) {
            ADC12_stopFrame(channelMask);
            ADC12_setDebugInfo(0U,
                ((channelMask & ADC12_MASK_ADC0) != 0U) ? 0U : 1U,
                ADC12_DBG_STAGE_TIMEOUT,
                ((channelMask & ADC12_MASK_ADC0) != 0U) ? ADC12_0_INST : ADC12_1_INST);
            return false;
        }
        timeout--;
        __WFI();
    }

    ADC12_stopFrame(channelMask);
    ADC_Finished = true;
    ADC12_setDebugInfo(ADC_BUFFER_LEN, 0U, ADC12_DBG_STAGE_FRAME_DONE, ADC12_getRegs(ADC12_CHANNEL_0));
    ADC12_printNode("DONE");
    return true;
}

bool ADC12_SampleADC0(void)
{
    return ADC12_sample(ADC12_MASK_ADC0, "ADC0_ENTER");
}

bool ADC12_SampleADC1(void)
{
    return ADC12_sample(ADC12_MASK_ADC1, "ADC1_ENTER");
}

bool ADC12_SampleADC0ADC1(void)
{
    return ADC12_sample(ADC12_MASK_BOTH, "ADC0_ADC1_ENTER");
}

bool ADC12_CalcRms(uint8_t channel, ADC12_RmsResult *result)
{
    uint16_t i;
    uint16_t *buffer;
    double sum = 0.0;
    double mean;
    double squareSum = 0.0;
    double rmsCode;

    if ((result == NULL) ||
        ((channel != ADC12_CHANNEL_0) && (channel != ADC12_CHANNEL_1))) {
        return false;
    }

    buffer = ADC12_getBuffer(channel);

    for (i = ADC_DISCARD_LEN; i < ADC_BUFFER_LEN; i++) {
        sum += (double) buffer[i];
    }

    mean = sum / (double) ADC_VALID_LEN;

    for (i = ADC_DISCARD_LEN; i < ADC_BUFFER_LEN; i++) {
        double acCode = (double) buffer[i] - mean;
        squareSum += acCode * acCode;
    }

    rmsCode = sqrt(squareSum / (double) ADC_VALID_LEN);

    result->sampleCount = ADC_VALID_LEN;
    result->dcCode = (float) mean;
    result->rmsCode = (float) rmsCode;
    result->rmsMv = (float) ((rmsCode * (double) ADC12_GetAVCC_Voltage() * 1000.0) /
        ((double) ADC12_ADC_CODE_MAX * (double) ADC_FRONTEND_GAIN));

    return true;
}

void ADC12_UpFrame(uint8_t channelSize, uint8_t channel)
{
    uint16_t i;
    uint16_t *buffer;

    if (channelSize == ADC12_CHANNEL_SIZE_2) {
        USR_UART_printf("BEGIN:ADC_FRAME,SAMPLE_N=%u,FS_HZ=%u,TIMER_PERIOD=%u,TIMER_PRESCALE=%u,MODE=TIMER_EVENT_SINGLE_DMA,CHANNEL_SIZE=2,CH0=PA22_ADC0,CH1=PA21_ADC1\r\n",
            ADC_VALID_LEN,
            ADC12_GetSampleRateHz(),
            ADC12_GetSampleTimerPeriod(),
            ADC12_GetSampleTimerPrescaler());
        USR_UART_printf("ch0\r\n");
        for (i = ADC_DISCARD_LEN; i < ADC_BUFFER_LEN; i++) {
            USR_UART_printf("%u,", pADC_Buffer0[i]);
        }
				USR_UART_printf("\n ch1\n");
				for (i = ADC_DISCARD_LEN; i < ADC_BUFFER_LEN; i++){
						USR_UART_printf("%u,", pADC_Buffer1[i]);
				
				}
        USR_UART_printf("END:ADC_FRAME\r\n");
        return;
    }

    if ((channelSize != ADC12_CHANNEL_SIZE_1) ||
        ((channel != ADC12_CHANNEL_0) && (channel != ADC12_CHANNEL_1))) {
        USR_UART_printf("ERR:ADC_UPFRAME_PARAM,channel_size=%u,channel=%u\r\n",
            channelSize,
            channel);
        return;
    }

    buffer = ADC12_getBuffer(channel);
    USR_UART_printf("BEGIN:ADC_FRAME,SAMPLE_N=%u,FS_HZ=%u,TIMER_PERIOD=%u,TIMER_PRESCALE=%u,MODE=TIMER_EVENT_SINGLE_DMA,CHANNEL_SIZE=1,CH=%s\r\n",
        ADC_VALID_LEN,
        ADC12_GetSampleRateHz(),
        ADC12_GetSampleTimerPeriod(),
        ADC12_GetSampleTimerPrescaler(),
        ADC12_getChannelName(channel));
    USR_UART_printf("index,ch%u\r\n", channel);
    for (i = ADC_DISCARD_LEN; i < ADC_BUFFER_LEN; i++) {
        USR_UART_printf("%u,",  buffer[i]);
    }
    USR_UART_printf("END:ADC_FRAME\r\n");
}

static float ADC12_getFFTBinMagSq(uint16_t bin)
{
    float real = gADC12FFTData[bin << 1];
    float imag = gADC12FFTData[(bin << 1) + 1U];

    return (real * real) + (imag * imag);
}

static uint16_t ADC12_findFundamentalBin(void)
{
    uint16_t bin;
    uint16_t maxBin = ADC12_FFT_SEARCH_MAX_BIN;
    uint16_t bestBin = ADC12_FFT_SEARCH_MIN_BIN;
    float bestMagSq = 0.0f;

    if (maxBin > (FFT_LEN / 2U)) {
        maxBin = FFT_LEN / 2U;
    }
    if (bestBin == 0U) {
        bestBin = 1U;
    }

    for (bin = bestBin; bin <= maxBin; bin++) {
        float magSq = ADC12_getFFTBinMagSq(bin);

        if (magSq > bestMagSq) {
            bestMagSq = magSq;
            bestBin = bin;
        }
    }

    return bestBin;
}

static void ADC12_updateFFTBins(uint16_t fundamentalBin)
{
    uint16_t i;

    for (i = 0U; i < ADC12_FFT_HARMONIC_COUNT; i++) {
        uint32_t bin = (uint32_t) fundamentalBin * (uint32_t) (i + 1U);

        if (bin > (FFT_LEN / 2U)) {
            bin = FFT_LEN / 2U;
        }
        gADC12FFTBins[i] = (uint16_t) bin;
    }
}

void ADC12_FFTSHOW(uint8_t channel)
{
    uint16_t i;
    uint16_t fundamentalBin;
    uint32_t fundamentalHz;
    uint32_t binFreqHz;

    if ((channel != ADC12_CHANNEL_0) && (channel != ADC12_CHANNEL_1)) {
        USR_UART_printf("ERR:ADC_FFT_PARAM,channel=%u\r\n", channel);
        return;
    }

    SP_myFFT(&ADC12_getBuffer(channel)[ADC_DISCARD_LEN], gADC12FFTData);
    fundamentalBin = ADC12_findFundamentalBin();
    fundamentalHz = ((uint32_t) fundamentalBin * ADC12_GetSampleRateHz()) /
        FFT_LEN;
    ADC12_updateFFTBins(fundamentalBin);
    SP_myFFT_CalcMagAngle(gADC12FFTData, gADC12FFTBins, ADC12_FFT_HARMONIC_COUNT,
        ADC12_FFT_ADC_VREF, gADC12FFTMag, gADC12FFTAngle);

    USR_UART_printf("ADC%u_FFT_FUND:bin=%u,freq_hz=%lu\r\n",
        channel,
        fundamentalBin,
        (unsigned long) fundamentalHz);
    USR_UART_printf("ADC%u_FFT_HARM:harmonic,bin,bin_hz,mag_vrms,angle_deg\r\n",
        channel);
    for (i = 0U; i < ADC12_FFT_HARMONIC_COUNT; i++) {
        binFreqHz = ((uint32_t) gADC12FFTBins[i] * ADC12_GetSampleRateHz()) /
            FFT_LEN;
        USR_UART_printf("ADC%u_FFT_HARM:%u,%u,%lu,%.6f,%.3f\r\n",
            channel,
            (uint16_t) (i + 1U),
            gADC12FFTBins[i],
            (unsigned long) binFreqHz,
            gADC12FFTMag[i],
            gADC12FFTAngle[i]);
    }
}

float ADC12_GetAVCC_Voltage(void)
{
    return ADC12_0_ADCMEM_0_REF_VOLTAGE_V;
}

void ADC12_0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
        case DL_ADC12_IIDX_DMA_DONE:
            gADC0DMADone = true;
            ADC12_setDebugInfo(ADC_BUFFER_LEN, 0U, ADC12_DBG_STAGE_WAIT_DMA, ADC12_0_INST);
            break;
        default:
            break;
    }
}

void ADC12_1_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_1_INST)) {
        case DL_ADC12_IIDX_DMA_DONE:
            gADC1DMADone = true;
            ADC12_setDebugInfo(ADC_BUFFER_LEN, 1U, ADC12_DBG_STAGE_WAIT_DMA, ADC12_1_INST);
            break;
        default:
            break;
    }
}
