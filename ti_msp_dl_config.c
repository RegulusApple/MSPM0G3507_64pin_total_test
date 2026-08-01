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

DL_TimerG_backupConfig gSQUARE_TIMERBackup;
DL_TimerA_backupConfig gPWM_TIMERBackup;
DL_TimerG_backupConfig gPSU_CTRL_TIMERBackup;

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
    SYSCFG_DL_SQUARE_TIMER_init();
    SYSCFG_DL_PWM_TIMER_init();
    SYSCFG_DL_FREQ_CAPTURE_init();
    SYSCFG_DL_ADC_SAMPLE_TIMER_init();
    SYSCFG_DL_PSU_CTRL_TIMER_init();
    SYSCFG_DL_LP_WAKE_TIMER_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_ADC12_0_init();
    SYSCFG_DL_ADC12_1_init();
    SYSCFG_DL_COMP_0_init();
    SYSCFG_DL_VREF_init();
    SYSCFG_DL_OPA_0_init();
    SYSCFG_DL_DMA_init();
    SYSCFG_DL_RTC_init();
    SYSCFG_DL_DAC12_init();
    /* Ensure backup structures have no valid state */
	gSQUARE_TIMERBackup.backupRdy 	= false;
	gPWM_TIMERBackup.backupRdy 	= false;

	gPSU_CTRL_TIMERBackup.backupRdy 	= false;


}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_saveConfiguration(SQUARE_TIMER_INST, &gSQUARE_TIMERBackup);
	retStatus &= DL_TimerA_saveConfiguration(PWM_TIMER_INST, &gPWM_TIMERBackup);
	retStatus &= DL_TimerG_saveConfiguration(PSU_CTRL_TIMER_INST, &gPSU_CTRL_TIMERBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_restoreConfiguration(SQUARE_TIMER_INST, &gSQUARE_TIMERBackup, false);
	retStatus &= DL_TimerA_restoreConfiguration(PWM_TIMER_INST, &gPWM_TIMERBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(PSU_CTRL_TIMER_INST, &gPSU_CTRL_TIMERBackup, false);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerG_reset(SQUARE_TIMER_INST);
    DL_TimerA_reset(PWM_TIMER_INST);
    DL_TimerG_reset(FREQ_CAPTURE_INST);
    DL_TimerG_reset(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_reset(PSU_CTRL_TIMER_INST);
    DL_TimerG_reset(LP_WAKE_TIMER_INST);
    DL_UART_Main_reset(UART_DEBUG_INST);
    DL_ADC12_reset(ADC12_0_INST);
    DL_ADC12_reset(ADC12_1_INST);
    DL_COMP_reset(COMP_0_INST);
    DL_VREF_reset(VREF);
    DL_OPA_reset(OPA_0_INST);

    DL_RTC_reset(RTC);
    DL_DAC12_reset(DAC0);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_enablePower(SQUARE_TIMER_INST);
    DL_TimerA_enablePower(PWM_TIMER_INST);
    DL_TimerG_enablePower(FREQ_CAPTURE_INST);
    DL_TimerG_enablePower(ADC_SAMPLE_TIMER_INST);
    DL_TimerG_enablePower(PSU_CTRL_TIMER_INST);
    DL_TimerG_enablePower(LP_WAKE_TIMER_INST);
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    DL_ADC12_enablePower(ADC12_0_INST);
    DL_ADC12_enablePower(ADC12_1_INST);
    DL_COMP_enablePower(COMP_0_INST);
    DL_VREF_enablePower(VREF);
    DL_OPA_enablePower(OPA_0_INST);

    DL_RTC_enablePower(RTC);
    DL_DAC12_enablePower(DAC0);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralAnalogFunction(GPIO_LFXIN_IOMUX);
    DL_GPIO_initPeripheralAnalogFunction(GPIO_LFXOUT_IOMUX);

    DL_GPIO_initPeripheralOutputFunction(GPIO_SQUARE_TIMER_C0_IOMUX,GPIO_SQUARE_TIMER_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_SQUARE_TIMER_C0_PORT, GPIO_SQUARE_TIMER_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_TIMER_C1_IOMUX,GPIO_PWM_TIMER_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_TIMER_C1_PORT, GPIO_PWM_TIMER_C1_PIN);

    DL_GPIO_initPeripheralInputFunction(GPIO_FREQ_CAPTURE_C0_IOMUX,GPIO_FREQ_CAPTURE_C0_IOMUX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DEBUG_IOMUX_TX, GPIO_UART_DEBUG_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_DEBUG_IOMUX_RX, GPIO_UART_DEBUG_IOMUX_RX_FUNC);

    DL_GPIO_initDigitalInputFeatures(KEY_KEY1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_KEY2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_KEY3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_KEY4_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_KEY5_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(KEY_LED_LED_IOMUX);

    DL_GPIO_initDigitalInputFeatures(KEY_LED_BUTTON_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInput(I2C_LCD_PINS_LCD_SDA_IOMUX);

    DL_GPIO_initDigitalInput(I2C_LCD_PINS_LCD_SCL_IOMUX);

    DL_GPIO_initDigitalOutput(BOARD_CONTROL_WIRELESS_POWER_IOMUX);

    DL_GPIO_initDigitalOutput(BOARD_CONTROL_ADC_MOS_SWITCH_IOMUX);

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

    DL_GPIO_initDigitalOutput(DDS_CTRL_DDS_WCLK_IOMUX);

    DL_GPIO_initDigitalOutput(DDS_CTRL_DDS_FQUD_IOMUX);

    DL_GPIO_initDigitalOutput(DDS_CTRL_DDS_DATA_IOMUX);

    DL_GPIO_initDigitalOutput(DDS_CTRL_DDS_RESET_IOMUX);

    DL_GPIO_clearPins(GPIOA, BOARD_CONTROL_WIRELESS_POWER_PIN |
		BOARD_CONTROL_ADC_MOS_SWITCH_PIN |
		DDS_CTRL_DDS_WCLK_PIN |
		DDS_CTRL_DDS_FQUD_PIN |
		DDS_CTRL_DDS_DATA_PIN |
		DDS_CTRL_DDS_RESET_PIN);
    DL_GPIO_enableOutput(GPIOA, BOARD_CONTROL_WIRELESS_POWER_PIN |
		BOARD_CONTROL_ADC_MOS_SWITCH_PIN |
		DDS_CTRL_DDS_WCLK_PIN |
		DDS_CTRL_DDS_FQUD_PIN |
		DDS_CTRL_DDS_DATA_PIN |
		DDS_CTRL_DDS_RESET_PIN);
    DL_GPIO_clearPins(GPIOB, KEY_LED_LED_PIN);
    DL_GPIO_enableOutput(GPIOB, KEY_LED_LED_PIN);

}



static const DL_SYSCTL_LFCLKConfig gLFCLKConfig = {
    .lowCap   = false,
    .monitor  = false,
    .xt1Drive = DL_SYSCTL_LFXT_DRIVE_STRENGTH_HIGHEST,
};
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setLFCLKSourceLFXT((DL_SYSCTL_LFCLKConfig *) &gLFCLKConfig);
    DL_SYSCTL_enableMFPCLK();
	DL_SYSCTL_setMFPCLKSource(DL_SYSCTL_MFPCLK_SOURCE_SYSOSC);

}


/*
 * Timer clock configuration to be sourced by  / 1 (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gSQUARE_TIMERClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gSQUARE_TIMERConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 32000,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_SQUARE_TIMER_init(void) {

    DL_TimerG_setClockConfig(
        SQUARE_TIMER_INST, (DL_TimerG_ClockConfig *) &gSQUARE_TIMERClockConfig);

    DL_TimerG_initPWMMode(
        SQUARE_TIMER_INST, (DL_TimerG_PWMConfig *) &gSQUARE_TIMERConfig);

    DL_TimerG_setCaptureCompareOutCtl(SQUARE_TIMER_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(SQUARE_TIMER_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(SQUARE_TIMER_INST, 16000, DL_TIMER_CC_0_INDEX);

    DL_TimerG_enableClock(SQUARE_TIMER_INST);


    
    DL_TimerG_setCCPDirection(SQUARE_TIMER_INST , DL_TIMER_CC0_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 1 (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_TIMERClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_TIMERConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 32000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_TIMER_init(void) {

    DL_TimerA_setClockConfig(
        PWM_TIMER_INST, (DL_TimerA_ClockConfig *) &gPWM_TIMERClockConfig);

    DL_TimerA_initPWMMode(
        PWM_TIMER_INST, (DL_TimerA_PWMConfig *) &gPWM_TIMERConfig);

    DL_TimerA_setCaptureCompareOutCtl(PWM_TIMER_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_TIMER_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_TIMER_INST, 16000, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(PWM_TIMER_INST);


    
    DL_TimerA_setCCPDirection(PWM_TIMER_INST , DL_TIMER_CC1_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gFREQ_CAPTUREClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * FREQ_CAPTURE_INST_LOAD_VALUE = (1 s * 32000000 Hz) - 1
 */
static const DL_TimerG_CaptureConfig gFREQ_CAPTURECaptureConfig = {
    .captureMode    = DL_TIMER_CAPTURE_MODE_PERIOD_CAPTURE,
    .period         = FREQ_CAPTURE_INST_LOAD_VALUE,
    .startTimer     = DL_TIMER_STOP,
    .edgeCaptMode   = DL_TIMER_CAPTURE_EDGE_DETECTION_MODE_RISING,
    .inputChan      = DL_TIMER_INPUT_CHAN_0,
    .inputInvMode   = DL_TIMER_CC_INPUT_INV_NOINVERT,
};

SYSCONFIG_WEAK void SYSCFG_DL_FREQ_CAPTURE_init(void) {

    DL_TimerG_setClockConfig(FREQ_CAPTURE_INST,
        (DL_TimerG_ClockConfig *) &gFREQ_CAPTUREClockConfig);

    DL_TimerG_initCaptureMode(FREQ_CAPTURE_INST,
        (DL_TimerG_CaptureConfig *) &gFREQ_CAPTURECaptureConfig);
    DL_TimerG_enableInterrupt(FREQ_CAPTURE_INST , DL_TIMERG_INTERRUPT_CC0_DN_EVENT |
		DL_TIMERG_INTERRUPT_ZERO_EVENT);

    DL_TimerG_enableClock(FREQ_CAPTURE_INST);

}


/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gADC_SAMPLE_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * ADC_SAMPLE_TIMER_INST_LOAD_VALUE = (78.125 us * 32000000 Hz) - 1
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

    DL_TimerG_enableEvent(ADC_SAMPLE_TIMER_INST, DL_TIMERG_EVENT_ROUTE_2, (DL_TIMERG_EVENT_ZERO_EVENT));

    DL_TimerG_setPublisherChanID(ADC_SAMPLE_TIMER_INST, DL_TIMERG_PUBLISHER_INDEX_1, ADC_SAMPLE_TIMER_INST_PUB_1_CH);


}

/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gPSU_CTRL_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * PSU_CTRL_TIMER_INST_LOAD_VALUE = (50 us * 32000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gPSU_CTRL_TIMERTimerConfig = {
    .period     = PSU_CTRL_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PSU_CTRL_TIMER_init(void) {

    DL_TimerG_setClockConfig(PSU_CTRL_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gPSU_CTRL_TIMERClockConfig);

    DL_TimerG_initTimerMode(PSU_CTRL_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gPSU_CTRL_TIMERTimerConfig);
    DL_TimerG_enableInterrupt(PSU_CTRL_TIMER_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(PSU_CTRL_TIMER_INST);





}

/*
 * Timer clock configuration to be sourced by LFCLK /  (32000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000 Hz = 32000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gLP_WAKE_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_LFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * LP_WAKE_TIMER_INST_LOAD_VALUE = (500 ms * 32000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gLP_WAKE_TIMERTimerConfig = {
    .period     = LP_WAKE_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_LP_WAKE_TIMER_init(void) {

    DL_TimerG_setClockConfig(LP_WAKE_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gLP_WAKE_TIMERClockConfig);

    DL_TimerG_initTimerMode(LP_WAKE_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gLP_WAKE_TIMERTimerConfig);
    DL_TimerG_enableInterrupt(LP_WAKE_TIMER_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(LP_WAKE_TIMER_INST);





}



static const DL_UART_Main_ClockConfig gUART_DEBUGClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DEBUGConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
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
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(UART_DEBUG_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_DEBUG_INST, UART_DEBUG_IBRD_32_MHZ_115200_BAUD, UART_DEBUG_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_DEBUG_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);

    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_DEBUG_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_DEBUG_INST, DL_UART_RX_FIFO_LEVEL_1_2_FULL);
    DL_UART_Main_setTXFIFOThreshold(UART_DEBUG_INST, DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_Main_enable(UART_DEBUG_INST);
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
        DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_EVENT,
        DL_ADC12_SAMP_CONV_RES_12_BIT, DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC12_0_INST, ADC12_0_ADCMEM_0,
        DL_ADC12_INPUT_CHAN_7, DL_ADC12_REFERENCE_VOLTAGE_INTREF, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(ADC12_0_INST,DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(ADC12_0_INST,16);
    DL_ADC12_enableDMA(ADC12_0_INST);
    DL_ADC12_setDMASamplesCnt(ADC12_0_INST,1);
    DL_ADC12_enableDMATrigger(ADC12_0_INST,(DL_ADC12_DMA_MEM0_RESULT_LOADED));
    DL_ADC12_setSubscriberChanID(ADC12_0_INST,ADC12_0_INST_SUB_CH);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC12_0_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    DL_ADC12_enableInterrupt(ADC12_0_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    DL_ADC12_enableConversions(ADC12_0_INST);
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
        DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_EVENT,
        DL_ADC12_SAMP_CONV_RES_12_BIT, DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC12_1_INST, ADC12_1_ADCMEM_0,
        DL_ADC12_INPUT_CHAN_7, DL_ADC12_REFERENCE_VOLTAGE_INTREF, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(ADC12_1_INST,DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(ADC12_1_INST,16);
    DL_ADC12_enableDMA(ADC12_1_INST);
    DL_ADC12_setDMASamplesCnt(ADC12_1_INST,1);
    DL_ADC12_enableDMATrigger(ADC12_1_INST,(DL_ADC12_DMA_MEM0_RESULT_LOADED));
    DL_ADC12_setSubscriberChanID(ADC12_1_INST,ADC12_1_INST_SUB_CH);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC12_1_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    DL_ADC12_enableInterrupt(ADC12_1_INST,(DL_ADC12_INTERRUPT_DMA_DONE));
    DL_ADC12_enableConversions(ADC12_1_INST);
}

/* COMP_0 Initialization */
static const DL_COMP_Config gCOMP_0Config = {
    .channelEnable = DL_COMP_ENABLE_CHANNEL_POS_NEG,
    .mode          = DL_COMP_MODE_FAST,
    .negChannel    = DL_COMP_IMSEL_CHANNEL_0,
    .posChannel    = DL_COMP_IPSEL_CHANNEL_0,
    .hysteresis    = DL_COMP_HYSTERESIS_NONE,
    .polarity      = DL_COMP_POLARITY_NON_INV
};
static const DL_COMP_RefVoltageConfig gCOMP_0VRefConfig = {
    .mode           = DL_COMP_REF_MODE_STATIC,
    .source         = DL_COMP_REF_SOURCE_VDDA_DAC,
    .terminalSelect = DL_COMP_REF_TERMINAL_SELECT_POS,
    .controlSelect  = DL_COMP_DAC_CONTROL_SW,
    .inputSelect    = DL_COMP_DAC_INPUT_DACCODE0
};

SYSCONFIG_WEAK void SYSCFG_DL_COMP_0_init(void)
{
    DL_COMP_init(COMP_0_INST, (DL_COMP_Config *) &gCOMP_0Config);
    DL_COMP_refVoltageInit(COMP_0_INST, (DL_COMP_RefVoltageConfig *) &gCOMP_0VRefConfig);
    DL_COMP_setDACCode0(COMP_0_INST, COMP_0_DACCODE0);
    DL_COMP_enableInterrupt(COMP_0_INST, (DL_COMP_INTERRUPT_OUTPUT_EDGE));

    DL_COMP_enable(COMP_0_INST);

}



static const DL_VREF_ClockConfig gVREFClockConfig = {
    .clockSel = DL_VREF_CLOCK_LFCLK,
    .divideRatio = DL_VREF_CLOCK_DIVIDE_1,
};
static const DL_VREF_Config gVREFConfig = {
    .vrefEnable     = DL_VREF_ENABLE_ENABLE,
    .bufConfig      = DL_VREF_BUFCONFIG_OUTPUT_2_5V,
    .shModeEnable   = DL_VREF_SHMODE_DISABLE,
    .holdCycleCount = DL_VREF_HOLD_MIN,
    .shCycleCount   = DL_VREF_SH_MIN,
};

SYSCONFIG_WEAK void SYSCFG_DL_VREF_init(void) {
    DL_VREF_setClockConfig(VREF,
        (DL_VREF_ClockConfig *) &gVREFClockConfig);
    DL_VREF_configReference(VREF,
        (DL_VREF_Config *) &gVREFConfig);
}


static const DL_OPA_Config gOPA_0Config0 = {
    .pselChannel    = DL_OPA_PSEL_IN0_POS,
    .nselChannel    = DL_OPA_NSEL_RTAP,
    .mselChannel    = DL_OPA_MSEL_GND,
    .gain           = DL_OPA_GAIN_N1_P2,
    .outputPinState = DL_OPA_OUTPUT_PIN_ENABLED,
    .choppingMode   = DL_OPA_CHOPPING_MODE_DISABLE,
};

SYSCONFIG_WEAK void SYSCFG_DL_OPA_0_init(void)
{
    DL_OPA_init(OPA_0_INST, (DL_OPA_Config *) &gOPA_0Config0);

    DL_OPA_enable(OPA_0_INST);
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
static const DL_DMA_Config gDMA_DACConfig = {
    .transferMode   = DL_DMA_FULL_CH_REPEAT_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement   = DL_DMA_ADDR_INCREMENT,
    .destWidth      = DL_DMA_WIDTH_HALF_WORD,
    .srcWidth       = DL_DMA_WIDTH_HALF_WORD,
    .trigger        = DAC12_INST_DMA_TRIGGER,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_DAC_init(void)
{
    DL_DMA_initChannel(DMA, DMA_DAC_CHAN_ID , (DL_DMA_Config *) &gDMA_DACConfig);
}
SYSCONFIG_WEAK void SYSCFG_DL_DMA_init(void){
    SYSCFG_DL_DMA_CH0_init();
    SYSCFG_DL_DMA_CH1_init();
    SYSCFG_DL_DMA_DAC_init();
}


static const DL_RTC_Calendar gRTCCalendarConfig = {
		.seconds    = 0,   /* Seconds = 0 */
		.minutes    = 0,   /* Minute = 0 */
		.hours      = 0,   /* Hour = 0 */
		.dayOfWeek  = 0,    /* Day of week = 0 (Sunday) */
		.dayOfMonth = 1,    /* Day of month = 1*/
		.month      = 1,    /* Month = 1 (January) */
		.year       = 2026, /* Year = 2026 */
};




SYSCONFIG_WEAK void SYSCFG_DL_RTC_init(void)
{
	/* Initialize RTC Calendar */
	DL_RTC_initCalendar(RTC , gRTCCalendarConfig, DL_RTC_FORMAT_BINARY);

}

static const DL_DAC12_Config gDAC12Config = {
    .outputEnable              = DL_DAC12_OUTPUT_ENABLED,
    .resolution                = DL_DAC12_RESOLUTION_12BIT,
    .representation            = DL_DAC12_REPRESENTATION_BINARY,
    .voltageReferenceSource    = DL_DAC12_VREF_SOURCE_VDDA_VSSA,
    .amplifierSetting          = DL_DAC12_AMP_ON,
    .fifoEnable                = DL_DAC12_FIFO_ENABLED,
    .fifoTriggerSource         = DL_DAC12_FIFO_TRIGGER_SAMPLETIMER,
    .dmaTriggerEnable          = DL_DAC12_DMA_TRIGGER_ENABLED,
    .dmaTriggerThreshold       = DL_DAC12_FIFO_THRESHOLD_TWO_QTRS_EMPTY,
    .sampleTimeGeneratorEnable = DL_DAC12_SAMPLETIMER_ENABLE,
    .sampleRate                = DL_DAC12_SAMPLES_PER_SECOND_1M,
};
SYSCONFIG_WEAK void SYSCFG_DL_DAC12_init(void)
{
    DL_DAC12_init(DAC0, (DL_DAC12_Config *) &gDAC12Config);
    DL_DAC12_output12(DAC0, 2048);
    DL_DAC12_enable(DAC0);
}

