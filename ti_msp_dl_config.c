/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gPWM_0Backup;
DL_TimerA_backupConfig gDAC_TIMERBackup;
DL_SPI_backupConfig gAD9833_SPIBackup;

#define SYSCFG_CLOCK_WAIT_FOREVER                                 (0UL)

bool Sysclk_WaitForSYSOSCBase(uint32_t timeoutLoops)
{
    if (timeoutLoops == SYSCFG_CLOCK_WAIT_FOREVER) {
        while (DL_SYSCTL_getCurrentSYSOSCFreq() != DL_SYSCTL_SYSOSC_FREQ_BASE) {
        }
        return true;
    }

    while (timeoutLoops > 0U) {
        if (DL_SYSCTL_getCurrentSYSOSCFreq() == DL_SYSCTL_SYSOSC_FREQ_BASE) {
            return true;
        }
        timeoutLoops--;
    }

    return false;
}

bool Sysclk_WaitForLFOSCGood(uint32_t timeoutLoops)
{
    if (timeoutLoops == SYSCFG_CLOCK_WAIT_FOREVER) {
        while ((DL_SYSCTL_getClockStatus() & DL_SYSCTL_CLK_STATUS_LFOSC_GOOD) == 0U) {
        }
        return true;
    }

    while (timeoutLoops > 0U) {
        if ((DL_SYSCTL_getClockStatus() & DL_SYSCTL_CLK_STATUS_LFOSC_GOOD) != 0U) {
            return true;
        }
        timeoutLoops--;
    }

    return false;
}

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    /* PA0/PA1 PWM output is intentionally disabled. PA0 is used as ADC sample IRQ probe GPIO. */
    SYSCFG_DL_ADC_SAMPLE_TIMER_init();
    SYSCFG_DL_ADC1_STAGGER_TIMER_init();
    SYSCFG_DL_DAC_TIMER_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_AD9833_SPI_init();
    SYSCFG_DL_VREF_init();
    SYSCFG_DL_ADC12_0_init();
    SYSCFG_DL_ADC12_1_init();
    SYSCFG_DL_DMA_init();
    SYSCFG_DL_DAC12_init();
    /* Ensure backup structures have no valid state */
	gPWM_0Backup.backupRdy 	= false;
	gDAC_TIMERBackup.backupRdy 	= false;

	gAD9833_SPIBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(PWM_0_INST, &gPWM_0Backup);
	retStatus &= DL_TimerA_saveConfiguration(DAC_TIMER_INST, &gDAC_TIMERBackup);
	retStatus &= DL_SPI_saveConfiguration(AD9833_SPI_INST, &gAD9833_SPIBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(PWM_0_INST, &gPWM_0Backup, false);
	retStatus &= DL_TimerA_restoreConfiguration(DAC_TIMER_INST, &gDAC_TIMERBackup, false);
	retStatus &= DL_SPI_restoreConfiguration(AD9833_SPI_INST, &gAD9833_SPIBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(PWM_0_INST);
    DL_TimerG_reset(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_reset(ADC1_STAGGER_TIMER_INST);
    DL_TimerA_reset(DAC_TIMER_INST);
    DL_UART_Main_reset(UART_DEBUG_INST);
    DL_SPI_reset(AD9833_SPI_INST);
    DL_VREF_reset(VREF);
    DL_ADC12_reset(ADC12_0_INST);
    DL_ADC12_reset(ADC12_1_INST);

    DL_DAC12_reset(DAC0);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(PWM_0_INST);
    DL_TimerG_enablePower(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_enablePower(ADC1_STAGGER_TIMER_INST);
    DL_TimerA_enablePower(DAC_TIMER_INST);
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    DL_SPI_enablePower(AD9833_SPI_INST);
    DL_VREF_enablePower(VREF);
    DL_ADC12_enablePower(ADC12_0_INST);
    DL_ADC12_enablePower(ADC12_1_INST);

    DL_DAC12_enablePower(DAC0);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    // DL_GPIO_initDigitalOutput(ADC_SAMPLE_PROBE_IOMUX);
    // DL_GPIO_clearPins(ADC_SAMPLE_PROBE_PORT, ADC_SAMPLE_PROBE_PIN);
    // DL_GPIO_enableOutput(ADC_SAMPLE_PROBE_PORT, ADC_SAMPLE_PROBE_PIN);

    // DL_GPIO_initDigitalOutput(ADC_SAMPLE_PROBE_UNUSED_IOMUX);
    // DL_GPIO_clearPins(ADC_SAMPLE_PROBE_UNUSED_PORT, ADC_SAMPLE_PROBE_UNUSED_PIN);
    // DL_GPIO_enableOutput(ADC_SAMPLE_PROBE_UNUSED_PORT, ADC_SAMPLE_PROBE_UNUSED_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DEBUG_IOMUX_TX, GPIO_UART_DEBUG_IOMUX_TX_FUNC);
    // DL_GPIO_initPeripheralInputFunction(
    //     GPIO_UART_DEBUG_IOMUX_RX, GPIO_UART_DEBUG_IOMUX_RX_FUNC);

    // DL_GPIO_initPeripheralOutputFunction(
    //     GPIO_AD9833_SPI_IOMUX_SCLK, GPIO_AD9833_SPI_IOMUX_SCLK_FUNC);
    // DL_GPIO_initPeripheralOutputFunction(
    //     GPIO_AD9833_SPI_IOMUX_PICO, GPIO_AD9833_SPI_IOMUX_PICO_FUNC);
    // DL_GPIO_initPeripheralInputFunction(
    //     GPIO_AD9833_SPI_IOMUX_POCI, GPIO_AD9833_SPI_IOMUX_POCI_FUNC);
    // DL_GPIO_initPeripheralOutputFunction(
    //     GPIO_AD9833_SPI_IOMUX_CS0, GPIO_AD9833_SPI_IOMUX_CS0_FUNC);

    DL_GPIO_initDigitalOutput(ADC_MOS_SWITCH_IOMUX);
    DL_GPIO_clearPins(ADC_MOS_SWITCH_PORT, ADC_MOS_SWITCH_PIN);
    DL_GPIO_enableOutput(ADC_MOS_SWITCH_PORT, ADC_MOS_SWITCH_PIN);

    DL_GPIO_initPeripheralAnalogFunction(GPIO_ADC12_0_C0_IOMUX);
    // DL_GPIO_initPeripheralAnalogFunction(GPIO_ADC12_1_C1_IOMUX);
    // DL_GPIO_initPeripheralAnalogFunction(GPIO_DAC12_IOMUX_OUT);
    DL_GPIO_initPeripheralAnalogFunction(GPIO_VREF_IOMUX_VREFPOS);

    DL_GPIO_initDigitalInputFeatures(ADDR_CODE_ADDR0_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ADDR_CODE_ADDR1_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ADDR_CODE_ADDR2_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ADDR_CODE_ADDR3_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    // DL_GPIO_initDigitalInputFeatures(KEY_KEY1_IOMUX,
	// 	 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
	// 	 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    // DL_GPIO_initDigitalInputFeatures(KEY_KEY2_IOMUX,
	// 	 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
	// 	 DL_GPIO_HYSTERESIS_ENABLE, KEY_KEY2_WAKEUP_MODE);

    // DL_GPIO_initDigitalInputFeatures(KEY_KEY3_IOMUX,
	// 	 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
	// 	 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    // DL_GPIO_initDigitalInputFeatures(KEY_KEY4_IOMUX,
	// 	 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
	// 	 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    // DL_GPIO_initDigitalInputFeatures(KEY_KEY5_IOMUX,
	// 	 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
	// 	 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

}

static const DL_VREF_ClockConfig gVREFClockConfig = {
    .clockSel    = DL_VREF_CLOCK_BUSCLK,
    .divideRatio = DL_VREF_CLOCK_DIVIDE_1,
};

static const DL_VREF_Config gVREFConfig = {
    .vrefEnable     = DL_VREF_ENABLE_ENABLE,
    .bufConfig      = DL_VREF_BUFCONFIG_OUTPUT_1_4V,
    .shModeEnable   = DL_VREF_SHMODE_DISABLE,
    .shCycleCount   = 0U,
    .holdCycleCount = 0U,
};

SYSCONFIG_WEAK void SYSCFG_DL_VREF_init(void)
{
    DL_VREF_setClockConfig(VREF, (DL_VREF_ClockConfig *) &gVREFClockConfig);
    DL_VREF_configReference(VREF, (DL_VREF_Config *) &gVREFConfig);
    while (DL_VREF_getStatus(VREF) != DL_VREF_CTL1_READY_RDY) {
    }
}



SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    (void) Sysclk_WaitForSYSOSCBase(SYSCFG_CLOCK_WAIT_FOREVER);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_4);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_enableMFPCLK();
	DL_SYSCTL_setMFPCLKSource(DL_SYSCTL_MFPCLK_SOURCE_SYSOSC);
    (void) Sysclk_WaitForLFOSCGood(SYSCFG_CLOCK_WAIT_FOREVER);

}


/*
 * Timer clock configuration to be sourced by BUSCLK / 1 (8000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   8000000 Hz = 8000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_0ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_0Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = ADC_SAMPLE_PROBE_PWM_PERIOD,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_0_init(void) {

    DL_TimerA_setClockConfig(
        PWM_0_INST, (DL_TimerA_ClockConfig *) &gPWM_0ClockConfig);

    DL_TimerA_initPWMMode(
        PWM_0_INST, (DL_TimerA_PWMConfig *) &gPWM_0Config);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(PWM_0_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(PWM_0_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_0_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_0_INST, ADC_SAMPLE_PROBE_PWM_COMPARE, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_0_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_0_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_0_INST, ADC_SAMPLE_PROBE_PWM_COMPARE, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(PWM_0_INST);


    
    DL_TimerA_setCCPDirection(PWM_0_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by BUSCLK / 1 (8000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   8000000 Hz = 8000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gADC_SAMPLE_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * ADC_SAMPLE_TIMER_INST_LOAD_VALUE is calibrated to 630 for measured ~8.08 MHz BUSCLK.
 * Fs ~= 8080000 Hz / (630 + 1) = 12.805 kS/s.
 */
static const DL_TimerG_TimerConfig gADC_SAMPLE_TIMERTimerConfig = {
    .period     = ADC_SAMPLE_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_ADC_SAMPLE_TIMER_init(void) {

    DL_TimerG_setClockConfig(ADC_SAMPLE_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gADC_SAMPLE_TIMERClockConfig);

    DL_TimerG_initTimerMode(ADC_SAMPLE_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gADC_SAMPLE_TIMERTimerConfig);
    DL_TimerG_enableClock(ADC_SAMPLE_TIMER_INST);


    DL_TimerG_enableEvent(ADC_SAMPLE_TIMER_INST, DL_TIMERG_EVENT_ROUTE_1, (DL_TIMERG_EVENT_ZERO_EVENT));

    DL_TimerG_setPublisherChanID(ADC_SAMPLE_TIMER_INST, DL_TIMERG_PUBLISHER_INDEX_0, ADC_SAMPLE_TIMER_INST_PUB_0_CH);
    DL_TimerG_clearInterruptStatus(ADC_SAMPLE_TIMER_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);

    /* ADC1_STAGGER_TIMER publishes ZERO event on gen ch 2 (replaced TIMG0 publisher index 1) */
    DL_TimerG_enableEvent(ADC1_STAGGER_TIMER_INST, DL_TIMERG_EVENT_ROUTE_1, (DL_TIMERG_EVENT_ZERO_EVENT));
    DL_TimerG_setPublisherChanID(ADC1_STAGGER_TIMER_INST, DL_TIMERG_PUBLISHER_INDEX_0, ADC1_STAGGER_TIMER_INST_PUB_0_CH);


}


/* ADC1 stagger timer: same clock and period as ADC_SAMPLE_TIMER,
 * but counter is offset by ADC_STAGGER_DELAY_TICKS at frame start.
 * Init is minimal — period/counter set dynamically by ADC12_SetSampleRateHz(). */
SYSCONFIG_WEAK void SYSCFG_DL_ADC1_STAGGER_TIMER_init(void) {

    DL_TimerG_setClockConfig(ADC1_STAGGER_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gADC_SAMPLE_TIMERClockConfig);

    DL_TimerG_initTimerMode(ADC1_STAGGER_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gADC_SAMPLE_TIMERTimerConfig);
    DL_TimerG_enableClock(ADC1_STAGGER_TIMER_INST);

}

/*
 * Timer clock configuration to be sourced by BUSCLK / 1 (8000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   8000000 Hz = 8000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gDAC_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * DAC_TIMER_INST_LOAD_VALUE = (0 ms * 8000000 Hz) - 1
 */
static const DL_TimerA_TimerConfig gDAC_TIMERTimerConfig = {
    .period     = DAC_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_DAC_TIMER_init(void) {

    DL_TimerA_setClockConfig(DAC_TIMER_INST,
        (DL_TimerA_ClockConfig *) &gDAC_TIMERClockConfig);

    DL_TimerA_initTimerMode(DAC_TIMER_INST,
        (DL_TimerA_TimerConfig *) &gDAC_TIMERTimerConfig);
    DL_TimerA_enableClock(DAC_TIMER_INST);





}


static const DL_UART_Main_ClockConfig gUART_DEBUGClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DEBUGConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_DEBUG_init(void)
{
    DL_UART_Main_setClockConfig(UART_DEBUG_INST, (DL_UART_Main_ClockConfig *) &gUART_DEBUGClockConfig);

    DL_UART_Main_init(UART_DEBUG_INST, (DL_UART_Main_Config *) &gUART_DEBUGConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115107.91
     */
    DL_UART_Main_setOversampling(UART_DEBUG_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_DEBUG_INST, UART_DEBUG_IBRD_8_MHZ_115200_BAUD, UART_DEBUG_FBRD_8_MHZ_115200_BAUD);


    /* RX is unused in this low-power TX-only build. */

    /* Keep UART FIFOs disabled for strict byte-by-byte binary protocol output. */
    DL_UART_Main_disableFIFOs(UART_DEBUG_INST);

    DL_UART_Main_enable(UART_DEBUG_INST);
}

static const DL_SPI_Config gAD9833_SPI_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
    .chipSelectPin = DL_SPI_CHIP_SELECT_0,
};

static const DL_SPI_ClockConfig gAD9833_SPI_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_AD9833_SPI_init(void) {
    DL_SPI_setClockConfig(AD9833_SPI_INST, (DL_SPI_ClockConfig *) &gAD9833_SPI_clockConfig);

    DL_SPI_init(AD9833_SPI_INST, (DL_SPI_Config *) &gAD9833_SPI_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     1000000 = (8000000)/((1 + 3) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(AD9833_SPI_INST, 3);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(AD9833_SPI_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(AD9833_SPI_INST);
}

/* ADC12_0 Initialization */
static const DL_ADC12_ClockConfig gADC12_0ClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC12_0_init(void)
{
    DL_ADC12_setClockConfig(ADC12_0_INST, (DL_ADC12_ClockConfig *) &gADC12_0ClockConfig);
    DL_ADC12_initSingleSample(ADC12_0_INST,
        DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_MANUAL, DL_ADC12_TRIG_SRC_EVENT,
        DL_ADC12_SAMP_CONV_RES_12_BIT, DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC12_0_INST, ADC12_0_ADCMEM_0,
        DL_ADC12_INPUT_CHAN_7, DL_ADC12_REFERENCE_VOLTAGE_INTREF, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(ADC12_0_INST,DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(ADC12_0_INST,2);
    // DL_ADC12_enableDMA(ADC12_0_INST);
    DL_ADC12_setDMASamplesCnt(ADC12_0_INST,1);
    // DL_ADC12_enableDMATrigger(ADC12_0_INST,(DL_ADC12_DMA_MEM0_RESULT_LOADED));
    DL_ADC12_setSubscriberChanID(ADC12_0_INST,ADC12_0_INST_SUB_CH);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC12_0_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    // DL_ADC12_enableInterrupt(ADC12_0_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    // DL_ADC12_enableConversions(ADC12_0_INST);
}
/* ADC12_1 Initialization */
static const DL_ADC12_ClockConfig gADC12_1ClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC12_1_init(void)
{
    DL_ADC12_setClockConfig(ADC12_1_INST, (DL_ADC12_ClockConfig *) &gADC12_1ClockConfig);
    DL_ADC12_initSingleSample(ADC12_1_INST,
        DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_MANUAL, DL_ADC12_TRIG_SRC_EVENT,
        DL_ADC12_SAMP_CONV_RES_12_BIT, DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC12_1_INST, ADC12_1_ADCMEM_0,
        DL_ADC12_INPUT_CHAN_7, DL_ADC12_REFERENCE_VOLTAGE_INTREF, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(ADC12_1_INST,DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(ADC12_1_INST,2);
    DL_ADC12_enableDMA(ADC12_1_INST);
    DL_ADC12_setDMASamplesCnt(ADC12_1_INST,1);
    DL_ADC12_enableDMATrigger(ADC12_1_INST,(DL_ADC12_DMA_MEM0_RESULT_LOADED));
    DL_ADC12_setSubscriberChanID(ADC12_1_INST,ADC12_1_INST_SUB_CH);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC12_1_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    DL_ADC12_enableInterrupt(ADC12_1_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    DL_ADC12_enableConversions(ADC12_1_INST);
}

static const DL_DMA_Config gDMA_CH0Config = {
    .transferMode   = DL_DMA_FULL_CH_REPEAT_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_INCREMENT,
    .srcIncrement   = DL_DMA_ADDR_UNCHANGED,
    .destWidth      = DL_DMA_WIDTH_HALF_WORD,
    .srcWidth       = DL_DMA_WIDTH_HALF_WORD,
    .trigger        = ADC12_0_INST_DMA_TRIGGER,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_CH0_init(void)
{
    DL_DMA_initChannel(DMA, DMA_CH0_CHAN_ID , (DL_DMA_Config *) &gDMA_CH0Config);
}
static const DL_DMA_Config gDMA_CH1Config = {
    .transferMode   = DL_DMA_FULL_CH_REPEAT_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_INCREMENT,
    .srcIncrement   = DL_DMA_ADDR_UNCHANGED,
    .destWidth      = DL_DMA_WIDTH_HALF_WORD,
    .srcWidth       = DL_DMA_WIDTH_HALF_WORD,
    .trigger        = ADC12_1_INST_DMA_TRIGGER,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_CH1_init(void)
{
    DL_DMA_initChannel(DMA, DMA_CH1_CHAN_ID , (DL_DMA_Config *) &gDMA_CH1Config);
}
static const DL_DMA_Config gDACConfig = {
    .transferMode   = DL_DMA_FULL_CH_REPEAT_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement   = DL_DMA_ADDR_UNCHANGED,
    .destWidth      = DL_DMA_WIDTH_HALF_WORD,
    .srcWidth       = DL_DMA_WIDTH_HALF_WORD,
    .trigger        = DMA_CH_2_TRIG,
    .triggerType    = DL_DMA_TRIGGER_TYPE_INTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DAC_init(void)
{
    DL_DMA_setTransferSize(DMA, DAC_CHAN_ID, 1024);
    DL_DMA_initChannel(DMA, DAC_CHAN_ID , (DL_DMA_Config *) &gDACConfig);
}
SYSCONFIG_WEAK void SYSCFG_DL_DMA_init(void){
    SYSCFG_DL_DMA_CH0_init();
    // SYSCFG_DL_DMA_CH1_init();
    // SYSCFG_DL_DAC_init();
}


static const DL_DAC12_Config gDAC12Config = {
    .outputEnable              = DL_DAC12_OUTPUT_ENABLED,
    .resolution                = DL_DAC12_RESOLUTION_12BIT,
    .representation            = DL_DAC12_REPRESENTATION_BINARY,
    .voltageReferenceSource    = DL_DAC12_VREF_SOURCE_VDDA_VSSA,
    .amplifierSetting          = DL_DAC12_AMP_ON,
    .fifoEnable                = DL_DAC12_FIFO_ENABLED,
    .fifoTriggerSource         = DL_DAC12_FIFO_TRIGGER_SAMPLETIMER,
    .dmaTriggerEnable          = DL_DAC12_DMA_TRIGGER_DISABLED,
    .dmaTriggerThreshold       = DL_DAC12_FIFO_THRESHOLD_ONE_QTR_EMPTY,
    .sampleTimeGeneratorEnable = DL_DAC12_SAMPLETIMER_ENABLE,
    .sampleRate                = DL_DAC12_SAMPLES_PER_SECOND_1M,
};
SYSCONFIG_WEAK void SYSCFG_DL_DAC12_init(void)
{
    DL_DAC12_init(DAC0, (DL_DAC12_Config *) &gDAC12Config);
    DL_DAC12_output12(DAC0, 2048);
    DL_DAC12_enable(DAC0);
}

