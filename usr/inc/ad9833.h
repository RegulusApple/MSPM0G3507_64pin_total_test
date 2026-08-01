#ifndef AD9833_H
#define AD9833_H

#include <stdbool.h>
#include <stdint.h>

#define SINE_WAVE     (0U)
#define TRIANGLE_WAVE (1U)
#define SQUARE_WAVE   (2U)

bool AD9833_IsAvailable(void);
bool AD9833_Init(void);
bool AD9833_WaveOut(uint8_t mode, uint32_t frequencyHz, uint16_t phaseDeg);

#endif
