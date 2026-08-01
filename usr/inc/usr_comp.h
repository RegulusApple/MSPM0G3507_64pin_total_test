#ifndef USR_COMP_H_
#define USR_COMP_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * COMP1 threshold comparator wrapper (IN0+ = PB26, IN0- = PB27).
 * Reference is the internal 8-bit DAC on VDDA; the threshold voltage is
 * set with 8-bit resolution over 0..VDDA.  The comparator output can be
 * polled; a rising/falling output edge can also wake the IRQ if enabled.
 */

#define USR_COMP_DAC_CODE_MAX   (255U)

void UsrComp_Init(void);
bool UsrComp_SetThresholdMv(uint16_t millivolt);  /* VDDA referenced */
uint16_t UsrComp_GetThresholdMv(void);
bool UsrComp_ReadOutput(void);   /* true = IN0+ above threshold */
void UsrComp_Enable(void);
void UsrComp_Disable(void);

#endif /* USR_COMP_H_ */
