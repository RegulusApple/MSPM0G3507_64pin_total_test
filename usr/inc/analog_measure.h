#ifndef ANALOG_MEASURE_H_
#define ANALOG_MEASURE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ANALOG_OK = 0,
    ANALOG_ERR_PARAM,
    ANALOG_ERR_TIMEOUT,
    ANALOG_ERR_NO_SIGNAL,
    ANALOG_ERR_CLIPPED
} AnalogStatus;

typedef struct {
    uint16_t sampleCount;
    uint16_t minCode;
    uint16_t maxCode;
    float dcMv;
    float rmsMv;
    float peakMv;
    float peakToPeakMv;
    float frequencyHz;
    float phaseDeg;
    float thdPercent;
    bool frequencyValid;
    bool phaseValid;
    bool thdValid;
    bool clipped;
} AnalogResult;

AnalogStatus Analog_Capture(uint8_t channel, uint32_t sampleRateHz,
    uint16_t sampleCount);
AnalogStatus Analog_CaptureDual(uint32_t sampleRateHz, uint16_t sampleCount);
AnalogStatus Analog_Analyze(uint8_t channel, AnalogResult *result);
AnalogStatus Analog_AnalyzeDual(AnalogResult *channel0,
    AnalogResult *channel1, float *phaseDeg);
AnalogStatus Analog_CaptureAndAnalyze(uint8_t channel,
    uint32_t sampleRateHz, AnalogResult *result);
AnalogStatus Analog_AnalyzeSamples(const uint16_t *samples,
    uint16_t sampleCount, float sampleRateHz, float fullScaleMv,
    float frontendGain, AnalogResult *result);

#endif /* ANALOG_MEASURE_H_ */
