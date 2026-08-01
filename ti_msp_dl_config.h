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
#define CONFIG_MSPM0G3507

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



#define CPUCLK_FREQ                                                      8000000



/* Defines for PWM_0 */
#define PWM_0_INST                                                         TIMA0
#define PWM_0_INST_IRQHandler                                   TIMA0_IRQHandler
#define PWM_0_INST_INT_IRQN                                     (TIMA0_INT_IRQn)
#define PWM_0_INST_CLK_FREQ                                              8000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_0_C0_PORT                                                 GPIOA
#define GPIO_PWM_0_C0_PIN                                          DL_GPIO_PIN_0
#define GPIO_PWM_0_C0_IOMUX                                       (IOMUX_PINCM1)
#define GPIO_PWM_0_C0_IOMUX_FUNC                      IOMUX_PINCM1_PF_TIMA0_CCP0
#define GPIO_PWM_0_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_0_C1_PORT                                                 GPIOA
#define GPIO_PWM_0_C1_PIN                                          DL_GPIO_PIN_1
#define GPIO_PWM_0_C1_IOMUX                                       (IOMUX_PINCM2)
#define GPIO_PWM_0_C1_IOMUX_FUNC                      IOMUX_PINCM2_PF_TIMA0_CCP1
#define GPIO_PWM_0_C1_IDX                                    DL_TIMER_CC_1_INDEX

/* GPIO probe for ADC sample timer ZERO interrupt. PA0 toggles once per sample trigger. */
#define ADC_SAMPLE_PROBE_PORT                                             GPIOA
#define ADC_SAMPLE_PROBE_PIN                                      DL_GPIO_PIN_0
#define ADC_SAMPLE_PROBE_IOMUX                                   (IOMUX_PINCM1)
/* PA1 is no longer used as PWM output; keep it as a quiet GPIO low. */
#define ADC_SAMPLE_PROBE_UNUSED_PORT                                      GPIOA
#define ADC_SAMPLE_PROBE_UNUSED_PIN                               DL_GPIO_PIN_1
#define ADC_SAMPLE_PROBE_UNUSED_IOMUX                            (IOMUX_PINCM2)



/* Defines for ADC_SAMPLE_TIMER */
#define ADC_SAMPLE_TIMER_INST                                            (TIMG0)
#define ADC_SAMPLE_TIMER_INST_IRQHandler                        TIMG0_IRQHandler
#define ADC_SAMPLE_TIMER_INST_INT_IRQN                          (TIMG0_INT_IRQn)
#define ADC_SAMPLE_TIMER_INST_LOAD_VALUE                                  (630U)
#define ADC_SAMPLE_TIMER_INST_PUB_0_CH                                       (1)
#define ADC_SAMPLE_TIMER_INST_PUB_1_CH                                       (2)
#define LP_WAKE_TIMER_INST                                              (TIMG8)
#define LP_WAKE_TIMER_INST_IRQHandler                          TIMG8_IRQHandler
#define LP_WAKE_TIMER_INST_INT_IRQN                            (TIMG8_INT_IRQn)
#define LP_WAKE_TIMER_LFCLK_HZ                                            (32768U)
#define LP_WAKE_TIMER_PERIOD_TICKS                                        (32767U)

/* Defines for ADC1_STAGGER_TIMER (TIMG6) — phase-delayed clone of ADC_SAMPLE_TIMER */
#define ADC1_STAGGER_TIMER_INST                                         (TIMG6)
#define ADC1_STAGGER_TIMER_INST_LOAD_VALUE                    ADC_SAMPLE_TIMER_INST_LOAD_VALUE
#define ADC1_STAGGER_TIMER_INST_PUB_0_CH                                     (2)
#define ADC_STAGGER_DELAY_TICKS                                               (7U)
/* 12.8kHz sample timing calibrated for measured ~8.08MHz BUSCLK: 8080000 / 12800 ~= 631 ticks */
#define ADC_SAMPLE_PROBE_PWM_PERIOD                                         (625U)
#define ADC_SAMPLE_PROBE_PWM_COMPARE                                        (312U)
/* Defines for DAC_TIMER */
#define DAC_TIMER_INST                                                   (TIMA1)
#define DAC_TIMER_INST_IRQHandler                               TIMA1_IRQHandler
#define DAC_TIMER_INST_INT_IRQN                                 (TIMA1_INT_IRQn)
#define DAC_TIMER_INST_LOAD_VALUE                                           (0U)



/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_FREQUENCY                                        8000000
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
#define UART_DEBUG_IBRD_8_MHZ_115200_BAUD                                    (4)
#define UART_DEBUG_FBRD_8_MHZ_115200_BAUD                                   (22)




/* Defines for AD9833_SPI */
#define AD9833_SPI_INST                                                    SPI0
#define AD9833_SPI_INST_IRQHandler                              SPI0_IRQHandler
#define AD9833_SPI_INST_INT_IRQN                                  SPI0_INT_IRQn
#define GPIO_AD9833_SPI_PICO_PORT                                         GPIOA
#define GPIO_AD9833_SPI_PICO_PIN                                 DL_GPIO_PIN_14
#define GPIO_AD9833_SPI_IOMUX_PICO                              (IOMUX_PINCM36)
#define GPIO_AD9833_SPI_IOMUX_PICO_FUNC              IOMUX_PINCM36_PF_SPI0_PICO

/* PA14 controls the external MOS switch for ADC sampling power/path. */
#define ADC_MOS_SWITCH_PORT                                               GPIOA
#define ADC_MOS_SWITCH_PIN                                        DL_GPIO_PIN_14
#define ADC_MOS_SWITCH_IOMUX                                     (IOMUX_PINCM36)
#define GPIO_AD9833_SPI_POCI_PORT                                         GPIOA
#define GPIO_AD9833_SPI_POCI_PIN                                  DL_GPIO_PIN_4
#define GPIO_AD9833_SPI_IOMUX_POCI                               (IOMUX_PINCM9)
#define GPIO_AD9833_SPI_IOMUX_POCI_FUNC               IOMUX_PINCM9_PF_SPI0_POCI
/* GPIO configuration for AD9833_SPI */
#define GPIO_AD9833_SPI_SCLK_PORT                                         GPIOA
#define GPIO_AD9833_SPI_SCLK_PIN                                 DL_GPIO_PIN_12
#define GPIO_AD9833_SPI_IOMUX_SCLK                              (IOMUX_PINCM34)
#define GPIO_AD9833_SPI_IOMUX_SCLK_FUNC              IOMUX_PINCM34_PF_SPI0_SCLK
#define GPIO_AD9833_SPI_CS0_PORT                                          GPIOA
#define GPIO_AD9833_SPI_CS0_PIN                                   DL_GPIO_PIN_2
#define GPIO_AD9833_SPI_IOMUX_CS0                                (IOMUX_PINCM7)
#define GPIO_AD9833_SPI_IOMUX_CS0_FUNC                 IOMUX_PINCM7_PF_SPI0_CS0



/* Defines for ADC12_0 */
#define ADC12_0_INST                                                        ADC0
#define ADC12_0_INST_IRQHandler                                  ADC0_IRQHandler
#define ADC12_0_INST_INT_IRQN                                    (ADC0_INT_IRQn)
#define ADC12_0_ADCMEM_0                                      DL_ADC12_MEM_IDX_0
#define ADC12_0_ADCMEM_0_REF                   DL_ADC12_REFERENCE_VOLTAGE_INTREF
#define ADC12_0_ADCMEM_0_REF_VOLTAGE_V                                       1.4
#define ADC12_0_INST_SUB_CH                                                  (1)
#define GPIO_ADC12_0_C0_PORT                                               GPIOA
#define GPIO_ADC12_0_C0_PIN                                       DL_GPIO_PIN_22
#define GPIO_ADC12_0_C0_IOMUX                                     (IOMUX_PINCM47)

/* Defines for ADC12_1 */
#define ADC12_1_INST                                                        ADC1
#define ADC12_1_INST_IRQHandler                                  ADC1_IRQHandler
#define ADC12_1_INST_INT_IRQN                                    (ADC1_INT_IRQn)
#define ADC12_1_ADCMEM_0                                      DL_ADC12_MEM_IDX_0
#define ADC12_1_ADCMEM_0_REF                   DL_ADC12_REFERENCE_VOLTAGE_INTREF
#define ADC12_1_ADCMEM_0_REF_VOLTAGE_V                                       1.4
#define ADC12_1_INST_SUB_CH                                                  (2)
#define GPIO_ADC12_1_C1_PORT                                               GPIOA
#define GPIO_ADC12_1_C1_PIN                                       DL_GPIO_PIN_21
#define GPIO_ADC12_1_C1_IOMUX                                     (IOMUX_PINCM46)



/* Defines for VREF output pin */
#define VREF_VOLTAGE_MV                                                     (1400U)
#define GPIO_VREF_VREFPOS_PORT                                             GPIOA
#define GPIO_VREF_VREFPOS_PIN                                      DL_GPIO_PIN_23
#define GPIO_VREF_IOMUX_VREFPOS                                    (IOMUX_PINCM53)
#define GPIO_VREF_IOMUX_VREFPOS_FUNC                    IOMUX_PINCM53_PF_UNCONNECTED



/* Defines for address code inputs: pulldown, active high */
#define ADDR_CODE_ADDR0_PORT                                               GPIOA
#define ADDR_CODE_ADDR0_PIN                                        DL_GPIO_PIN_27
#define ADDR_CODE_ADDR0_IOMUX                                      (IOMUX_PINCM60)
#define ADDR_CODE_ADDR0_BIT                                               (0x01U)

#define ADDR_CODE_ADDR1_PORT                                               GPIOA
#define ADDR_CODE_ADDR1_PIN                                        DL_GPIO_PIN_25
#define ADDR_CODE_ADDR1_IOMUX                                      (IOMUX_PINCM55)
#define ADDR_CODE_ADDR1_BIT                                               (0x02U)

#define ADDR_CODE_ADDR2_PORT                                               GPIOB
#define ADDR_CODE_ADDR2_PIN                                        DL_GPIO_PIN_24
#define ADDR_CODE_ADDR2_IOMUX                                      (IOMUX_PINCM52)
#define ADDR_CODE_ADDR2_BIT                                               (0x04U)

#define ADDR_CODE_ADDR3_PORT                                               GPIOB
#define ADDR_CODE_ADDR3_PIN                                        DL_GPIO_PIN_18
#define ADDR_CODE_ADDR3_IOMUX                                      (IOMUX_PINCM44)
#define ADDR_CODE_ADDR3_BIT                                               (0x08U)



/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (0)
#define ADC12_0_INST_DMA_TRIGGER                      (DMA_ADC0_EVT_GEN_BD_TRIG)
/* Defines for DMA_CH1 */
#define DMA_CH1_CHAN_ID                                                      (1)
#define ADC12_1_INST_DMA_TRIGGER                      (DMA_ADC1_EVT_GEN_BD_TRIG)
/* Defines for DAC */
#define DAC_CHAN_ID                                                          (2)
#define DAC_TRIGGER_SEL_SW                                   (DMA_SOFTWARE_TRIG)


/* Port definition for Pin Group AD9833_FSYNC */
#define AD9833_FSYNC_PORT                                                (GPIOA)

/* Defines for FSYNC: GPIOA.13 with pinCMx 35 on package pin 6 */
#define AD9833_FSYNC_FSYNC_PIN                                  (DL_GPIO_PIN_13)
#define AD9833_FSYNC_FSYNC_IOMUX                                 (IOMUX_PINCM35)

/* Port definitions for Pin Group KEY */
#define KEY_PORTA                                                        (GPIOA)
#define KEY_PORTB                                                        (GPIOB)

/* Defines for KEY1: GPIOA.8 with pinCMx 19 on package pin 16 */
#define KEY_KEY1_PORT                                                    (GPIOA)
#define KEY_KEY1_PIN                                             (DL_GPIO_PIN_8)
#define KEY_KEY1_IOMUX                                           (IOMUX_PINCM19)
/* Defines for KEY2: GPIOB.3 with pinCMx 16 on package pin 15 */
#define KEY_KEY2_PORT                                                    (GPIOB)
#define KEY_KEY2_PIN                                             (DL_GPIO_PIN_3)
#define KEY_KEY2_IOMUX                                           (IOMUX_PINCM16)
#define KEY_KEY2_EDGE_POLARITY                              DL_GPIO_PIN_3_EDGE_FALL
#define KEY_KEY2_WAKEUP_MODE                                  DL_GPIO_WAKEUP_ON_0
#define KEY_KEY2_INT_IRQN                                           GPIOB_INT_IRQn
/* Defines for KEY3: GPIOB.2 with pinCMx 15 on package pin 14 */
#define KEY_KEY3_PORT                                                    (GPIOB)
#define KEY_KEY3_PIN                                             (DL_GPIO_PIN_2)
#define KEY_KEY3_IOMUX                                           (IOMUX_PINCM15)
/* Defines for KEY4: GPIOA.7 with pinCMx 14 on package pin 13 */
#define KEY_KEY4_PORT                                                    (GPIOA)
#define KEY_KEY4_PIN                                             (DL_GPIO_PIN_7)
#define KEY_KEY4_IOMUX                                           (IOMUX_PINCM14)
/* Defines for KEY5: GPIOA.31 with pinCMx 6 on package pin 5 */
#define KEY_KEY5_PORT                                                    (GPIOA)
#define KEY_KEY5_PIN                                            (DL_GPIO_PIN_31)
#define KEY_KEY5_IOMUX                                            (IOMUX_PINCM6)



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
void SYSCFG_DL_PWM_0_init(void);
void SYSCFG_DL_ADC_SAMPLE_TIMER_init(void);
void SYSCFG_DL_ADC1_STAGGER_TIMER_init(void);
void SYSCFG_DL_DAC_TIMER_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_AD9833_SPI_init(void);
void SYSCFG_DL_VREF_init(void);
void SYSCFG_DL_ADC12_0_init(void);
void SYSCFG_DL_ADC12_1_init(void);
void SYSCFG_DL_DMA_init(void);

void SYSCFG_DL_DAC12_init(void);

bool Sysclk_WaitForSYSOSCBase(uint32_t timeoutLoops);
bool Sysclk_WaitForLFOSCGood(uint32_t timeoutLoops);

/* Compatibility aliases for the existing application modules. */
#define ADC_SAMPLE_TIMER_DEFAULT_LOAD_VALUE ADC_SAMPLE_TIMER_INST_LOAD_VALUE
#define ADC_SAMPLE_TIMER_DEFAULT_PRESCALE (0U)
#define UART_0_INST UART_DEBUG_INST
#define UART_0_INST_IRQHandler UART_DEBUG_INST_IRQHandler
#define UART_0_INST_INT_IRQN UART_DEBUG_INST_INT_IRQN
#define KEY1_PIN KEY_KEY1_PIN
#define KEY2_PIN KEY_KEY2_PIN
#define KEY3_PIN KEY_KEY3_PIN
#define KEY4_PIN KEY_KEY4_PIN
#define KEY5_PIN KEY_KEY5_PIN
#define KEY_PORT KEY_PORTB
#define KEY_ALL_PORTA_PINS (KEY1_PIN | KEY4_PIN | KEY5_PIN)
#define KEY_ALL_PORTB_PINS (KEY2_PIN | KEY3_PIN)
#define KEY_ALL_PINS KEY_ALL_PORTB_PINS
#define SYSCFG_DL_KEY_init() ((void)0)
#define DAC12_INST DAC0
#define DAC12_REF_MV (3300U)
#define DAC12_DEFAULT_CODE (2048U)
#define DMA_DAC_CHAN_ID DAC_CHAN_ID
#define AD9833_FSYNC_PIN AD9833_FSYNC_FSYNC_PIN

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
