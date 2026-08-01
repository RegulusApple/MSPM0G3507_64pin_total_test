#include "ad9850.h"

#include <stddef.h>

#include "ti_msp_dl_config.h"
#include "system_time.h"

/*
 * Software-serial bit-bang for the AD9850 40-bit load word.
 * Word layout (LSB first on the wire):
 *   bits  0..31 : frequency tuning word (FTW), LSB first
 *   bit  32     : control bit (0)
 *   bits 33..37 : phase word, LSB first (5 bits)
 *   bits 38..39 : control bits (00)
 *
 * Timing: W_CLK toggles for each of the 40 bits; after the last bit an
 * FQ_UD rising edge latches the word into the DDS.
 */

#define AD9850_WORD_BITS              (40U)
#define AD9850_PHASE_SHIFT            (33U)

/* Minimum half-period for W_CLK; keep comfortably slow for bit-bang. */
#define AD9850_CLK_HALF_PERIOD_US     (1U)
#define AD9850_SETTLE_US              (1U)

#define AD9850_DDS_PORT               DDS_CTRL_PORT
#define AD9850_WCLK_PIN               DDS_CTRL_DDS_WCLK_PIN
#define AD9850_FQUD_PIN               DDS_CTRL_DDS_FQUD_PIN
#define AD9850_DATA_PIN               DDS_CTRL_DDS_DATA_PIN
#define AD9850_RESET_PIN              DDS_CTRL_DDS_RESET_PIN

static void Ad9850_SetPin(uint32_t pin, bool high)
{
    if (high) {
        DL_GPIO_setPins(AD9850_DDS_PORT, pin);
    } else {
        DL_GPIO_clearPins(AD9850_DDS_PORT, pin);
    }
}

static void Ad9850_ClockPulse(void)
{
    Ad9850_SetPin(AD9850_WCLK_PIN, true);
    SystemTime_DelayUs(AD9850_CLK_HALF_PERIOD_US);
    Ad9850_SetPin(AD9850_WCLK_PIN, false);
    SystemTime_DelayUs(AD9850_CLK_HALF_PERIOD_US);
}

bool AD9850_Init(void)
{
    /* Pins are outputs from SysConfig (DDS_CTRL group, cleared default). */
    Ad9850_SetPin(AD9850_FQUD_PIN, false);
    Ad9850_SetPin(AD9850_WCLK_PIN, false);
    Ad9850_SetPin(AD9850_DATA_PIN, false);

    return AD9850_Reset();
}

bool AD9850_Reset(void)
{
    /* Reset pulse: high for a few W_CLK periods, then low. */
    Ad9850_SetPin(AD9850_RESET_PIN, true);
    SystemTime_DelayUs(AD9850_CLK_HALF_PERIOD_US * 4U);
    Ad9850_SetPin(AD9850_RESET_PIN, false);
    SystemTime_DelayUs(AD9850_CLK_HALF_PERIOD_US);
    return true;
}

bool AD9850_PowerDown(void)
{
    uint64_t word;
    uint8_t bit;

    /* Word with control bit 32 set (power-down) and everything else zero. */
    word = UINT64_C(1) << 32U;

    Ad9850_SetPin(AD9850_FQUD_PIN, false);
    SystemTime_DelayUs(AD9850_SETTLE_US);

    for (bit = 0U; bit < AD9850_WORD_BITS; bit++) {
        Ad9850_SetPin(AD9850_DATA_PIN, (word & UINT64_C(1)) != 0U);
        Ad9850_ClockPulse();
        word >>= 1U;
    }

    Ad9850_SetPin(AD9850_FQUD_PIN, true);
    SystemTime_DelayUs(AD9850_SETTLE_US);
    Ad9850_SetPin(AD9850_FQUD_PIN, false);
    return true;
}

bool AD9850_WriteFtwPhase(uint32_t freqTuningWord, uint8_t phaseWord5)
{
    uint64_t word;
    uint8_t bit;

    if (phaseWord5 > AD9850_PHASE_MAX) {
        return false;
    }

    /*
     * Build the 40-bit word.  FTW occupies bits 0..31, phase bits 33..37,
     * control bits 32/38/39 are zero.
     */
    word = ((uint64_t) freqTuningWord) |
           (((uint64_t) phaseWord5) << AD9850_PHASE_SHIFT);

    /* Latch-in preamble: FQ_UD low before loading. */
    Ad9850_SetPin(AD9850_FQUD_PIN, false);
    SystemTime_DelayUs(AD9850_SETTLE_US);

    /* Shift out LSB first, W_CLK rising edge per bit. */
    for (bit = 0U; bit < AD9850_WORD_BITS; bit++) {
        Ad9850_SetPin(AD9850_DATA_PIN, (word & UINT64_C(1)) != 0U);
        Ad9850_ClockPulse();
        word >>= 1U;
    }

    /* FQ_UD rising edge latches the word into the DDS. */
    Ad9850_SetPin(AD9850_FQUD_PIN, true);
    SystemTime_DelayUs(AD9850_SETTLE_US);
    Ad9850_SetPin(AD9850_FQUD_PIN, false);
    return true;
}

bool AD9850_SetFrequencyHz(uint32_t frequencyHz)
{
    uint32_t ftw;
    uint64_t scaled;

    if ((frequencyHz < AD9850_FREQ_MIN_HZ) ||
        (frequencyHz > AD9850_FREQ_MAX_HZ)) {
        return false;
    }

    /* FTW = round(freq * 2^32 / REF_CLK) */
    scaled = ((uint64_t) frequencyHz << 32U) + (AD9850_REF_CLK_HZ / 2U);
    ftw = (uint32_t) (scaled / AD9850_REF_CLK_HZ);

    return AD9850_WriteFtwPhase(ftw, 0U);
}

bool AD9850_SetFrequencyPhaseHz(uint32_t frequencyHz, uint16_t phaseDeg)
{
    uint32_t ftw;
    uint64_t scaled;
    uint8_t phaseWord;

    if ((frequencyHz < AD9850_FREQ_MIN_HZ) ||
        (frequencyHz > AD9850_FREQ_MAX_HZ) || (phaseDeg > 360U)) {
        return false;
    }

    scaled = ((uint64_t) frequencyHz << 32U) + (AD9850_REF_CLK_HZ / 2U);
    ftw = (uint32_t) (scaled / AD9850_REF_CLK_HZ);

    /* 5-bit phase: 360 deg -> 32 counts. */
    phaseWord = (uint8_t) ((((uint32_t) phaseDeg * (AD9850_PHASE_MAX + 1U)) +
        (360U / 2U)) / 360U);
    if (phaseWord > AD9850_PHASE_MAX) {
        phaseWord = AD9850_PHASE_MAX;
    }

    return AD9850_WriteFtwPhase(ftw, phaseWord);
}
