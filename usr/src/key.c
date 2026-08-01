#include "key.h"

static uint8_t gKeyRawLastState;
static uint8_t gKeyDebounceCandidate;
static uint8_t gKeyDebouncedState;
static uint8_t gKeyDebouncedLastState;
static uint8_t gKeyCandidateCount;
static uint8_t gKeyEventState;
static uint32_t gKeyPressedMs;
static uint32_t gKeyLastRepeatMs;
static bool gKeyLongPressSent;

void Key_Init(void)
{
    gKeyRawLastState = 0U;
    gKeyDebounceCandidate = 0U;
    gKeyDebouncedState = 0U;
    gKeyDebouncedLastState = 0U;
    gKeyCandidateCount = 0U;
    gKeyEventState = 0U;
    gKeyPressedMs = 0U;
    gKeyLastRepeatMs = 0U;
    gKeyLongPressSent = false;
}

static uint8_t Key_ReadGPIOState(void)
{
    uint8_t value = 0U;

#if (KEY_ACTIVE_LOW != 0U)
    if ((DL_GPIO_readPins(KEY_KEY1_PORT, KEY_KEY1_PIN) & KEY_KEY1_PIN) == 0U) {
        value |= KEY_1;
    }
    if ((DL_GPIO_readPins(KEY_KEY2_PORT, KEY_KEY2_PIN) & KEY_KEY2_PIN) == 0U) {
        value |= KEY_2;
    }
    if ((DL_GPIO_readPins(KEY_KEY3_PORT, KEY_KEY3_PIN) & KEY_KEY3_PIN) == 0U) {
        value |= KEY_3;
    }
    if ((DL_GPIO_readPins(KEY_KEY4_PORT, KEY_KEY4_PIN) & KEY_KEY4_PIN) == 0U) {
        value |= KEY_4;
    }
    if ((DL_GPIO_readPins(KEY_KEY5_PORT, KEY_KEY5_PIN) & KEY_KEY5_PIN) == 0U) {
        value |= KEY_5;
    }
#else
    if ((DL_GPIO_readPins(KEY_KEY1_PORT, KEY_KEY1_PIN) & KEY_KEY1_PIN) != 0U) {
        value |= KEY_1;
    }
    if ((DL_GPIO_readPins(KEY_KEY2_PORT, KEY_KEY2_PIN) & KEY_KEY2_PIN) != 0U) {
        value |= KEY_2;
    }
    if ((DL_GPIO_readPins(KEY_KEY3_PORT, KEY_KEY3_PIN) & KEY_KEY3_PIN) != 0U) {
        value |= KEY_3;
    }
    if ((DL_GPIO_readPins(KEY_KEY4_PORT, KEY_KEY4_PIN) & KEY_KEY4_PIN) != 0U) {
        value |= KEY_4;
    }
    if ((DL_GPIO_readPins(KEY_KEY5_PORT, KEY_KEY5_PIN) & KEY_KEY5_PIN) != 0U) {
        value |= KEY_5;
    }
#endif

    return value;
}

uint8_t Key_ReadRaw(uint8_t repeat)
{
    uint8_t now = Key_ReadGPIOState();
    uint8_t pressed;

    if (repeat != 0U) {
        gKeyRawLastState = now;
        return now;
    }

    pressed = (uint8_t) (now & (uint8_t) (~gKeyRawLastState));
    gKeyRawLastState = now;
    return pressed;
}

static uint8_t Key_UpdateDebounce(uint8_t now)
{
    if (now != gKeyDebounceCandidate) {
        gKeyDebounceCandidate = now;
        gKeyCandidateCount = 1U;
    } else if (gKeyCandidateCount < KEY_DEBOUNCE_SAMPLES) {
        gKeyCandidateCount++;
    } else {
        /* Saturate to avoid wrapping after long operation. */
    }

    if (gKeyCandidateCount >= KEY_DEBOUNCE_SAMPLES) {
        gKeyDebouncedState = gKeyDebounceCandidate;
    }
    return gKeyDebouncedState;
}

uint8_t Key_ReadDebounced(uint8_t repeat)
{
    uint8_t now = Key_UpdateDebounce(Key_ReadGPIOState());
    uint8_t pressed;

    if (repeat != 0U) {
        gKeyDebouncedLastState = now;
        return now;
    }

    pressed = (uint8_t) (now & (uint8_t) (~gKeyDebouncedLastState));
    gKeyDebouncedLastState = now;
    return pressed;
}

KeyEvent Key_Poll(uint32_t elapsedMs, uint8_t *keyCode)
{
    uint8_t now;

    if (keyCode == NULL) {
        return KEY_EVENT_NONE;
    }

    now = Key_UpdateDebounce(Key_ReadGPIOState());
    if (now != gKeyEventState) {
        uint8_t previous = gKeyEventState;

        gKeyEventState = now;
        gKeyPressedMs = 0U;
        gKeyLastRepeatMs = 0U;
        gKeyLongPressSent = false;
        if (now != 0U) {
            *keyCode = now;
            return KEY_EVENT_PRESS;
        }
        *keyCode = previous;
        return KEY_EVENT_RELEASE;
    }

    if (now == 0U) {
        *keyCode = 0U;
        return KEY_EVENT_NONE;
    }

    if ((UINT32_MAX - gKeyPressedMs) < elapsedMs) {
        gKeyPressedMs = UINT32_MAX;
    } else {
        gKeyPressedMs += elapsedMs;
    }
    *keyCode = now;

    if ((gKeyLongPressSent == false) &&
        (gKeyPressedMs >= KEY_LONG_PRESS_MS)) {
        gKeyLongPressSent = true;
        gKeyLastRepeatMs = gKeyPressedMs;
        return KEY_EVENT_LONG_PRESS;
    }

    if ((gKeyPressedMs >= KEY_REPEAT_START_MS) &&
        ((gKeyPressedMs - gKeyLastRepeatMs) >= KEY_REPEAT_PERIOD_MS)) {
        gKeyLastRepeatMs = gKeyPressedMs;
        return KEY_EVENT_REPEAT;
    }
    return KEY_EVENT_NONE;
}

