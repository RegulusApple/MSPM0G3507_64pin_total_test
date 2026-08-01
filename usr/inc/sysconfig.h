#ifndef _SYSCONFIG_H
#define _SYSCONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"

extern uint8_t ALLOCATE_DATA_BUFF[1024 * 6];

void Sysclk_Init(void);
bool Sysclk_IsFclActive(void);

#endif
