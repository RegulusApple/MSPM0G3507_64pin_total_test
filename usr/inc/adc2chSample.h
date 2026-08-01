#ifndef ADC2CHSAMPLE_H_
#define ADC2CHSAMPLE_H_

#include <stdbool.h>
#include <stdint.h>
#include "sysconfig.h"

#define ADC_SAMPLE_RATE_MAX_HZ      (4000000U)
#define ADC_SAMPLE_RATE_DEFAULT_HZ  (12800U)
#define ADC_SAMPLE_RATE             ((float) ADC_SAMPLE_RATE_DEFAULT_HZ)
#define ADC_BUFFER_LEN              (1074U)
#define ADC_DISCARD_LEN             (50U)
#define ADC_VALID_LEN               (1024U)
#define ADC_POINTS_PER_50HZ_CYCLE   (256U)
#define ADC_WINDOW_CYCLES_50HZ      (4U)
#define ADC_WINDOW_TIME_MS          (80U)
#define ADC_FRONTEND_GAIN_DEFAULT   (5.56f)
#define ADC_FRONTEND_GAIN           ADC_FRONTEND_GAIN_DEFAULT
#define CHANNEL_SIZE                (2U)

#define ADC12_CHANNEL_0             (0U)
#define ADC12_CHANNEL_1             (1U)
#define ADC12_CHANNEL_SIZE_1        (1U)
#define ADC12_CHANNEL_SIZE_2        (2U)

extern uint16_t *pADC_Buffer0;
extern uint16_t *pADC_Buffer1;
extern volatile bool ADC_Finished;

typedef struct {
    uint16_t sampleIndex;
    uint8_t adcId;
    uint8_t stage;
    uint8_t dma0Done;
    uint8_t dma1Done;
    uint8_t timerRunning;
    uint32_t timerCount;
    uint32_t rawInterruptStatus;
    uint32_t control1;
} ADC12_DebugInfo;

typedef struct {
    uint16_t sampleCount;
    float dcCode;
    float rmsCode;
    float rmsMv;
} ADC12_RmsResult;

void ADC12_SetSampleRateHz(uint32_t sampleRateHz);
uint32_t ADC12_GetSampleRateHz(void);
uint16_t ADC12_GetSampleTimerPeriod(void);
uint8_t ADC12_GetSampleTimerPrescaler(void);
void ADC12_SetFrontendGain(float gain);
float ADC12_GetFrontendGain(void);
ADC12_DebugInfo ADC12_GetDebugInfo(void);

void ADC12_Init(void);
bool ADC12_SampleADC0(void);
bool ADC12_SampleADC1(void);
bool ADC12_SampleADC0ADC1(void);
void ADC12_UpFrame(uint8_t channelSize, uint8_t channel);
bool ADC12_CalcRms(uint8_t channel, ADC12_RmsResult *result);
bool ADC12_IsClipped(uint8_t channel);
bool ADC12_CheckSampleRate(float *actualRateHz);
bool ADC12_CalcMean(uint8_t channel, float *meanCode);
bool ADC12_CalcPeakToPeak(uint8_t channel, float *peakToPeakMv);
bool ADC12_CalcFrequency(uint8_t channel, float *frequencyHz);
bool ADC12_CalcPhase(uint8_t channel0, uint8_t channel1, float *phaseDeg);
void ADC12_FFTSHOW(uint8_t channel);

/* High-speed single-frame capture helper: temporarily switches the sample
 * timer to rateHz, captures one frame on the given channel(s), restores the
 * previous rate, and returns the valid-sample window.  Useful for verifying
 * elevated ADC rates (e.g. 100 kHz..1 Msps) without changing the default
 * 12.8 kHz configuration. */
bool ADC12_CaptureFrameHighRate(uint8_t channel, uint32_t rateHz,
    uint16_t *outValidCount);

float ADC12_GetReferenceVoltage(void);
void ADC12_PrintDebug(const char *tag);

#endif




