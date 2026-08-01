#include "soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>

#include "ti_msp_dl_config.h"
#include "system_time.h"

#define SOFT_I2C_PORT                 I2C_LCD_PINS_PORT
#define SOFT_I2C_SDA_PIN              I2C_LCD_PINS_LCD_SDA_PIN
#define SOFT_I2C_SDA_IOMUX            I2C_LCD_PINS_LCD_SDA_IOMUX
#define SOFT_I2C_SCL_PIN              I2C_LCD_PINS_LCD_SCL_PIN
#define SOFT_I2C_SCL_IOMUX            I2C_LCD_PINS_LCD_SCL_IOMUX

/* Standard-mode first-test speed for the board's populated 2.2 kOhm pulls. */
#define SOFT_I2C_BUS_HZ               (100000UL)
#define SOFT_I2C_HALF_PERIOD_US       (1000000UL / (2UL * SOFT_I2C_BUS_HZ))
#define SOFT_I2C_SCL_TIMEOUT_MS       (2U)

static void SoftI2C_Delay(void)
{
    SystemTime_DelayUs(SOFT_I2C_HALF_PERIOD_US);
}

static void SoftI2C_DriveLow(uint32_t pin)
{
    DL_GPIO_clearPins(SOFT_I2C_PORT, pin);
    DL_GPIO_enableOutput(SOFT_I2C_PORT, pin);
}

static void SoftI2C_Release(uint32_t pin)
{
    DL_GPIO_disableOutput(SOFT_I2C_PORT, pin);
}

static bool SoftI2C_IsHigh(uint32_t pin)
{
    return (DL_GPIO_readPins(SOFT_I2C_PORT, pin) != 0U);
}

static bool SoftI2C_ReleaseSclAndWait(void)
{
    uint32_t timeout = SystemTime_CyclesFromMs(SOFT_I2C_SCL_TIMEOUT_MS);

    SoftI2C_Release(SOFT_I2C_SCL_PIN);
    while (!SoftI2C_IsHigh(SOFT_I2C_SCL_PIN)) {
        if (timeout == 0U) {
            return false;
        }
        timeout--;
    }
    return true;
}

static SoftI2C_Status SoftI2C_Start(void)
{
    SoftI2C_Release(SOFT_I2C_SDA_PIN);
    if (!SoftI2C_ReleaseSclAndWait()) {
        return SOFT_I2C_ERR_BUS_STUCK;
    }
    SoftI2C_Delay();
    if (!SoftI2C_IsHigh(SOFT_I2C_SDA_PIN)) {
        return SOFT_I2C_ERR_BUS_STUCK;
    }

    SoftI2C_DriveLow(SOFT_I2C_SDA_PIN);
    SoftI2C_Delay();
    SoftI2C_DriveLow(SOFT_I2C_SCL_PIN);
    SoftI2C_Delay();
    return SOFT_I2C_OK;
}

static SoftI2C_Status SoftI2C_Stop(void)
{
    SoftI2C_DriveLow(SOFT_I2C_SDA_PIN);
    SoftI2C_Delay();
    if (!SoftI2C_ReleaseSclAndWait()) {
        SoftI2C_Release(SOFT_I2C_SDA_PIN);
        return SOFT_I2C_ERR_BUS_STUCK;
    }
    SoftI2C_Delay();
    SoftI2C_Release(SOFT_I2C_SDA_PIN);
    SoftI2C_Delay();

    return SoftI2C_IsHigh(SOFT_I2C_SDA_PIN) ?
        SOFT_I2C_OK : SOFT_I2C_ERR_BUS_STUCK;
}

static SoftI2C_Status SoftI2C_WriteByte(uint8_t value)
{
    uint8_t bit;
    bool acknowledged;

    for (bit = 0U; bit < 8U; bit++) {
        SoftI2C_DriveLow(SOFT_I2C_SCL_PIN);
        if ((value & 0x80U) != 0U) {
            SoftI2C_Release(SOFT_I2C_SDA_PIN);
        } else {
            SoftI2C_DriveLow(SOFT_I2C_SDA_PIN);
        }
        SoftI2C_Delay();
        if (!SoftI2C_ReleaseSclAndWait()) {
            return SOFT_I2C_ERR_BUS_STUCK;
        }
        SoftI2C_Delay();
        SoftI2C_DriveLow(SOFT_I2C_SCL_PIN);
        value <<= 1U;
    }

    SoftI2C_Release(SOFT_I2C_SDA_PIN);
    SoftI2C_Delay();
    if (!SoftI2C_ReleaseSclAndWait()) {
        return SOFT_I2C_ERR_BUS_STUCK;
    }
    SoftI2C_Delay();
    acknowledged = !SoftI2C_IsHigh(SOFT_I2C_SDA_PIN);
    SoftI2C_DriveLow(SOFT_I2C_SCL_PIN);
    SoftI2C_Delay();

    return acknowledged ? SOFT_I2C_OK : SOFT_I2C_ERR_NACK;
}

void SoftI2C_Init(void)
{
    /* MSP-LITO R7/R8 provide 2.2 kOhm pull-ups; keep internal pulls disabled. */
    DL_GPIO_initDigitalInputFeatures(SOFT_I2C_SDA_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(SOFT_I2C_SCL_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_clearPins(SOFT_I2C_PORT, SOFT_I2C_SDA_PIN | SOFT_I2C_SCL_PIN);
    SoftI2C_Release(SOFT_I2C_SDA_PIN | SOFT_I2C_SCL_PIN);
    (void) SoftI2C_RecoverBus();
}

SoftI2C_Status SoftI2C_RecoverBus(void)
{
    uint8_t pulse;

    SoftI2C_Release(SOFT_I2C_SDA_PIN);
    for (pulse = 0U; (pulse < 9U) && !SoftI2C_IsHigh(SOFT_I2C_SDA_PIN);
         pulse++) {
        SoftI2C_DriveLow(SOFT_I2C_SCL_PIN);
        SoftI2C_Delay();
        if (!SoftI2C_ReleaseSclAndWait()) {
            return SOFT_I2C_ERR_BUS_STUCK;
        }
        SoftI2C_Delay();
    }

    return SoftI2C_Stop();
}

SoftI2C_Status SoftI2C_Probe(uint8_t address7)
{
    return SoftI2C_Write(address7, NULL, 0U);
}

SoftI2C_Status SoftI2C_Write(
    uint8_t address7, const uint8_t *data, uint16_t length)
{
    SoftI2C_Status status;
    SoftI2C_Status stopStatus;
    uint16_t index;

    if ((address7 > 0x7FU) || ((length > 0U) && (data == NULL))) {
        return SOFT_I2C_ERR_ARGUMENT;
    }

    status = SoftI2C_Start();
    if (status == SOFT_I2C_OK) {
        status = SoftI2C_WriteByte((uint8_t) (address7 << 1U));
    }
    for (index = 0U; (index < length) && (status == SOFT_I2C_OK);
         index++) {
        status = SoftI2C_WriteByte(data[index]);
    }

    stopStatus = SoftI2C_Stop();
    return (status != SOFT_I2C_OK) ? status : stopStatus;
}
