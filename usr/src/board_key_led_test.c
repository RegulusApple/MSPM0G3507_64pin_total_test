#include "board_key_led_test.h"

#include <stdbool.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"

#include "system_time.h"

static bool BoardKeyLedTest_ButtonPressed(void)
{
    return ((DL_GPIO_readPins(KEY_LED_PORT, KEY_LED_BUTTON_PIN) &
             KEY_LED_BUTTON_PIN) == 0U);
}

static void BoardKeyLedTest_DebounceDelay(void)
{
    SystemTime_DelayMs(20U);
}

void BoardKeyLedTest_Run(void)
{
    while (1) {
        while (BoardKeyLedTest_ButtonPressed() == false) {
            /* Wait for the active-low S2 button on PB21. */
        }

        BoardKeyLedTest_DebounceDelay();
        if (BoardKeyLedTest_ButtonPressed() == false) {
            continue;
        }

        DL_GPIO_togglePins(KEY_LED_PORT, KEY_LED_LED_PIN);

        do {
            while (BoardKeyLedTest_ButtonPressed()) {
                /* One LED transition per complete press-and-release cycle. */
            }
            BoardKeyLedTest_DebounceDelay();
        } while (BoardKeyLedTest_ButtonPressed());
    }
}
