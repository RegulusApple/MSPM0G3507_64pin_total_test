#ifndef WAVE_GEN_H_
#define WAVE_GEN_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_DC
} WaveType;

typedef enum {
    WAVE_SOURCE_DAC = 0,
    WAVE_SOURCE_AD9833,
    WAVE_SOURCE_AD9850
} WaveSource;

typedef struct {
    WaveSource source;
    WaveType type;
    float frequencyHz;
    float amplitudeMv;
    float offsetMv;
    float phaseDeg;
} WaveConfig;

bool WaveGen_Start(const WaveConfig *config);
bool WaveGen_SetFrequency(float frequencyHz);
bool WaveGen_SetAmplitude(float amplitudeMv);
bool WaveGen_SetOffset(float offsetMv);
void WaveGen_Stop(void);
bool WaveGen_IsRunning(void);
bool WaveGen_GetConfig(WaveConfig *config);

#endif /* WAVE_GEN_H_ */
