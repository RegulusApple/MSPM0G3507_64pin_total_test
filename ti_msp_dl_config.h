/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_LFXT_PORT                                                     GPIOA
#define GPIO_LFXIN_PIN                                             DL_GPIO_PIN_3
#define GPIO_LFXIN_IOMUX                                          (IOMUX_PINCM8)
#define GPIO_LFXOUT_PIN                                            DL_GPIO_PIN_4
#define GPIO_LFXOUT_IOMUX                                         (IOMUX_PINCM9)
#define CPUCLK_FREQ                                                     32000000



/* Defines for SQUARE_TIMER */
#define SQUARE_TIMER_INST                                                  TIMG7
#define SQUARE_TIMER_INST_IRQHandler                            TIMG7_IRQHandler
#define SQUARE_TIMER_INST_INT_IRQN                              (TIMG7_INT_IRQn)
#define SQUARE_TIMER_INST_CLK_FREQ                                      32000000
/* GPIO defines for channel 0 */
#define GPIO_SQUARE_TIMER_C0_PORT                                          GPIOB
#define GPIO_SQUARE_TIMER_C0_PIN                                  DL_GPIO_PIN_15
#define GPIO_SQUARE_TIMER_C0_IOMUX                               (IOMUX_PINCM32)
#define GPIO_SQUARE_TIMER_C0_IOMUX_FUNC              IOMUX_PINCM32_PF_TIMG7_CCP0
#define GPIO_SQUARE_TIMER_C0_IDX                             DL_TIMER_CC_0_INDEX

/* Defines for PWM_TIMER */
#define PWM_TIMER_INST                                                     TIMA0
#define PWM_TIMER_INST_IRQHandler                               TIMA0_IRQHandler
#define PWM_TIMER_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define PWM_TIMER_INST_CLK_FREQ                                         32000000
/* GPIO defines for channel 1 */
#define GPIO_PWM_TIMER_C1_PORT                                             GPIOA
#define GPIO_PWM_TIMER_C1_PIN                                      DL_GPIO_PIN_9
#define GPIO_PWM_TIMER_C1_IOMUX                                  (IOMUX_PINCM20)
#define GPIO_PWM_TIMER_C1_IOMUX_FUNC                 IOMUX_PINCM20_PF_TIMA0_CCP1
#define GPIO_PWM_TIMER_C1_IDX                                DL_TIMER_CC_1_INDEX



/* Defines for FREQ_CAPTURE */
#define FREQ_CAPTURE_INST                                               (TIMG12)
#define FREQ_CAPTURE_INST_IRQHandler                           TIMG12_IRQHandler
#define FREQ_CAPTURE_INST_INT_IRQN                             (TIMG12_INT_IRQn)
#define FREQ_CAPTURE_INST_LOAD_VALUE                                 (31999999U)
/* GPIO defines for channel 0 */
#define GPIO_FREQ_CAPTURE_C0_PORT                                          GPIOB
#define GPIO_FREQ_CAPTURE_C0_PIN                                  DL_GPIO_PIN_13
#define GPIO_FREQ_CAPTURE_C0_IOMUX                               (IOMUX_PINCM30)
#define GPIO_FREQ_CAPTURE_C0_IOMUX_FUNC             IOMUX_PINCM30_PF_TIMG12_CCP0





/* Defines for ADC_SAMPLE_TIMER */
#define ADC_SAMPLE_TIMER_INST                                            (TIMG0)
#define ADC_SAMPLE_TIMER_INST_IRQHandler                        TIMG0_IRQHandler
#define ADC_SAMPLE_TIMER_INST_INT_IRQN                          (TIMG0_INT_IRQn)
#define ADC_SAMPLE_TIMER_INST_LOAD_VALUE                                 (2499U)
#define ADC_SAMPLE_TIMER_INST_PUB_0_CH                                       (1)
#define ADC_SAMPLE_TIMER_INST_PUB_1_CH                                       (2)
/* Defines for PSU_CTRL_TIMER */
#define PSU_CTRL_TIMER_INST                                              (TIMG6)
#define PSU_CTRL_TIMER_INST_IRQHandler                          TIMG6_IRQHandler
#define PSU_CTRL_TIMER_INST_INT_IRQN                            (TIMG6_INT_IRQn)
#define PSU_CTRL_TIMER_INST_LOAD_VALUE                                   (1599U)
/* Defines for LP_WAKE_TIMER */
#define LP_WAKE_TIMER_INST                                               (TIMG8)
#define LP_WAKE_TIMER_INST_IRQHandler                           TIMG8_IRQHandler
#define LP_WAKE_TIMER_INST_INT_IRQN                             (TIMG8_INT_IRQn)
#define LP_WAKE_TIMER_INST_LOAD_VALUE                                   (15999U)



/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_IRQHandler                              UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN                                  UART0_INT_IRQn
#define GPIO_UART_DEBUG_RX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_TX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_RX_PIN                                    DL_GPIO_PIN_11
#define GPIO_UART_DEBUG_TX_PIN                                    DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_IOMUX_RX                                 (IOMUX_PINCM22)
#define GPIO_UART_DEBUG_IOMUX_TX                                 (IOMUX_PINCM21)
#define GPIO_UART_DEBUG_IOMUX_RX_FUNC                  IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_DEBUG_IOMUX_TX_FUNC                  IOMUX_PINCM21_PF_UART0_TX
#define UART_DEBUG_BAUD_RATE                                            (115200)
#define UART_DEBUG_IBRD_32_MHZ_115200_BAUD                                  (17)
#define UART_DEBUG_FBRD_32_MHZ_115200_BAUD                                  (23)





/* Defines for ADC12_0 */
#define ADC12_0_INST                                                        ADC0
#define ADC12_0_INST_IRQHandler                                  ADC0_IRQHandler
#define ADC12_0_INST_INT_IRQN                                    (ADC0_INT_IRQn)
#define ADC12_0_ADCMEM_0                                      DL_ADC12_MEM_IDX_0
#define ADC12_0_ADCMEM_0_REF                   DL_ADC12_REFERENCE_VOLTAGE_INTREF
#define ADC12_0_ADCMEM_0_REF_VOLTAGE_V                                      2.50
#define ADC12_0_INST_SUB_CH                                                  (1)
#define GPIO_ADC12_0_C7_PORT                                               GPIOA
#define GPIO_ADC12_0_C7_PIN                                       DL_GPIO_PIN_22

/* Defines for ADC12_1 */
#define ADC12_1_INST                                                        ADC1
#define ADC12_1_INST_IRQHandler                                  ADC1_IRQHandler
#define ADC12_1_INST_INT_IRQN                                    (ADC1_INT_IRQn)
#define ADC12_1_ADCMEM_0                                      DL_ADC12_MEM_IDX_0
#define ADC12_1_ADCMEM_0_REF                   DL_ADC12_REFERENCE_VOLTAGE_INTREF
#define ADC12_1_ADCMEM_0_REF_VOLTAGE_V                                      2.50
#define ADC12_1_INST_SUB_CH                                                  (2)
#define GPIO_ADC12_1_C7_PORT                                               GPIOA
#define GPIO_ADC12_1_C7_PIN                                       DL_GPIO_PIN_21



/* Defines for COMP_0 */
#define COMP_0_INST                                                        COMP1
#define COMP_0_INST_INT_IRQN                                      COMP1_INT_IRQn

/* Defines for COMP_0 DACCODE0 */
#define COMP_0_DACCODE0                                                      (0)


/* GPIO configuration for COMP_0 */
#define GPIO_COMP_0_IN0P_PORT                                            (GPIOB)
#define GPIO_COMP_0_IN0P_PIN                                    (DL_GPIO_PIN_26)
#define GPIO_COMP_0_IOMUX_IN0P                                   (IOMUX_PINCM57)
#define GPIO_COMP_0_IOMUX_IN0P_FUNC               (IOMUX_PINCM57_PF_UNCONNECTED)

#define GPIO_COMP_0_IN0N_PORT                                            (GPIOB)
#define GPIO_COMP_0_IN0N_PIN                                    (DL_GPIO_PIN_27)
#define GPIO_COMP_0_IOMUX_IN0N                                   (IOMUX_PINCM58)
#define GPIO_COMP_0_IOMUX_IN0N_FUNC               (IOMUX_PINCM58_PF_UNCONNECTED)



/* Defines for VREF */
#define VREF_VOLTAGE_MV                                                     2500
#define GPIO_VREF_VREFPOS_PORT                                             GPIOA
#define GPIO_VREF_VREFPOS_PIN                                     DL_GPIO_PIN_23
#define GPIO_VREF_IOMUX_VREFPOS                                  (IOMUX_PINCM53)
#define GPIO_VREF_IOMUX_VREFPOS_FUNC                IOMUX_PINCM53_PF_UNCONNECTED



/* Defines for OPA_0 */
#define OPA_0_INST                                                          OPA1
#define GPIO_OPA_0_IN0POS_PORT                                             GPIOB
#define GPIO_OPA_0_IN0POS_PIN                                     DL_GPIO_PIN_19
#define GPIO_OPA_0_IOMUX_IN0POS                                  (IOMUX_PINCM45)
#define GPIO_OPA_0_IOMUX_IN0POS_FUNC                IOMUX_PINCM45_PF_UNCONNECTED
#define GPIO_OPA_0_OUT_PORT                                                GPIOA
#define GPIO_OPA_0_OUT_PIN                                        DL_GPIO_PIN_16
#define GPIO_OPA_0_IOMUX_OUT                                     (IOMUX_PINCM38)
#define GPIO_OPA_0_IOMUX_OUT_FUNC                   IOMUX_PINCM38_PF_UNCONNECTED



/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (0)
#define ADC12_0_INST_DMA_TRIGGER                      (DMA_ADC0_EVT_GEN_BD_TRIG)

/* Defines for DMA_CH1 */
#define DMA_CH1_CHAN_ID                                                      (1)
#define ADC12_1_INST_DMA_TRIGGER                      (DMA_ADC1_EVT_GEN_BD_TRIG)

/* Defines for DMA_DAC */
#define DMA_DAC_CHAN_ID                                                      (2)
#define DAC12_INST_DMA_TRIGGER                          (DMA_DAC0_EVT_BD_1_TRIG)



/* Defines for KEY1: GPIOA.8 with pinCMx 19 on package pin 54 */
#define KEY_KEY1_PORT                                                    (GPIOA)
#define KEY_KEY1_PIN                                             (DL_GPIO_PIN_8)
#define KEY_KEY1_IOMUX                                           (IOMUX_PINCM19)
/* Defines for KEY2: GPIOB.3 with pinCMx 16 on package pin 51 */
#define KEY_KEY2_PORT                                                    (GPIOB)
#define KEY_KEY2_PIN                                             (DL_GPIO_PIN_3)
#define KEY_KEY2_IOMUX                                           (IOMUX_PINCM16)
/* Defines for KEY3: GPIOB.2 with pinCMx 15 on package pin 50 */
#define KEY_KEY3_PORT                                                    (GPIOB)
#define KEY_KEY3_PIN                                             (DL_GPIO_PIN_2)
#define KEY_KEY3_IOMUX                                           (IOMUX_PINCM15)
/* Defines for KEY4: GPIOA.7 with pinCMx 14 on package pin 49 */
#define KEY_KEY4_PORT                                                    (GPIOA)
#define KEY_KEY4_PIN                                             (DL_GPIO_PIN_7)
#define KEY_KEY4_IOMUX                                           (IOMUX_PINCM14)
/* Defines for KEY5: GPIOA.31 with pinCMx 6 on package pin 39 */
#define KEY_KEY5_PORT                                                    (GPIOA)
#define KEY_KEY5_PIN                                            (DL_GPIO_PIN_31)
#define KEY_KEY5_IOMUX                                            (IOMUX_PINCM6)
/* Port definition for Pin Group KEY_LED */
#define KEY_LED_PORT                                                     (GPIOB)

/* Defines for LED: GPIOB.14 with pinCMx 31 on package pin 2 */
#define KEY_LED_LED_PIN                                         (DL_GPIO_PIN_14)
#define KEY_LED_LED_IOMUX                                        (IOMUX_PINCM31)
/* Defines for BUTTON: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_LED_BUTTON_PIN                                      (DL_GPIO_PIN_21)
#define KEY_LED_BUTTON_IOMUX                                     (IOMUX_PINCM49)
/* Port definition for Pin Group I2C_LCD_PINS */
#define I2C_LCD_PINS_PORT                                                (GPIOA)

/* Defines for LCD_SDA: GPIOA.0 with pinCMx 1 on package pin 33 */
#define I2C_LCD_PINS_LCD_SDA_PIN                                 (DL_GPIO_PIN_0)
#define I2C_LCD_PINS_LCD_SDA_IOMUX                                (IOMUX_PINCM1)
/* Defines for LCD_SCL: GPIOA.1 with pinCMx 2 on package pin 34 */
#define I2C_LCD_PINS_LCD_SCL_PIN                                 (DL_GPIO_PIN_1)
#define I2C_LCD_PINS_LCD_SCL_IOMUX                                (IOMUX_PINCM2)
/* Port definition for Pin Group BOARD_CONTROL */
#define BOARD_CONTROL_PORT                                               (GPIOA)

/* Defines for WIRELESS_POWER: GPIOA.13 with pinCMx 35 on package pin 6 */
#define BOARD_CONTROL_WIRELESS_POWER_PIN                        (DL_GPIO_PIN_13)
#define BOARD_CONTROL_WIRELESS_POWER_IOMUX                       (IOMUX_PINCM35)
/* Defines for ADC_MOS_SWITCH: GPIOA.14 with pinCMx 36 on package pin 7 */
#define BOARD_CONTROL_ADC_MOS_SWITCH_PIN                        (DL_GPIO_PIN_14)
#define BOARD_CONTROL_ADC_MOS_SWITCH_IOMUX                       (IOMUX_PINCM36)
/* Defines for ADDR0: GPIOA.27 with pinCMx 60 on package pin 31 */
#define ADDR_CODE_ADDR0_PORT                                             (GPIOA)
#define ADDR_CODE_ADDR0_PIN                                     (DL_GPIO_PIN_27)
#define ADDR_CODE_ADDR0_IOMUX                                    (IOMUX_PINCM60)
/* Defines for ADDR1: GPIOA.25 with pinCMx 55 on package pin 26 */
#define ADDR_CODE_ADDR1_PORT                                             (GPIOA)
#define ADDR_CODE_ADDR1_PIN                                     (DL_GPIO_PIN_25)
#define ADDR_CODE_ADDR1_IOMUX                                    (IOMUX_PINCM55)
/* Defines for ADDR2: GPIOB.24 with pinCMx 52 on package pin 23 */
#define ADDR_CODE_ADDR2_PORT                                             (GPIOB)
#define ADDR_CODE_ADDR2_PIN                                     (DL_GPIO_PIN_24)
#define ADDR_CODE_ADDR2_IOMUX                                    (IOMUX_PINCM52)
/* Defines for ADDR3: GPIOB.18 with pinCMx 44 on package pin 15 */
#define ADDR_CODE_ADDR3_PORT                                             (GPIOB)
#define ADDR_CODE_ADDR3_PIN                                     (DL_GPIO_PIN_18)
#define ADDR_CODE_ADDR3_IOMUX                                    (IOMUX_PINCM44)
/* Port definition for Pin Group DDS_CTRL */
#define DDS_CTRL_PORT                                                    (GPIOA)

/* Defines for DDS_WCLK: GPIOA.28 with pinCMx 3 on package pin 35 */
#define DDS_CTRL_DDS_WCLK_PIN                                   (DL_GPIO_PIN_28)
#define DDS_CTRL_DDS_WCLK_IOMUX                                   (IOMUX_PINCM3)
/* Defines for DDS_FQUD: GPIOA.29 with pinCMx 4 on package pin 36 */
#define DDS_CTRL_DDS_FQUD_PIN                                   (DL_GPIO_PIN_29)
#define DDS_CTRL_DDS_FQUD_IOMUX                                   (IOMUX_PINCM4)
/* Defines for DDS_DATA: GPIOA.30 with pinCMx 5 on package pin 37 */
#define DDS_CTRL_DDS_DATA_PIN                                   (DL_GPIO_PIN_30)
#define DDS_CTRL_DDS_DATA_IOMUX                                   (IOMUX_PINCM5)
/* Defines for DDS_RESET: GPIOA.24 with pinCMx 54 on package pin 25 */
#define DDS_CTRL_DDS_RESET_PIN                                  (DL_GPIO_PIN_24)
#define DDS_CTRL_DDS_RESET_IOMUX                                 (IOMUX_PINCM54)






/* Defines for DAC12 */
#define DAC12_IRQHandler                                         DAC0_IRQHandler
#define DAC12_INT_IRQN                                           (DAC0_INT_IRQn)
#define GPIO_DAC12_OUT_PORT                                                GPIOA
#define GPIO_DAC12_OUT_PIN                                        DL_GPIO_PIN_15
#define GPIO_DAC12_IOMUX_OUT                                     (IOMUX_PINCM37)
#define GPIO_DAC12_IOMUX_OUT_FUNC                   IOMUX_PINCM37_PF_UNCONNECTED

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SQUARE_TIMER_init(void);
void SYSCFG_DL_PWM_TIMER_init(void);
void SYSCFG_DL_FREQ_CAPTURE_init(void);
void SYSCFG_DL_ADC_SAMPLE_TIMER_init(void);
void SYSCFG_DL_PSU_CTRL_TIMER_init(void);
void SYSCFG_DL_LP_WAKE_TIMER_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_ADC12_0_init(void);
void SYSCFG_DL_ADC12_1_init(void);
void SYSCFG_DL_COMP_0_init(void);
void SYSCFG_DL_VREF_init(void);
void SYSCFG_DL_OPA_0_init(void);
void SYSCFG_DL_DMA_init(void);

void SYSCFG_DL_RTC_init(void);
void SYSCFG_DL_DAC12_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
