#include "usr_comp.h"

#include "ti_msp_dl_config.h"

#define USR_COMP_INST (COMP_0_INST)
#define USR_COMP_VDDA_MV (3300U)

static uint16_t gUsrCompThresholdMv;

void UsrComp_Init(void)
{
    gUsrCompThresholdMv = USR_COMP_VDDA_MV / 2U;
    (void) UsrComp_SetThresholdMv(gUsrCompThresholdMv);
}

bool UsrComp_SetThresholdMv(uint16_t millivolt)
{
    uint32_t code;
    uint32_t vdda;

    if (millivolt > USR_COMP_VDDA_MV) {
        return false;
    }
    vdda = USR_COMP_VDDA_MV;
    code = ((uint32_t) millivolt * (USR_COMP_DAC_CODE_MAX + 1U) +
        (vdda / 2U)) / vdda;
    if (code > USR_COMP_DAC_CODE_MAX) {
        code = USR_COMP_DAC_CODE_MAX;
    }
    DL_COMP_setDACCode0(USR_COMP_INST, code);
    gUsrCompThresholdMv = millivolt;
    return true;
}

uint16_t UsrComp_GetThresholdMv(void)
{
    return gUsrCompThresholdMv;
}

bool UsrComp_ReadOutput(void)
{
    return (DL_COMP_getComparatorOutput(USR_COMP_INST) == DL_COMP_OUTPUT_HIGH);
}

void UsrComp_Enable(void)
{
    DL_COMP_enable(USR_COMP_INST);
}

void UsrComp_Disable(void)
{
    DL_COMP_disable(USR_COMP_INST);
}
