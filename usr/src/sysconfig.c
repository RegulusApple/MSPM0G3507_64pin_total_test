#include "sysconfig.h"

uint8_t ALLOCATE_DATA_BUFF[1024 * 6];

void Sysclk_Init(void)
{
    SYSCFG_DL_init();
}
