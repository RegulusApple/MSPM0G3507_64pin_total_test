#include "usr_opa.h"

#include "ti_msp_dl_config.h"

#define USR_OPA_INST (OPA_0_INST)

static bool gUsrOpaEnabled;

void UsrOpa_Init(void)
{
    gUsrOpaEnabled = true;
}

bool UsrOpa_SetGain(UsrOpaGain gain)
{
    DL_OPA_GAIN dlGain;

    switch (gain) {
        case USR_OPA_GAIN_2X:
            dlGain = DL_OPA_GAIN_N1_P2;
            break;
        case USR_OPA_GAIN_4X:
            dlGain = DL_OPA_GAIN_N3_P4;
            break;
        case USR_OPA_GAIN_8X:
            dlGain = DL_OPA_GAIN_N7_P8;
            break;
        case USR_OPA_GAIN_16X:
            dlGain = DL_OPA_GAIN_N15_P16;
            break;
        case USR_OPA_GAIN_32X:
            dlGain = DL_OPA_GAIN_N31_P32;
            break;
        default:
            return false;
    }
    DL_OPA_setGain(USR_OPA_INST, dlGain);
    return true;
}

UsrOpaGain UsrOpa_GetGain(void)
{
    DL_OPA_GAIN dlGain = DL_OPA_getGain(USR_OPA_INST);

    switch (dlGain) {
        case DL_OPA_GAIN_N3_P4:
            return USR_OPA_GAIN_4X;
        case DL_OPA_GAIN_N7_P8:
            return USR_OPA_GAIN_8X;
        case DL_OPA_GAIN_N15_P16:
            return USR_OPA_GAIN_16X;
        case DL_OPA_GAIN_N31_P32:
            return USR_OPA_GAIN_32X;
        default:
            return USR_OPA_GAIN_2X;
    }
}

void UsrOpa_Enable(void)
{
    DL_OPA_enable(USR_OPA_INST);
    gUsrOpaEnabled = true;
}

void UsrOpa_Disable(void)
{
    DL_OPA_disable(USR_OPA_INST);
    gUsrOpaEnabled = false;
}

bool UsrOpa_IsEnabled(void)
{
    return gUsrOpaEnabled;
}
