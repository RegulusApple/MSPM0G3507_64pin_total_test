#ifndef ANALOG_CALIBRATION_H_
#define ANALOG_CALIBRATION_H_

#include <stdbool.h>
#include <stdint.h>

#define CALIBRATION_CHANNEL_COUNT  (2U)
#define CALIBRATION_FORMAT_VERSION (1U)

typedef struct {
    float offset;
    float gain;
    uint32_t version;
    uint32_t crc32;
} AnalogCalibration;

void Calibration_ResetDefault(void);
bool Calibration_SetZero(uint8_t channel, float measuredZero);
bool Calibration_TwoPoint(uint8_t channel,
    float actualLow, float measuredLow,
    float actualHigh, float measuredHigh);
float Calibration_Apply(uint8_t channel, float measuredValue);
bool Calibration_Get(uint8_t channel, AnalogCalibration *calibration);
bool Calibration_Set(uint8_t channel,
    const AnalogCalibration *calibration);
uint32_t Calibration_Crc32(const void *data, uint32_t length);

#endif /* ANALOG_CALIBRATION_H_ */
