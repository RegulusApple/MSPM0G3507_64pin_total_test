#ifndef DAC_H_
#define DAC_H_

#include <stdint.h>
#include <stdbool.h>
#include "ti_msp_dl_config.h"

#define DAC_CODE_MAX        (4095U)
#define DAC_OUTPUT_PIN_NAME "PA15/DAC_OUT"
#define DAC_SINE_DEFAULT_HZ (100000U)
#define DAC_SINE_FS_MAX_HZ  (1000000U)
#define DAC_SINE_TABLE_MAX  (1024U)
#define DAC_SINE_TABLE_MIN  (8U)

void DAC_Init(void);
void DAC_SetCode12(uint16_t code);
void DAC_SetVoltageMv(uint16_t millivolt);
void DAC_SetSineAmplitudeMv(uint16_t millivolt);
uint16_t DAC_GetCode12(void);
uint16_t DAC_GetVoltageMv(void);
const char *DAC_GetOutputPinName(void);
bool DAC_StartSineHz(uint32_t freqHz);
void DAC_StopSine(void);
uint32_t DAC_GetSineRequestedHz(void);
uint32_t DAC_GetSineActualHz(void);
uint32_t DAC_GetSineSampleRateHz(void);
uint16_t DAC_GetSineTableLen(void);
void DAC_PrintDebug(const char *tag);

/* Sine sweep helper: steps frequencyHz from startHz to stopHz (multiply by
 * stepRatio, e.g. 1.26 for ~1/3-octave steps), dwellMs per point.  Calls
 * callback(startHz, stopHz, steps, dwellMs) with the chosen parameters.
 * Returns the number of points actually run (0 on invalid input). */
typedef void (*DacSweepPointFn)(uint32_t frequencyHz, uint32_t pointIndex);
uint32_t DAC_SweepSine(uint32_t startHz, uint32_t stopHz, float stepRatio,
    uint32_t dwellMs, DacSweepPointFn pointFn);

#endif
