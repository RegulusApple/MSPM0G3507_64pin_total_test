#include "low_power.h"

#include <ti/driverlib/m0p/dl_interrupt.h>
#include "ti_msp_dl_config.h"
#include "adc2chSample.h"
#include "addr_code.h"
#include "serial_protocol.h"
#include "usr_uart.h"

#define LP_ANALOG_SETTLE_CYCLES         (CPUCLK_FREQ / 50U)   /* about 20 ms */
#define LP_WIRELESS_READY_CYCLES        (CPUCLK_FREQ / 100U)  /* about 10 ms */
#define LP_UART_TX_TIMEOUT_CYCLES       (CPUCLK_FREQ / 10U)   /* about 100 ms */
#define LP_WIRELESS_SLEEP_CMD           "ATON\r\nAT+SLEEP=1\r\n"
#define LP_WIRELESS_AT_SLEEP_ENABLE     (1U)

static volatile WakeReason gWakeReason = WAKE_REASON_NONE;
static volatile uint32_t gWakeTimerTicks = 0U;
static LowPowerMeasurement gLatestMeasurement;
static bool gWakeTimerRunning = false;
static bool gKey2Configured = false;

static void LowPower_ConfigWakeTimer1s(bool forceReload)
{
    static const DL_TimerG_ClockConfig wakeClockConfig = {
        .clockSel = DL_TIMER_CLOCK_LFCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale = 0U,
    };

    static const DL_TimerG_TimerConfig wakeTimerConfig = {
        .timerMode = DL_TIMER_TIMER_MODE_PERIODIC,
        .period = LP_WAKE_TIMER_PERIOD_TICKS,
        .startTimer = DL_TIMER_STOP,
    };

    if ((gWakeTimerRunning == true) && (forceReload == false)) {
        DL_TimerG_enableInterrupt(LP_WAKE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
        NVIC_EnableIRQ(LP_WAKE_TIMER_INST_INT_IRQN);
        return;
    }

    /*
     * SYSCTL clock stability is checked during system clock initialization.
     * Do not wait forever here: this function is also called after every
     * measurement before re-entering STANDBY1. If the LFOSC status flag is
     * temporarily not reported as GOOD during the run/standby handoff, an
     * infinite wait here would stop all later 1 s wakeups after the first send.
     */

    DL_TimerG_enablePower(LP_WAKE_TIMER_INST);
    DL_TimerG_stopCounter(LP_WAKE_TIMER_INST);
    DL_TimerG_disableInterrupt(LP_WAKE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_clearInterruptStatus(LP_WAKE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    NVIC_ClearPendingIRQ(LP_WAKE_TIMER_INST_INT_IRQN);

    DL_TimerG_setClockConfig(LP_WAKE_TIMER_INST,
        (DL_TimerG_ClockConfig *) &wakeClockConfig);
    DL_TimerG_initTimerMode(LP_WAKE_TIMER_INST,
        (DL_TimerG_TimerConfig *) &wakeTimerConfig);
    DL_TimerG_setLoadValue(LP_WAKE_TIMER_INST, LP_WAKE_TIMER_PERIOD_TICKS);
    DL_TimerG_setTimerCount(LP_WAKE_TIMER_INST, LP_WAKE_TIMER_PERIOD_TICKS);
    DL_TimerG_enableClock(LP_WAKE_TIMER_INST);
    DL_TimerG_enableInterrupt(LP_WAKE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);

    NVIC_EnableIRQ(LP_WAKE_TIMER_INST_INT_IRQN);
    DL_TimerG_startCounter(LP_WAKE_TIMER_INST);
    gWakeTimerRunning = true;
}

static void LowPower_ConfigKey2Wake(void)
{
    DL_GPIO_initDigitalInputFeatures(KEY_KEY2_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_ON_0);

    DL_GPIO_setLowerPinsPolarity(KEY_KEY2_PORT, DL_GPIO_PIN_3_EDGE_FALL);
    if (gKey2Configured == false) {
        DL_GPIO_clearInterruptStatus(KEY_KEY2_PORT, KEY_KEY2_PIN);
        gKey2Configured = true;
    }
    DL_GPIO_enableInterrupt(KEY_KEY2_PORT, KEY_KEY2_PIN);
    DL_Interrupt_clearGroup(DL_INTERRUPT_GROUP_1, DL_INTERRUPT_GROUP1_GPIOB);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
}

static bool LowPower_IsKey2Pressed(void)
{
    return ((DL_GPIO_readPins(KEY_KEY2_PORT, KEY_KEY2_PIN) & KEY_KEY2_PIN) == 0U);
}

static void LowPower_RestoreAdcTimer(void)
{
    DL_TimerG_stopCounter(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_disableInterrupt(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_clearInterruptStatus(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    NVIC_ClearPendingIRQ(ADC_SAMPLE_TIMER_INST_INT_IRQN);

    SYSCFG_DL_ADC_SAMPLE_TIMER_init();
    ADC12_SetSampleRateHz(ADC_SAMPLE_RATE_DEFAULT_HZ);
}

static void LowPower_EnableMeasurementPeripherals(void)
{
    DL_VREF_enablePower(VREF);
    DL_ADC12_enablePower(ADC12_0_INST);
    DL_TimerG_enablePower(ADC_SAMPLE_TIMER_INST);
    delay_cycles(POWER_STARTUP_DELAY);

    SYSCFG_DL_VREF_init();
    SYSCFG_DL_ADC12_0_init();
    SYSCFG_DL_DMA_init();
    LowPower_RestoreAdcTimer();
    delay_cycles(LP_ANALOG_SETTLE_CYCLES);
}

static void LowPower_DisableMeasurementPeripherals(void)
{
    DL_TimerG_stopCounter(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_stopCounter(ADC1_STAGGER_TIMER_INST);
    DL_TimerG_disableInterrupt(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_clearInterruptStatus(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_disablePower(ADC_SAMPLE_TIMER_INST);

    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
    DL_DMA_disableChannel(DMA, DMA_CH1_CHAN_ID);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);

    DL_ADC12_stopConversion(ADC12_0_INST);
    DL_ADC12_stopConversion(ADC12_1_INST);
    DL_ADC12_disableConversions(ADC12_0_INST);
    DL_ADC12_disableConversions(ADC12_1_INST);
    DL_ADC12_disableDMA(ADC12_0_INST);
    DL_ADC12_disableDMATrigger(ADC12_0_INST, DL_ADC12_DMA_MEM0_RESULT_LOADED);
    DL_ADC12_disableInterrupt(ADC12_0_INST, DL_ADC12_INTERRUPT_DMA_DONE);
    DL_ADC12_clearInterruptStatus(ADC12_0_INST, 0xFFFFFFFFU);
    NVIC_DisableIRQ(ADC12_0_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(ADC12_0_INST_INT_IRQN);
    DL_ADC12_reset(ADC12_0_INST);
    DL_ADC12_disablePower(ADC12_0_INST);
    DL_ADC12_disablePower(ADC12_1_INST);
    DL_VREF_disableInternalRef(VREF);
    DL_VREF_reset(VREF);
    DL_VREF_disablePower(VREF);
    DL_TimerG_disablePower(ADC1_STAGGER_TIMER_INST);
}

static void LowPower_EnableUart(void)
{
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    delay_cycles(POWER_STARTUP_DELAY);
    SYSCFG_DL_UART_DEBUG_init();
    USR_UART_init();
}

static void LowPower_WaitUartTxDone(void)
{
    uint32_t timeout = LP_UART_TX_TIMEOUT_CYCLES;

    while ((DL_UART_Main_isBusy(UART_DEBUG_INST) == true) && (timeout > 0U)) {
        timeout--;
    }
}

static void LowPower_DisableUnusedPeripheralsBeforeStandby(void)
{
    LowPower_WaitUartTxDone();

    DL_SPI_disablePower(AD9833_SPI_INST);
    DL_DAC12_disablePower(DAC0);
    DL_TimerA_stopCounter(DAC_TIMER_INST);
    DL_TimerA_disablePower(DAC_TIMER_INST);
    DL_TimerA_stopCounter(PWM_0_INST);
    DL_TimerA_disablePower(PWM_0_INST);
    DL_UART_Main_disablePower(UART_DEBUG_INST);
}

static void LowPower_WirelessWake(void)
{
    LowPower_EnableUart();
    delay_cycles(LP_WIRELESS_READY_CYCLES);
    USR_UART_sendByte(0x00U);
    LowPower_WaitUartTxDone();
    delay_cycles(LP_WIRELESS_READY_CYCLES*5);
    USR_UART_sendByte(0x00U);
    LowPower_WaitUartTxDone();
    delay_cycles(LP_WIRELESS_READY_CYCLES*5);
    USR_UART_sendByte(0x00U);
    LowPower_WaitUartTxDone();
    delay_cycles(LP_WIRELESS_READY_CYCLES*5);
    USR_UART_sendByte(0x00U);
    LowPower_WaitUartTxDone();
    delay_cycles(LP_WIRELESS_READY_CYCLES*5);
    USR_UART_sendByte(0x00U);
    LowPower_WaitUartTxDone();
    delay_cycles(LP_WIRELESS_READY_CYCLES*5);
    
    
    // delay_cycles(LP_WIRELESS_READY_CYCLES);

}

static void LowPower_WirelessSleep(void)
{
    delay_cycles(LP_WIRELESS_READY_CYCLES);
    LowPower_WaitUartTxDone();
    USR_UART_sendString(LP_WIRELESS_SLEEP_CMD);
    LowPower_WaitUartTxDone();
}

static void LowPower_SendMeasurement(const char *tag, const LowPowerMeasurement *m)
{
    if ((m == 0) || (m->valid == false)) {
        (void) tag;
        return;
    }

    (void) tag;
    (void) SerialProtocol_SendCurrentFrame(m->addrCode, m->currentMa);
}

static bool LowPower_DoMeasurement(void)
{
    ADC12_RmsResult rms;
    uint8_t addrCode;

    addrCode = AddrCode_Read();
    LowPower_EnableMeasurementPeripherals();

    if (ADC12_SampleADC0() == false) {
        LowPower_DisableMeasurementPeripherals();
        return false;
    }

    if (ADC12_CalcRms(ADC12_CHANNEL_0, &rms) == false) {
        LowPower_DisableMeasurementPeripherals();
        return false;
    }

    gLatestMeasurement.sequence++;
    gLatestMeasurement.addrCode = addrCode;
    gLatestMeasurement.sampleCount = rms.sampleCount;
    gLatestMeasurement.rmsMv = rms.rmsMv;
    gLatestMeasurement.currentMa = rms.rmsMv;
    gLatestMeasurement.valid = true;

    LowPower_DisableMeasurementPeripherals();
    return true;
}

static void LowPower_ServiceWakeTimerPending(void)
{
    uint32_t pending;

    pending = DL_TimerG_getRawInterruptStatus(LP_WAKE_TIMER_INST,
        DL_TIMERG_INTERRUPT_ZERO_EVENT);
    if ((pending & DL_TIMERG_INTERRUPT_ZERO_EVENT) != 0U) {
        DL_TimerG_clearInterruptStatus(LP_WAKE_TIMER_INST,
            DL_TIMERG_INTERRUPT_ZERO_EVENT);
        gWakeTimerTicks++;
        gWakeReason = WAKE_REASON_TIMER;
    }
}

static void LowPower_ServiceKey2Pending(void)
{
    uint32_t pending;

    pending = DL_GPIO_getEnabledInterruptStatus(KEY_KEY2_PORT, KEY_KEY2_PIN);
    if ((pending & KEY_KEY2_PIN) != 0U) {
        DL_GPIO_clearInterruptStatus(KEY_KEY2_PORT, KEY_KEY2_PIN);
        DL_Interrupt_clearGroup(DL_INTERRUPT_GROUP_1, DL_INTERRUPT_GROUP1_GPIOB);
        gWakeReason = WAKE_REASON_EXTERNAL;
    } else if (LowPower_IsKey2Pressed() == true) {
        gWakeReason = WAKE_REASON_EXTERNAL;
    }
}

void LowPower_Init(void)
{
    gWakeReason = WAKE_REASON_NONE;
    gWakeTimerTicks = 0U;
    gLatestMeasurement.sequence = 0U;
    gLatestMeasurement.valid = false;
    gWakeTimerRunning = false;
    gKey2Configured = false;

    LowPower_ConfigKey2Wake();
    LowPower_ConfigWakeTimer1s(true);
}

WakeReason LowPower_GetWakeReason(void)
{
    return gWakeReason;
}

uint32_t LowPower_GetWakeTimerTicks(void)
{
    return gWakeTimerTicks;
}

void LowPower_ClearWakeReason(void)
{
    gWakeReason = WAKE_REASON_NONE;
}

const LowPowerMeasurement *LowPower_GetLatestMeasurement(void)
{
    return &gLatestMeasurement;
}

void LowPower_RunTimerWakeFlow(void)
{
    LowPower_ClearWakeReason();
    (void) LowPower_DoMeasurement();
    LowPower_WirelessWake();
    LowPower_SendMeasurement("LP_TIMER_SEND", &gLatestMeasurement);
    LowPower_WirelessSleep();
}

void LowPower_RunTimerMeasurementFlow(void)
{
    LowPower_ClearWakeReason();
    (void) LowPower_DoMeasurement();
}

void LowPower_RunExternalWakeFlow(void)
{
    LowPower_ClearWakeReason();
    LowPower_WirelessWake();
    LowPower_SendMeasurement("LP_KEY2_SEND", &gLatestMeasurement);
    LowPower_WirelessSleep();
}

void LowPower_EnterStandby1(void)
{
    __disable_irq();
    LowPower_ConfigKey2Wake();
    LowPower_ConfigWakeTimer1s(false);
    LowPower_DisableUnusedPeripheralsBeforeStandby();
    LowPower_ServiceWakeTimerPending();
    LowPower_ServiceKey2Pending();
    if (gWakeReason != WAKE_REASON_EXTERNAL) {
        NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    }
    if (gWakeReason != WAKE_REASON_TIMER) {
        NVIC_ClearPendingIRQ(LP_WAKE_TIMER_INST_INT_IRQN);
    }
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setPowerPolicySTANDBY1();
    __enable_irq();
    if (gWakeReason == WAKE_REASON_NONE) {
        __WFI();
    }
    DL_SYSCTL_setPowerPolicyRUN0SLEEP0();
}

void LP_WAKE_TIMER_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(LP_WAKE_TIMER_INST)) {
        case DL_TIMERG_IIDX_ZERO:
            DL_TimerG_clearInterruptStatus(LP_WAKE_TIMER_INST,
                DL_TIMERG_INTERRUPT_ZERO_EVENT);
            gWakeTimerTicks++;
            gWakeReason = WAKE_REASON_TIMER;
            break;
        default:
            break;
    }
}

void GROUP1_IRQHandler(void)
{
    uint32_t group = DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);

    if (group == DL_INTERRUPT_GROUP1_IIDX_GPIOB) {
        uint32_t pending = DL_GPIO_getEnabledInterruptStatus(KEY_KEY2_PORT, KEY_KEY2_PIN);
        DL_GPIO_clearInterruptStatus(KEY_KEY2_PORT, KEY_KEY2_PIN);
        DL_Interrupt_clearGroup(DL_INTERRUPT_GROUP_1, DL_INTERRUPT_GROUP1_GPIOB);
        if ((pending & KEY_KEY2_PIN) != 0U) {
            gWakeReason = WAKE_REASON_EXTERNAL;
        }
    } else {
        DL_Interrupt_clearGroup(DL_INTERRUPT_GROUP_1,
            DL_INTERRUPT_GROUP1_GPIOA | DL_INTERRUPT_GROUP1_GPIOB);
    }
}
