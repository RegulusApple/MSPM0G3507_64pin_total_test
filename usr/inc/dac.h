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

#endif
