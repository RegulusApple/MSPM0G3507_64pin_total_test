#ifndef _SIGNALPROCESS_H
#define _SIGNALPROCESS_H

/////////////////////////////////////////////////////////////

// STD
#include <math.h>
#include <stdint.h>

// USER
#include "adc2chSample.h"
#include "uart_debug.h"

// DEFINE
#define FFT_LEN     2048U
#define PI          3.141592653f
#define SQRT2       1.414213562373095f

// FFT ��ʼ��
void SP_myFFTInit(void);

// ִ�� FFT
void SP_myFFT(uint16_t *regBuff, float *pFFTIOData);
void SP_myFFT_CalcMagAngle(float* pFFTResIn, uint16_t* pFFTFreqPointSelect, uint16_t numPointSelect, float adcVref, float* pFFTMagOut, float* pFFTAngleOut);
void SP_AngleCompensate50HzHarmonic(float* pFFTAngleIn, float* pFFTAngleOut, float compensateDelayS, uint16_t numHarmonic);

// ���㹦�ʲ���
float SP_CalcSumSqrt(float* pMagInput, uint16_t num);
float SP_CalcTHD(float* pMagBaseHarmonic, uint16_t numHarmonic);
void SP_CalcPowerS(float* pCurrentMagBaseHarmonic, float* pVoltageMagBaseHarmonic, uint16_t numHarmonic, float* currentRms, float* voltageRms, float* powerS);
float SP_CalcPowerP(float* pCurrentMagBaseHarmonic, float* pVoltageMagBaseHarmonic, float* pCurrentAngleBaseHarmonic, float* pVoltageAngleBaseHarmonic, uint16_t numHarmonic);

#endif
