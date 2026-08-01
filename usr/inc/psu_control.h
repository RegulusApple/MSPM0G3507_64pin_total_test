#ifndef PSU_CONTROL_H_
#define PSU_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * CV/CC dual-loop power-supply control (e.g. solar-array simulator).
 *
 * TIMG6 runs a 20 kHz control tick (PSU_CTRL_TIMER).  Every tick the
 * application feeds the sampled output voltage and current via
 * Psu_Update(); the library runs the voltage loop and the current loop,
 * selects the lower control output (standard CV/CC "winner takes all"),
 * and writes the resulting code to the DAC (PA15).
 *
 * The DAC output is a 12-bit code; the power stage maps it to the actual
 * output voltage/current.  A V(I) curve table lets the reference follow a
 * photovoltaic-style I-V characteristic.
 */

#define PSU_CTRL_TICK_HZ        (20000U)
#define PSU_DAC_CODE_MAX        (4095U)

typedef enum {
    CURVE_CV_ONLY = 0,   /* constant voltage only, current loop disabled */
    CURVE_CC_ONLY,       /* constant current only, voltage loop disabled */
    CURVE_PV             /* photovoltaic curve: Vref drops as I rises */
} PsuCurveMode;

typedef struct {
    float vrefTargetV;       /* open-circuit voltage target (V) */
    float irefTargetA;       /* short-circuit current target (A) */
    PsuCurveMode curveMode;
    /* CV loop gains */
    float vKp;
    float vKi;
    float vKd;
    /* CC loop gains */
    float iKp;
    float iKi;
    float iKd;
    /* DAC mapping: dacCode = base + k * controlValue */
    float dacBase;
    float dacVoltPerCode;    /* volts per DAC code at the power stage */
    float dacAmpPerCode;     /* amperes per DAC code at the power stage */
} PsuConfig;

typedef struct {
    volatile bool running;
    volatile uint32_t tickCount;   /* written by ISR, read by main loop */
    float vMeasV;
    float iMeasA;
    float vRefV;
    float iRefA;
    float vLoopOut;
    float iLoopOut;
    float controlOut;        /* selected (min) loop output in code space */
    uint16_t dacCode;
    bool inCvMode;
    bool inCcMode;
} PsuState;

void Psu_Init(const PsuConfig *config);
void Psu_Start(void);
void Psu_Stop(void);
void Psu_Update(float measuredVoltageV, float measuredCurrentA);
PsuState Psu_GetState(void);
bool Psu_IsRunning(void);

/* ISR entry; do not define your own TIMG6_IRQHandler when using this lib. */
void PSU_CTRL_TIMER_INST_IRQHandler(void);

#endif /* PSU_CONTROL_H_ */
