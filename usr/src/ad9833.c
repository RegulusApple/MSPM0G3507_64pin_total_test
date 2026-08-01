#include "ad9833.h"

#include "board_config.h"

bool AD9833_IsAvailable(void)
{
    return (BOARD_AD9833_AVAILABLE != 0U);
}

bool AD9833_Init(void)
{
    /*
     * The active board profile assigns PA13 to wireless power and PA14 to the
     * ADC analog-path MOS switch.  Refuse the legacy AD9833 mapping instead of
     * silently changing those pins to SPI functions.
     */
    return false;
}

bool AD9833_WaveOut(uint8_t mode, uint32_t frequencyHz, uint16_t phaseDeg)
{
    (void) mode;
    (void) frequencyHz;
    (void) phaseDeg;
    return false;
}
