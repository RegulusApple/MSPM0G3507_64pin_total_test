#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define KEY_1 (0x01U)
#define KEY_2 (0x02U)
#define KEY_3 (0x04U)
#define KEY_4 (0x08U)
#define KEY_5 (0x10U)

#define KEY_ACTIVE_LOW (1U)

#define KEY_TRIGGER_ONCE   (0U)
#define KEY_TRIGGER_REPEAT (1U)

void Key_Init(void);
uint8_t Key_ReadRaw(uint8_t repeat);
uint8_t Key_ReadDebounced(uint8_t repeat);

#endif
