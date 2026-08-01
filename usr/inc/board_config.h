#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include "ti_msp_dl_config.h"

/*
 * Application-owned board semantics.  SysConfig owns the physical pins and
 * peripherals; this header only gives those generated resources domain names.
 */
#define BOARD_SYSOSC_FCL_EXTERNAL_ROSC      (1U)
#define BOARD_SYSOSC_FCL_SETTLE_US          (30U)

#define BOARD_WIRELESS_POWER_PORT           BOARD_CONTROL_PORT
#define BOARD_WIRELESS_POWER_PIN            BOARD_CONTROL_WIRELESS_POWER_PIN
#define BOARD_ADC_MOS_SWITCH_PORT           BOARD_CONTROL_PORT
#define BOARD_ADC_MOS_SWITCH_PIN            BOARD_CONTROL_ADC_MOS_SWITCH_PIN

/* PA13 and PA14 are occupied above in the active measurement-board profile. */
#define BOARD_AD9833_AVAILABLE              (0U)

/*
 * External AD9850 DDS (dropped module) support.  Software-serial via the
 * SysConfig DDS_CTRL group (PA28 W_CLK / PA29 FQ_UD / PA30 DATA / PA24 RESET).
 * Set to 1 to enable the AD9850 path in wave_gen; the module needs 3V3/GND
 * plus the four control lines.
 */
#define BOARD_AD9850_AVAILABLE              (1U)

#define BOARD_DAC_INST                      (DAC0)
#define BOARD_DAC_REFERENCE_MV              (3300U)
#define BOARD_DAC_DEFAULT_CODE              (2048U)

#endif
