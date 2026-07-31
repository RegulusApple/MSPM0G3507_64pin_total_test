#include "ti_msp_dl_config.h"
#include "low_power.h"
#include "OLED.h"

#define APP_OLED_MIN_SECONDS 5U
#define APP_OLED_HOLD_TICKS  (APP_OLED_MIN_SECONDS + 1U)
#define APP_TIMER_SEND_DIVIDER 1U

static void App_ShowLatestMeasurement(void)
{
    const LowPowerMeasurement *measurement = LowPower_GetLatestMeasurement();
    uint32_t currentMa;

    OLED_DisplayOn();
    OLED_Clear();

    if ((measurement == 0) || (measurement->valid == false)) {
        OLED_ShowString(1, 1, "NO DATA");
    } else {
        currentMa = (uint32_t) (measurement->currentMa + 0.5f);
        OLED_ShowString(1, 1, "CUR:");
        OLED_ShowNum(1, 6, currentMa, 5);
        OLED_ShowString(1, 12, "mA");
        OLED_ShowString(2, 1, "SEQ:");
        OLED_ShowNum(2, 6, measurement->sequence, 5);
    }
}

static void App_ServiceOledTimerTick(bool *oledActive, uint32_t *oledTicksLeft)
{
    if (*oledActive == false) {
        return;
    }

    if (*oledTicksLeft > 0U) {
        (*oledTicksLeft)--;
    }

    if (*oledTicksLeft == 0U) {
        OLED_DisplayOff();
        *oledActive = false;
    }
}

static void App_PrepareForStandby(bool keepOledOn)
{
    __disable_irq();

    DL_UART_Main_disableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_RX);
    DL_UART_Main_clearInterruptStatus(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(UART_DEBUG_INST_INT_IRQN);
    DL_UART_Main_disable(UART_DEBUG_INST);
    DL_UART_Main_disablePower(UART_DEBUG_INST);

    DL_TimerG_stopCounter(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_disableInterrupt(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_clearInterruptStatus(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_disablePower(ADC_SAMPLE_TIMER_INST);

    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
    DL_DMA_disableChannel(DMA, DMA_CH1_CHAN_ID);
    DL_DMA_disableChannel(DMA, DMA_DAC_CHAN_ID);
    DL_ADC12_stopConversion(ADC12_0_INST);
    DL_ADC12_disableConversions(ADC12_0_INST);
    DL_ADC12_disableDMA(ADC12_0_INST);
    DL_ADC12_disableDMATrigger(ADC12_0_INST, DL_ADC12_DMA_MEM0_RESULT_LOADED);
    DL_ADC12_disableInterrupt(ADC12_0_INST, DL_ADC12_INTERRUPT_DMA_DONE);
    DL_ADC12_clearInterruptStatus(ADC12_0_INST, 0xFFFFFFFFU);
    NVIC_DisableIRQ(ADC12_0_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(ADC12_0_INST_INT_IRQN);
    DL_ADC12_reset(ADC12_0_INST);
    DL_ADC12_disablePower(ADC12_0_INST);
    DL_VREF_disableInternalRef(VREF);
    DL_VREF_reset(VREF);
    DL_VREF_disablePower(VREF);

    if (keepOledOn == false) {
        OLED_DisplayOff();
    }

    DL_SYSCTL_disableHFXT();
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_disableMFPCLK();
}

int main(void)
{
    bool oledActive = false;
    uint32_t oledTicksLeft = 0U;
    uint32_t timerSendDivider = 0U;

    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_ADC_SAMPLE_TIMER_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_VREF_init();
    SYSCFG_DL_ADC12_0_init();
    SYSCFG_DL_DMA_init();
    OLED_Init();
    OLED_ShowString(1, 1, "LP Ready");
    OLED_DisplayOff();
    LowPower_Init();

    while (1) {
        App_PrepareForStandby(oledActive);
        LowPower_EnterStandby1();

        if (LowPower_GetWakeReason() == WAKE_REASON_TIMER) {
            LowPower_ClearWakeReason();
            timerSendDivider++;
            if (timerSendDivider >= APP_TIMER_SEND_DIVIDER) {
                timerSendDivider = 0U;
                LowPower_RunTimerWakeFlow();
            }
            App_ServiceOledTimerTick(&oledActive, &oledTicksLeft);
        } else if (LowPower_GetWakeReason() == WAKE_REASON_EXTERNAL) {
            LowPower_ClearWakeReason();
            App_ShowLatestMeasurement();
            oledActive = true;
            oledTicksLeft = APP_OLED_HOLD_TICKS;
        } else {
            LowPower_ClearWakeReason();
        }
    }
}
