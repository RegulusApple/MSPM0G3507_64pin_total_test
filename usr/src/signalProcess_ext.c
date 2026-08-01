#include "signalProcess.h"

#include <stddef.h>

#define SP_TWO_PI                 (2.0f * PI)
#define SP_MIN_SIGNAL_SPAN        (1.0e-6f)
#define SP_MIN_DENOMINATOR        (1.0e-12f)

static float SP_MeanF32(const float *data, uint16_t length)
{
    uint16_t i;
    float sum = 0.0f;

    for (i = 0U; i < length; i++) {
        sum += data[i];
    }
    return sum / (float) length;
}

static float SP_MeanU16(const uint16_t *data, uint16_t length)
{
    uint16_t i;
    float sum = 0.0f;

    for (i = 0U; i < length; i++) {
        sum += (float) data[i];
    }
    return sum / (float) length;
}

static float SP_WrapPhaseDeg(float phaseDeg)
{
    while (phaseDeg > 180.0f) {
        phaseDeg -= 360.0f;
    }
    while (phaseDeg < -180.0f) {
        phaseDeg += 360.0f;
    }
    return phaseDeg;
}

static float SP_Median3(float a, float b, float c)
{
    if (a > b) {
        float temp = a;
        a = b;
        b = temp;
    }
    if (b > c) {
        float temp = b;
        b = c;
        c = temp;
    }
    if (a > b) {
        b = a;
    }
    return b;
}

bool SP_CalcBasicStatsU16(const uint16_t *data, uint16_t length,
    SP_BasicStats *result)
{
    uint16_t i;
    uint16_t minCode;
    uint16_t maxCode;
    float mean;
    float squareSum = 0.0f;
    float peakAbs = 0.0f;

    if ((data == NULL) || (result == NULL) || (length == 0U)) {
        return false;
    }

    minCode = data[0];
    maxCode = data[0];
    mean = SP_MeanU16(data, length);

    for (i = 0U; i < length; i++) {
        float ac = (float) data[i] - mean;
        float absAc = fabsf(ac);

        squareSum += ac * ac;
        if (absAc > peakAbs) {
            peakAbs = absAc;
        }
        if (data[i] < minCode) {
            minCode = data[i];
        }
        if (data[i] > maxCode) {
            maxCode = data[i];
        }
    }

    result->sampleCount = length;
    result->minCode = minCode;
    result->maxCode = maxCode;
    result->mean = mean;
    result->rmsAc = sqrtf(squareSum / (float) length);
    result->peakAbs = peakAbs;
    result->peakToPeak = (float) maxCode - (float) minCode;
    return true;
}

void SP_RemoveDC(float *data, uint16_t length)
{
    uint16_t i;
    float mean;

    if ((data == NULL) || (length == 0U)) {
        return;
    }

    mean = SP_MeanF32(data, length);
    for (i = 0U; i < length; i++) {
        data[i] -= mean;
    }
}

void SP_MovingAverage(float *data, uint16_t length, uint16_t window)
{
    uint16_t i;

    if ((data == NULL) || (length == 0U) || (window <= 1U)) {
        return;
    }
    if (window > length) {
        window = length;
    }

    /* Forward-looking implementation is intentionally in-place safe. */
    for (i = 0U; i < length; i++) {
        uint16_t j;
        uint16_t count = window;
        float sum = 0.0f;

        if (((uint32_t) i + count) > length) {
            count = (uint16_t) (length - i);
        }
        for (j = 0U; j < count; j++) {
            sum += data[i + j];
        }
        data[i] = sum / (float) count;
    }
}

void SP_MedianFilter3(float *data, uint16_t length)
{
    uint16_t i;
    float left;
    float center;

    if ((data == NULL) || (length < 3U)) {
        return;
    }

    left = data[0];
    center = data[1];
    for (i = 1U; i < (uint16_t) (length - 1U); i++) {
        float right = data[i + 1U];
        data[i] = SP_Median3(left, center, right);
        left = center;
        center = right;
    }
}

void SP_IIRLowPass(float *data, uint16_t length,
    float cutoffHz, float sampleRateHz)
{
    uint16_t i;
    float dt;
    float rc;
    float alpha;
    float output;

    if ((data == NULL) || (length < 2U) || (cutoffHz <= 0.0f) ||
        (sampleRateHz <= 0.0f) || (cutoffHz >= (sampleRateHz * 0.5f))) {
        return;
    }

    dt = 1.0f / sampleRateHz;
    rc = 1.0f / (SP_TWO_PI * cutoffHz);
    alpha = dt / (rc + dt);
    output = data[0];

    for (i = 1U; i < length; i++) {
        output += alpha * (data[i] - output);
        data[i] = output;
    }
}

void SP_IIRHighPass(float *data, uint16_t length,
    float cutoffHz, float sampleRateHz)
{
    uint16_t i;
    float dt;
    float rc;
    float alpha;
    float previousInput;
    float previousOutput = 0.0f;

    if ((data == NULL) || (length < 2U) || (cutoffHz <= 0.0f) ||
        (sampleRateHz <= 0.0f) || (cutoffHz >= (sampleRateHz * 0.5f))) {
        return;
    }

    dt = 1.0f / sampleRateHz;
    rc = 1.0f / (SP_TWO_PI * cutoffHz);
    alpha = rc / (rc + dt);
    previousInput = data[0];
    data[0] = 0.0f;

    for (i = 1U; i < length; i++) {
        float currentInput = data[i];
        float currentOutput = alpha *
            (previousOutput + currentInput - previousInput);

        data[i] = currentOutput;
        previousInput = currentInput;
        previousOutput = currentOutput;
    }
}

bool SP_EstimateFrequency(const float *data, uint16_t length,
    float sampleRateHz, float *frequencyHz)
{
    uint16_t i;
    uint16_t crossingCount = 0U;
    float mean;
    float minValue;
    float maxValue;
    float firstCrossing = 0.0f;
    float lastCrossing = 0.0f;

    if ((data == NULL) || (frequencyHz == NULL) || (length < 3U) ||
        (sampleRateHz <= 0.0f)) {
        return false;
    }

    mean = SP_MeanF32(data, length);
    minValue = data[0];
    maxValue = data[0];
    for (i = 1U; i < length; i++) {
        if (data[i] < minValue) {
            minValue = data[i];
        }
        if (data[i] > maxValue) {
            maxValue = data[i];
        }
    }
    if ((maxValue - minValue) < SP_MIN_SIGNAL_SPAN) {
        return false;
    }

    for (i = 1U; i < length; i++) {
        float previous = data[i - 1U] - mean;
        float current = data[i] - mean;

        if ((previous <= 0.0f) && (current > 0.0f)) {
            float denominator = current - previous;
            float fraction = 0.0f;
            float crossing;

            if (fabsf(denominator) > SP_MIN_DENOMINATOR) {
                fraction = -previous / denominator;
            }
            crossing = (float) (i - 1U) + fraction;
            if (crossingCount == 0U) {
                firstCrossing = crossing;
            }
            lastCrossing = crossing;
            crossingCount++;
        }
    }

    if ((crossingCount < 2U) || (lastCrossing <= firstCrossing)) {
        return false;
    }

    *frequencyHz = sampleRateHz * (float) (crossingCount - 1U) /
        (lastCrossing - firstCrossing);
    return true;
}

bool SP_EstimateFrequencyU16(const uint16_t *data, uint16_t length,
    float sampleRateHz, float *frequencyHz)
{
    uint16_t i;
    uint16_t crossingCount = 0U;
    uint16_t minValue;
    uint16_t maxValue;
    float mean;
    float firstCrossing = 0.0f;
    float lastCrossing = 0.0f;

    if ((data == NULL) || (frequencyHz == NULL) || (length < 3U) ||
        (sampleRateHz <= 0.0f)) {
        return false;
    }

    mean = SP_MeanU16(data, length);
    minValue = data[0];
    maxValue = data[0];
    for (i = 1U; i < length; i++) {
        if (data[i] < minValue) {
            minValue = data[i];
        }
        if (data[i] > maxValue) {
            maxValue = data[i];
        }
    }
    if (maxValue == minValue) {
        return false;
    }

    for (i = 1U; i < length; i++) {
        float previous = (float) data[i - 1U] - mean;
        float current = (float) data[i] - mean;

        if ((previous <= 0.0f) && (current > 0.0f)) {
            float denominator = current - previous;
            float fraction = 0.0f;
            float crossing;

            if (fabsf(denominator) > SP_MIN_DENOMINATOR) {
                fraction = -previous / denominator;
            }
            crossing = (float) (i - 1U) + fraction;
            if (crossingCount == 0U) {
                firstCrossing = crossing;
            }
            lastCrossing = crossing;
            crossingCount++;
        }
    }

    if ((crossingCount < 2U) || (lastCrossing <= firstCrossing)) {
        return false;
    }

    *frequencyHz = sampleRateHz * (float) (crossingCount - 1U) /
        (lastCrossing - firstCrossing);
    return true;
}

static bool SP_PhaseFromF32(const float *channelA, const float *channelB,
    uint16_t length, float frequencyHz, float sampleRateHz, float *phaseDeg)
{
    uint16_t i;
    float meanA;
    float meanB;
    float realA = 0.0f;
    float imagA = 0.0f;
    float realB = 0.0f;
    float imagB = 0.0f;
    float omega;

    if ((channelA == NULL) || (channelB == NULL) || (phaseDeg == NULL) ||
        (length < 2U) || (sampleRateHz <= 0.0f) || (frequencyHz <= 0.0f) ||
        (frequencyHz >= (sampleRateHz * 0.5f))) {
        return false;
    }

    meanA = SP_MeanF32(channelA, length);
    meanB = SP_MeanF32(channelB, length);
    omega = SP_TWO_PI * frequencyHz / sampleRateHz;

    for (i = 0U; i < length; i++) {
        float angle = omega * (float) i;
        float cosine = cosf(angle);
        float sine = sinf(angle);
        float a = channelA[i] - meanA;
        float b = channelB[i] - meanB;

        realA += a * cosine;
        imagA -= a * sine;
        realB += b * cosine;
        imagB -= b * sine;
    }

    if ((((realA * realA) + (imagA * imagA)) < SP_MIN_DENOMINATOR) ||
        (((realB * realB) + (imagB * imagB)) < SP_MIN_DENOMINATOR)) {
        return false;
    }

    *phaseDeg = SP_WrapPhaseDeg(
        (atan2f(imagB, realB) - atan2f(imagA, realA)) * 180.0f / PI);
    return true;
}

bool SP_EstimatePhase(const float *channelA, const float *channelB,
    uint16_t length, float frequencyHz, float sampleRateHz, float *phaseDeg)
{
    return SP_PhaseFromF32(channelA, channelB, length,
        frequencyHz, sampleRateHz, phaseDeg);
}

bool SP_EstimatePhaseU16(const uint16_t *channelA, const uint16_t *channelB,
    uint16_t length, float frequencyHz, float sampleRateHz, float *phaseDeg)
{
    uint16_t i;
    float meanA;
    float meanB;
    float realA = 0.0f;
    float imagA = 0.0f;
    float realB = 0.0f;
    float imagB = 0.0f;
    float omega;

    if ((channelA == NULL) || (channelB == NULL) || (phaseDeg == NULL) ||
        (length < 2U) || (sampleRateHz <= 0.0f) || (frequencyHz <= 0.0f) ||
        (frequencyHz >= (sampleRateHz * 0.5f))) {
        return false;
    }

    meanA = SP_MeanU16(channelA, length);
    meanB = SP_MeanU16(channelB, length);
    omega = SP_TWO_PI * frequencyHz / sampleRateHz;

    for (i = 0U; i < length; i++) {
        float angle = omega * (float) i;
        float cosine = cosf(angle);
        float sine = sinf(angle);
        float a = (float) channelA[i] - meanA;
        float b = (float) channelB[i] - meanB;

        realA += a * cosine;
        imagA -= a * sine;
        realB += b * cosine;
        imagB -= b * sine;
    }

    if ((((realA * realA) + (imagA * imagA)) < SP_MIN_DENOMINATOR) ||
        (((realB * realB) + (imagB * imagB)) < SP_MIN_DENOMINATOR)) {
        return false;
    }

    *phaseDeg = SP_WrapPhaseDeg(
        (atan2f(imagB, realB) - atan2f(imagA, realA)) * 180.0f / PI);
    return true;
}

float SP_GoertzelMagnitude(const float *data, uint16_t length,
    float sampleRateHz, float targetFrequencyHz)
{
    uint16_t i;
    float mean;
    float omega;
    float coefficient;
    float state1 = 0.0f;
    float state2 = 0.0f;
    float power;

    if ((data == NULL) || (length < 2U) || (sampleRateHz <= 0.0f) ||
        (targetFrequencyHz <= 0.0f) ||
        (targetFrequencyHz >= (sampleRateHz * 0.5f))) {
        return 0.0f;
    }

    mean = SP_MeanF32(data, length);
    omega = SP_TWO_PI * targetFrequencyHz / sampleRateHz;
    coefficient = 2.0f * cosf(omega);

    for (i = 0U; i < length; i++) {
        float state0 = (data[i] - mean) +
            (coefficient * state1) - state2;
        state2 = state1;
        state1 = state0;
    }

    power = (state1 * state1) + (state2 * state2) -
        (coefficient * state1 * state2);
    if (power <= 0.0f) {
        return 0.0f;
    }
    return (2.0f * sqrtf(power)) / (float) length;
}

float SP_GoertzelMagnitudeU16(const uint16_t *data, uint16_t length,
    float sampleRateHz, float targetFrequencyHz)
{
    uint16_t i;
    float mean;
    float omega;
    float coefficient;
    float state1 = 0.0f;
    float state2 = 0.0f;
    float power;

    if ((data == NULL) || (length < 2U) || (sampleRateHz <= 0.0f) ||
        (targetFrequencyHz <= 0.0f) ||
        (targetFrequencyHz >= (sampleRateHz * 0.5f))) {
        return 0.0f;
    }

    mean = SP_MeanU16(data, length);
    omega = SP_TWO_PI * targetFrequencyHz / sampleRateHz;
    coefficient = 2.0f * cosf(omega);

    for (i = 0U; i < length; i++) {
        float state0 = ((float) data[i] - mean) +
            (coefficient * state1) - state2;
        state2 = state1;
        state1 = state0;
    }

    power = (state1 * state1) + (state2 * state2) -
        (coefficient * state1 * state2);
    if (power <= 0.0f) {
        return 0.0f;
    }
    return (2.0f * sqrtf(power)) / (float) length;
}
