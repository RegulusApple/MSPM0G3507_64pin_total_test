#include "key.h"

static uint8_t gKeyLastState = 0U;

void Key_Init(void)
{
    SYSCFG_DL_KEY_init();
    gKeyLastState = 0U;
}

static uint8_t Key_ReadGPIOState(void)
{
    uint32_t pinsA = DL_GPIO_readPins(KEY_PORTA, KEY_ALL_PORTA_PINS);
    uint32_t pinsB = DL_GPIO_readPins(KEY_PORTB, KEY_ALL_PORTB_PINS);
#if (KEY_ACTIVE_LOW != 0U)
    pinsA = (~pinsA) & KEY_ALL_PORTA_PINS;
    pinsB = (~pinsB) & KEY_ALL_PORTB_PINS;
#endif
    uint8_t value = 0U;

    if ((pinsA & KEY1_PIN) != 0U) {
        value |= KEY_1;
    }
    if ((pinsB & KEY2_PIN) != 0U) {
        value |= KEY_2;
    }
    if ((pinsB & KEY3_PIN) != 0U) {
        value |= KEY_3;
    }
    if ((pinsA & KEY4_PIN) != 0U) {
        value |= KEY_4;
    }
    if ((pinsA & KEY5_PIN) != 0U) {
        value |= KEY_5;
    }

    return value;
}

uint8_t Key_ReadRaw(uint8_t repeat)
{
    uint8_t now = Key_ReadGPIOState();
    uint8_t pressed;

    if (repeat != 0U) {
        gKeyLastState = now;
        return now;
    }

    pressed = (uint8_t) (now & (uint8_t) (~gKeyLastState));
    gKeyLastState = now;
    return pressed;
}

uint8_t Key_ReadDebounced(uint8_t repeat)
{
    return Key_ReadRaw(repeat);
}

