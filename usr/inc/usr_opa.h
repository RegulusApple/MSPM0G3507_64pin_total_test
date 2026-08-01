#ifndef USR_OPA_H_
#define USR_OPA_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * OPA1 non-inverting PGA wrapper (IN0+ = PB19, OUT = PA16).
 * Configured by SysConfig as 2x non-inverting PGA; this library lets the
 * application change gain at runtime and read the output directly.
 * The OPA output is also available on ADC1 channel 13 internally.
 */

typedef enum {
    USR_OPA_GAIN_2X = 0,   /* N1_P2  (non-inverting 2x) */
    USR_OPA_GAIN_4X,       /* N3_P4  */
    USR_OPA_GAIN_8X,       /* N7_P8  */
    USR_OPA_GAIN_16X,      /* N15_P16 */
    USR_OPA_GAIN_32X       /* N31_P32 */
} UsrOpaGain;

void UsrOpa_Init(void);
bool UsrOpa_SetGain(UsrOpaGain gain);
UsrOpaGain UsrOpa_GetGain(void);
void UsrOpa_Enable(void);
void UsrOpa_Disable(void);
bool UsrOpa_IsEnabled(void);

#endif /* USR_OPA_H_ */
