#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdbool.h>
#include "ti_msp_dl_config.h"

#define KEY_1 (0x01U)
#define KEY_2 (0x02U)
#define KEY_3 (0x04U)
#define KEY_4 (0x08U)
#define KEY_5 (0x10U)

#define KEY_ACTIVE_LOW (1U)

#define KEY_TRIGGER_ONCE   (0U)
#define KEY_TRIGGER_REPEAT (1U)

#define KEY_DEBOUNCE_SAMPLES       (3U)
#define KEY_LONG_PRESS_MS          (800U)
#define KEY_REPEAT_START_MS        (1000U)
#define KEY_REPEAT_PERIOD_MS       (200U)

typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE,
    KEY_EVENT_LONG_PRESS,
    KEY_EVENT_REPEAT
} KeyEvent;

void Key_Init(void);
uint8_t Key_ReadRaw(uint8_t repeat);
uint8_t Key_ReadDebounced(uint8_t repeat);
KeyEvent Key_Poll(uint32_t elapsedMs, uint8_t *keyCode);

#endif
