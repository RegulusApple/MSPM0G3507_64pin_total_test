#ifndef AD9833_H
#define AD9833_H

#include <stdint.h>

#define SINE_WAVE     (0U)
#define TRIANGLE_WAVE (1U)
#define SQUARE_WAVE   (2U)

void AD9833_Init(void);
void AD9833_Send(uint16_t data);
void AD9833_WaveOut(uint8_t mode, uint32_t frequencyHz, uint16_t phaseDeg);

#endif
