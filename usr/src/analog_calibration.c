#include "analog_calibration.h"

#include <stddef.h>

#define CALIBRATION_MIN_DELTA (1.0e-6f)

static AnalogCalibration gCalibration[CALIBRATION_CHANNEL_COUNT];
static bool gCalibrationInitialized;

uint32_t Calibration_Crc32(const void *data, uint32_t length)
{
    const uint8_t *bytes = (const uint8_t *) data;
    uint32_t crc = 0xFFFFFFFFU;
    uint32_t i;

    if ((data == NULL) && (length != 0U)) {
        return 0U;
    }

    for (i = 0U; i < length; i++) {
        uint8_t bit;
        crc ^= bytes[i];
        for (bit = 0U; bit < 8U; bit++) {
            uint32_t mask = (uint32_t) (-(int32_t) (crc & 1U));
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

static uint32_t Calibration_RecordCrc(const AnalogCalibration *calibration)
{
    return Calibration_Crc32(calibration,
        (uint32_t) offsetof(AnalogCalibration, crc32));
}

static void Calibration_EnsureInitialized(void)
{
    if (gCalibrationInitialized == false) {
        Calibration_ResetDefault();
    }
}

void Calibration_ResetDefault(void)
{
    uint8_t channel;

    for (channel = 0U; channel < CALIBRATION_CHANNEL_COUNT; channel++) {
        gCalibration[channel].offset = 0.0f;
        gCalibration[channel].gain = 1.0f;
        gCalibration[channel].version = CALIBRATION_FORMAT_VERSION;
        gCalibration[channel].crc32 =
            Calibration_RecordCrc(&gCalibration[channel]);
    }
    gCalibrationInitialized = true;
}

bool Calibration_SetZero(uint8_t channel, float measuredZero)
{
    if (channel >= CALIBRATION_CHANNEL_COUNT) {
        return false;
    }

    Calibration_EnsureInitialized();
    gCalibration[channel].offset =
        -gCalibration[channel].gain * measuredZero;
    gCalibration[channel].crc32 =
        Calibration_RecordCrc(&gCalibration[channel]);
    return true;
}

bool Calibration_TwoPoint(uint8_t channel,
    float actualLow, float measuredLow,
    float actualHigh, float measuredHigh)
{
    float measuredDelta;
    float gain;
    float offset;

    if (channel >= CALIBRATION_CHANNEL_COUNT) {
        return false;
    }

    measuredDelta = measuredHigh - measuredLow;
    if ((measuredDelta > -CALIBRATION_MIN_DELTA) &&
        (measuredDelta < CALIBRATION_MIN_DELTA)) {
        return false;
    }

    gain = (actualHigh - actualLow) / measuredDelta;
    offset = actualLow - (gain * measuredLow);
    if (((gain > -CALIBRATION_MIN_DELTA) &&
            (gain < CALIBRATION_MIN_DELTA)) ||
        (gain > 1000.0f) || (gain < -1000.0f)) {
        return false;
    }

    Calibration_EnsureInitialized();
    gCalibration[channel].gain = gain;
    gCalibration[channel].offset = offset;
    gCalibration[channel].crc32 =
        Calibration_RecordCrc(&gCalibration[channel]);
    return true;
}

float Calibration_Apply(uint8_t channel, float measuredValue)
{
    if (channel >= CALIBRATION_CHANNEL_COUNT) {
        return measuredValue;
    }

    Calibration_EnsureInitialized();
    return (measuredValue * gCalibration[channel].gain) +
        gCalibration[channel].offset;
}

bool Calibration_Get(uint8_t channel, AnalogCalibration *calibration)
{
    if ((channel >= CALIBRATION_CHANNEL_COUNT) || (calibration == NULL)) {
        return false;
    }

    Calibration_EnsureInitialized();
    *calibration = gCalibration[channel];
    return true;
}

bool Calibration_Set(uint8_t channel,
    const AnalogCalibration *calibration)
{
    if ((channel >= CALIBRATION_CHANNEL_COUNT) || (calibration == NULL) ||
        (calibration->version != CALIBRATION_FORMAT_VERSION) ||
        (((calibration->gain > -CALIBRATION_MIN_DELTA) &&
            (calibration->gain < CALIBRATION_MIN_DELTA)) ||
            (calibration->gain > 1000.0f) ||
            (calibration->gain < -1000.0f)) ||
        (calibration->crc32 != Calibration_RecordCrc(calibration))) {
        return false;
    }

    Calibration_EnsureInitialized();
    gCalibration[channel] = *calibration;
    return true;
}
