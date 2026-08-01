#ifndef _SIGNALPROCESS_H
#define _SIGNALPROCESS_H

/////////////////////////////////////////////////////////////

// STD
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

// DEFINE
#define FFT_LEN     1024U
#define PI          3.141592653f
#define SQRT2       1.414213562373095f

typedef struct {
    uint16_t sampleCount;
    uint16_t minCode;
    uint16_t maxCode;
    float mean;
    float rmsAc;
    float peakAbs;
    float peakToPeak;
} SP_BasicStats;

/* Initialize the fixed-size FFT tables. */
void SP_myFFTInit(void);

/* Execute FFT and harmonic post-processing. */
void SP_myFFT(uint16_t *regBuff, float *pFFTIOData);
void SP_myFFT_CalcMagAngle(float* pFFTResIn, uint16_t* pFFTFreqPointSelect, uint16_t numPointSelect, float adcVref, float* pFFTMagOut, float* pFFTAngleOut);
void SP_AngleCompensate50HzHarmonic(float* pFFTAngleIn, float* pFFTAngleOut, float compensateDelayS, uint16_t numHarmonic);

/* Calculate power-domain quantities from harmonic magnitudes and phases. */
float SP_CalcSumSqrt(float* pMagInput, uint16_t num);
float SP_CalcTHD(float* pMagBaseHarmonic, uint16_t numHarmonic);
void SP_CalcPowerS(float* pCurrentMagBaseHarmonic, float* pVoltageMagBaseHarmonic, uint16_t numHarmonic, float* currentRms, float* voltageRms, float* powerS);
float SP_CalcPowerP(float* pCurrentMagBaseHarmonic, float* pVoltageMagBaseHarmonic, float* pCurrentAngleBaseHarmonic, float* pVoltageAngleBaseHarmonic, uint16_t numHarmonic);

/* Contest-oriented, allocation-free signal analysis helpers. */
bool SP_CalcBasicStatsU16(const uint16_t *data, uint16_t length,
    SP_BasicStats *result);
void SP_RemoveDC(float *data, uint16_t length);
void SP_MovingAverage(float *data, uint16_t length, uint16_t window);
void SP_MedianFilter3(float *data, uint16_t length);
void SP_IIRLowPass(float *data, uint16_t length,
    float cutoffHz, float sampleRateHz);
void SP_IIRHighPass(float *data, uint16_t length,
    float cutoffHz, float sampleRateHz);
bool SP_EstimateFrequency(const float *data, uint16_t length,
    float sampleRateHz, float *frequencyHz);
bool SP_EstimateFrequencyU16(const uint16_t *data, uint16_t length,
    float sampleRateHz, float *frequencyHz);
bool SP_EstimatePhase(const float *channelA, const float *channelB,
    uint16_t length, float frequencyHz, float sampleRateHz,
    float *phaseDeg);
bool SP_EstimatePhaseU16(const uint16_t *channelA, const uint16_t *channelB,
    uint16_t length, float frequencyHz, float sampleRateHz,
    float *phaseDeg);
float SP_GoertzelMagnitude(const float *data, uint16_t length,
    float sampleRateHz, float targetFrequencyHz);
float SP_GoertzelMagnitudeU16(const uint16_t *data, uint16_t length,
    float sampleRateHz, float targetFrequencyHz);

#endif
