#ifndef AD9850_H
#define AD9850_H

#include <stdbool.h>
#include <stdint.h>

/*
 * AD9850 software-serial DDS driver (dropped-module friendly).
 *
 * The AD9850 is programmed through a 40-bit word shifted in LSB-first on the
 * W_CLK rising edge, then latched by an FQ_UD rising edge.  The module is
 * usually an "AD9850 DDS signal generator" board with its own 125 MHz
 * reference oscillator; only four control lines (W_CLK / FQ_UD / DATA /
 * RESET) plus 3V3 and GND need to be wired.  This driver uses SysConfig
 * GPIOs from the DDS_CTRL pin group and does NOT need a hardware SPI.
 *
 * Wiring (MSP-LITO-G3507 default, see .syscfg DDS_CTRL group):
 *   PA28 -> W_CLK    (module serial clock)
 *   PA29 -> FQ_UD    (module frequency update)
 *   PA30 -> DATA     (module serial data, also labelled D7 / DATA on board)
 *   PA24 -> RESET    (module reset, active high pulse)
 *
 * Outputs on the module: IOUT / IOUTB (sine, current output - add a load
 * resistor or op-amp) and QOUT / QOUTB (square wave via the internal
 * comparator; VINP/VINN are the comparator inputs).
 *
 * Reference clock: most AD9850 modules use 125 MHz.  If the dropped module
 * instead has a different reference (e.g. AD9851 with 6x multiplier), adjust
 * AD9850_REF_CLK_HZ accordingly; frequency accuracy depends on this value.
 */

/* Reference clock of the DDS system (module oscillator), in Hz. */
#define AD9850_REF_CLK_HZ              (125000000UL)

/* Minimum / maximum useful output frequency (sine). */
#define AD9850_FREQ_MIN_HZ             (1UL)
#define AD9850_FREQ_MAX_HZ             (AD9850_REF_CLK_HZ / 2UL)

/* Phase word is 5 bits on AD9850. */
#define AD9850_PHASE_MAX               (31U)

bool AD9850_Init(void);
bool AD9850_Reset(void);

/* Put the DDS into power-down (control bit set).  Output goes high-Z. */
bool AD9850_PowerDown(void);

/* Set sine frequency in Hz.  Returns false on out-of-range input. */
bool AD9850_SetFrequencyHz(uint32_t frequencyHz);

/* Set sine frequency (Hz) and phase (0..360 degrees). */
bool AD9850_SetFrequencyPhaseHz(uint32_t frequencyHz, uint16_t phaseDeg);

/* Low-level: write a full 40-bit word (32-bit FTW + 8 control/phase bits).
 * freqTuningWord is the raw 32-bit frequency tuning word. */
bool AD9850_WriteFtwPhase(uint32_t freqTuningWord, uint8_t phaseWord5);

#endif /* AD9850_H */
