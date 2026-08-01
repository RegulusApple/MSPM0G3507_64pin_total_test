#include "analog_measure.h"

#include <math.h>
#include <stddef.h>

#include "adc2chSample.h"
#include "signalProcess.h"

#define ANALOG_ADC_CODE_MAX       (4095.0f)
#define ANALOG_CLIP_LOW_CODE      (8U)
#define ANALOG_CLIP_HIGH_CODE     (4087U)
#define ANALOG_THD_HARMONIC_MAX   (5U)
#define ANALOG_MIN_FUNDAMENTAL    (1.0e-6f)

static bool Analog_IsChannelValid(uint8_t channel)
{
    return (channel == ADC12_CHANNEL_0) || (channel == ADC12_CHANNEL_1);
}

static uint16_t *Analog_GetValidSamples(uint8_t channel)
{
    uint16_t *buffer = (channel == ADC12_CHANNEL_1) ?
        pADC_Buffer1 : pADC_Buffer0;

    return &buffer[ADC_DISCARD_LEN];
}

AnalogStatus Analog_AnalyzeSamples(const uint16_t *samples,
    uint16_t sampleCount, float sampleRateHz, float fullScaleMv,
    float frontendGain, AnalogResult *result)
{
    SP_BasicStats stats;
    float scaleMvPerCode;

    if ((samples == NULL) || (result == NULL) || (sampleCount == 0U) ||
        (sampleRateHz <= 0.0f) || (fullScaleMv <= 0.0f) ||
        (frontendGain <= 0.0f)) {
        return ANALOG_ERR_PARAM;
    }

    if (SP_CalcBasicStatsU16(samples, sampleCount, &stats) == false) {
        return ANALOG_ERR_PARAM;
    }

    scaleMvPerCode = fullScaleMv /
        (ANALOG_ADC_CODE_MAX * frontendGain);
    result->sampleCount = sampleCount;
    result->minCode = stats.minCode;
    result->maxCode = stats.maxCode;
    result->dcMv = stats.mean * scaleMvPerCode;
    result->rmsMv = stats.rmsAc * scaleMvPerCode;
    result->peakMv = stats.peakAbs * scaleMvPerCode;
    result->peakToPeakMv = stats.peakToPeak * scaleMvPerCode;
    result->frequencyHz = 0.0f;
    result->phaseDeg = 0.0f;
    result->thdPercent = 0.0f;
    result->frequencyValid = SP_EstimateFrequencyU16(samples, sampleCount,
        sampleRateHz, &result->frequencyHz);
    result->phaseValid = false;
    result->thdValid = false;
    result->clipped = (stats.minCode <= ANALOG_CLIP_LOW_CODE) ||
        (stats.maxCode >= ANALOG_CLIP_HIGH_CODE);

    if (result->frequencyValid) {
        float fundamental = SP_GoertzelMagnitudeU16(samples, sampleCount,
            sampleRateHz, result->frequencyHz);

        if (fundamental > ANALOG_MIN_FUNDAMENTAL) {
            uint8_t harmonic;
            float harmonicSquareSum = 0.0f;

            for (harmonic = 2U; harmonic <= ANALOG_THD_HARMONIC_MAX;
                harmonic++) {
                float harmonicHz = result->frequencyHz * (float) harmonic;
                float magnitude;

                if (harmonicHz >= (sampleRateHz * 0.5f)) {
                    break;
                }
                magnitude = SP_GoertzelMagnitudeU16(samples, sampleCount,
                    sampleRateHz, harmonicHz);
                harmonicSquareSum += magnitude * magnitude;
            }
            result->thdPercent =
                100.0f * sqrtf(harmonicSquareSum) / fundamental;
            result->thdValid = true;
        }
    }

    if (result->clipped) {
        return ANALOG_ERR_CLIPPED;
    }
    return ANALOG_OK;
}

AnalogStatus Analog_Capture(uint8_t channel, uint32_t sampleRateHz,
    uint16_t sampleCount)
{
    bool captured;

    if ((Analog_IsChannelValid(channel) == false) ||
        (sampleCount != ADC_VALID_LEN) || (sampleRateHz == 0U)) {
        return ANALOG_ERR_PARAM;
    }

    ADC12_SetSampleRateHz(sampleRateHz);
    captured = (channel == ADC12_CHANNEL_0) ?
        ADC12_SampleADC0() : ADC12_SampleADC1();
    return captured ? ANALOG_OK : ANALOG_ERR_TIMEOUT;
}

AnalogStatus Analog_CaptureDual(uint32_t sampleRateHz, uint16_t sampleCount)
{
    if ((sampleCount != ADC_VALID_LEN) || (sampleRateHz == 0U)) {
        return ANALOG_ERR_PARAM;
    }

    ADC12_SetSampleRateHz(sampleRateHz);
    return ADC12_SampleADC0ADC1() ? ANALOG_OK : ANALOG_ERR_TIMEOUT;
}

AnalogStatus Analog_Analyze(uint8_t channel, AnalogResult *result)
{
    if ((Analog_IsChannelValid(channel) == false) || (result == NULL)) {
        return ANALOG_ERR_PARAM;
    }

    return Analog_AnalyzeSamples(Analog_GetValidSamples(channel),
        ADC_VALID_LEN, (float) ADC12_GetSampleRateHz(),
        ADC12_GetReferenceVoltage() * 1000.0f,
        ADC12_GetFrontendGain(), result);
}

AnalogStatus Analog_AnalyzeDual(AnalogResult *channel0,
    AnalogResult *channel1, float *phaseDeg)
{
    AnalogStatus status0;
    AnalogStatus status1;
    float frequencyHz;

    if ((channel0 == NULL) || (channel1 == NULL) || (phaseDeg == NULL)) {
        return ANALOG_ERR_PARAM;
    }

    status0 = Analog_Analyze(ADC12_CHANNEL_0, channel0);
    status1 = Analog_Analyze(ADC12_CHANNEL_1, channel1);
    if ((status0 == ANALOG_ERR_PARAM) || (status1 == ANALOG_ERR_PARAM)) {
        return ANALOG_ERR_PARAM;
    }
    if ((status0 == ANALOG_ERR_CLIPPED) ||
        (status1 == ANALOG_ERR_CLIPPED)) {
        return ANALOG_ERR_CLIPPED;
    }

    if (channel0->frequencyValid) {
        frequencyHz = channel0->frequencyHz;
    } else if (channel1->frequencyValid) {
        frequencyHz = channel1->frequencyHz;
    } else {
        return ANALOG_ERR_NO_SIGNAL;
    }

    if (SP_EstimatePhaseU16(Analog_GetValidSamples(ADC12_CHANNEL_0),
            Analog_GetValidSamples(ADC12_CHANNEL_1), ADC_VALID_LEN,
            frequencyHz, (float) ADC12_GetSampleRateHz(), phaseDeg) == false) {
        return ANALOG_ERR_NO_SIGNAL;
    }

    channel0->phaseDeg = *phaseDeg;
    channel1->phaseDeg = *phaseDeg;
    channel0->phaseValid = true;
    channel1->phaseValid = true;
    return ANALOG_OK;
}

AnalogStatus Analog_CaptureAndAnalyze(uint8_t channel,
    uint32_t sampleRateHz, AnalogResult *result)
{
    AnalogStatus status = Analog_Capture(channel, sampleRateHz,
        ADC_VALID_LEN);

    if (status != ANALOG_OK) {
        return status;
    }
    return Analog_Analyze(channel, result);
}
