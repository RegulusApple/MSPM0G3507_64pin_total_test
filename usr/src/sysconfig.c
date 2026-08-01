#include "sysconfig.h"

#include "board_config.h"
#include "system_time.h"

uint8_t ALLOCATE_DATA_BUFF[1024 * 6];

void Sysclk_Init(void)
{
    SYSCFG_DL_init();

#if (BOARD_SYSOSC_FCL_EXTERNAL_ROSC != 0U)
    DL_SYSCTL_enableSYSOSCFCLExternalResistor();
    SystemTime_DelayUs(BOARD_SYSOSC_FCL_SETTLE_US);
#endif
}

bool Sysclk_IsFclActive(void)
{
    return ((DL_SYSCTL_getClockStatus() & DL_SYSCTL_CLK_STATUS_FCL_ON) != 0U);
}
