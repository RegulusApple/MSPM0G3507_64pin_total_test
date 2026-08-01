#ifndef PWM_OUT_H_
#define PWM_OUT_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * General PWM output on TIMA0 CCP1 (PA9).
 * Frequency = PWM_TIMER_INST_CLK_FREQ / timerCount.
 * Duty is set as a percentage 0..100 (or as raw CC value).
 */

void PwmOut_Init(void);
bool PwmOut_SetFreqHz(uint32_t frequencyHz);
uint32_t PwmOut_GetFreqHz(void);
bool PwmOut_SetDutyPercent(float percent);   /* 0.0 .. 100.0 */
float PwmOut_GetDutyPercent(void);
void PwmOut_Start(void);
void PwmOut_Stop(void);
bool PwmOut_IsRunning(void);

/* D-class audio helper: map a signed sample to PWM duty. */
float PwmOut_SampleToDuty(int16_t sample, uint16_t fullScale);

#endif /* PWM_OUT_H_ */
