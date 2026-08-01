#include "psu_control.h"

#include "dac.h"
#include "pid_controller.h"
#include "ti_msp_dl_config.h"

static PsuConfig gPsuConfig;
static PsuState gPsuState;
static PID_Controller gPsuVPid;
static PID_Controller gPsuIPid;

static float Psu_Clamp(float value, float lo, float hi)
{
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

void Psu_Init(const PsuConfig *config)
{
    if (config == NULL) {
        return;
    }
    gPsuConfig = *config;

    PID_Init(&gPsuVPid, config->vKp, config->vKi, config->vKd,
        0.0f, (float) PSU_DAC_CODE_MAX);
    PID_Init(&gPsuIPid, config->iKp, config->iKi, config->iKd,
        0.0f, (float) PSU_DAC_CODE_MAX);

    gPsuState.running = false;
    gPsuState.tickCount = 0U;
    gPsuState.vMeasV = 0.0f;
    gPsuState.iMeasA = 0.0f;
    gPsuState.vRefV = config->vrefTargetV;
    gPsuState.iRefA = config->irefTargetA;
    gPsuState.vLoopOut = 0.0f;
    gPsuState.iLoopOut = 0.0f;
    gPsuState.controlOut = 0.0f;
    gPsuState.dacCode = 0U;
    gPsuState.inCvMode = false;
    gPsuState.inCcMode = false;
}

void Psu_Start(void)
{
    gPsuState.running = true;
    gPsuState.tickCount = 0U;
    NVIC_EnableIRQ(PSU_CTRL_TIMER_INST_INT_IRQN);
    DL_TimerG_startCounter(PSU_CTRL_TIMER_INST);
}

void Psu_Stop(void)
{
    DL_TimerG_stopCounter(PSU_CTRL_TIMER_INST);
    gPsuState.running = false;
    DAC_SetCode12(0U);
}

/*
 * Called from the main loop every control period with fresh ADC samples.
 * The ISR only flags a tick; the loops run here so ADC/DAC calls stay in
 * thread context and the ISR stays short.
 */
void Psu_Update(float measuredVoltageV, float measuredCurrentA)
{
    const PsuConfig *cfg = &gPsuConfig;
    float dt = 1.0f / (float) PSU_CTRL_TICK_HZ;
    float vError, iError;
    float vOut, iOut;
    float vRef, iRef;

    if (!gPsuState.running) {
        return;
    }

    gPsuState.vMeasV = measuredVoltageV;
    gPsuState.iMeasA = measuredCurrentA;

    /* Reference generation from curve mode. */
    vRef = cfg->vrefTargetV;
    iRef = cfg->irefTargetA;
    if (cfg->curveMode == CURVE_PV) {
        /* As load current rises toward Ishort, Vref falls toward zero. */
        if (cfg->irefTargetA > 0.0f) {
            float ratio = measuredCurrentA / cfg->irefTargetA;
            if (ratio < 0.0f) {
                ratio = 0.0f;
            }
            if (ratio > 1.0f) {
                ratio = 1.0f;
            }
            vRef = cfg->vrefTargetV * (1.0f - 0.85f * ratio);
        }
    }
    gPsuState.vRefV = vRef;
    gPsuState.iRefA = iRef;

    /* Voltage loop. */
    vError = vRef - measuredVoltageV;
    vOut = PID_Update(&gPsuVPid, vRef, measuredVoltageV, dt);
    (void) vError;
    gPsuState.vLoopOut = vOut;

    /* Current loop. */
    iError = iRef - measuredCurrentA;
    iOut = PID_Update(&gPsuIPid, iRef, measuredCurrentA, dt);
    (void) iError;
    gPsuState.iLoopOut = iOut;

    /* Winner-takes-all selection in DAC-code space. */
    gPsuState.inCvMode = (cfg->curveMode != CURVE_CC_ONLY);
    gPsuState.inCcMode = (cfg->curveMode != CURVE_CV_ONLY);
    if ((cfg->curveMode == CURVE_CV_ONLY) ||
        ((cfg->curveMode == CURVE_PV) && (vOut <= iOut))) {
        gPsuState.controlOut = Psu_Clamp(vOut, 0.0f,
            (float) PSU_DAC_CODE_MAX);
        gPsuState.inCcMode = false;
    } else {
        gPsuState.controlOut = Psu_Clamp(iOut, 0.0f,
            (float) PSU_DAC_CODE_MAX);
        gPsuState.inCvMode = false;
    }

    gPsuState.dacCode = (uint16_t) (gPsuState.controlOut + 0.5f);
    DAC_SetCode12(gPsuState.dacCode);
}

PsuState Psu_GetState(void)
{
    return gPsuState;
}

bool Psu_IsRunning(void)
{
    return gPsuState.running;
}

void PSU_CTRL_TIMER_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(PSU_CTRL_TIMER_INST)) {
        case DL_TIMERG_IIDX_ZERO:
            gPsuState.tickCount++;
            break;
        default:
            break;
    }
}
