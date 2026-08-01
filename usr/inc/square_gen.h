#ifndef SQUARE_GEN_H_
#define SQUARE_GEN_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Square-wave generator on TIMG7 (PWM edge mode, 50% duty) output PB15.
 * Frequency = SQUARE_TIMER_INST_CLK_FREQ / timerCount.
 * 16-bit timer: min 2 ticks => max ~16 MHz; practical range 1 Hz .. ~1 MHz.
 */

void SquareGen_Init(void);
bool SquareGen_SetFreqHz(uint32_t frequencyHz);
uint32_t SquareGen_GetFreqHz(void);
void SquareGen_Start(void);
void SquareGen_Stop(void);
bool SquareGen_IsRunning(void);

#endif /* SQUARE_GEN_H_ */
