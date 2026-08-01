#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <stdint.h>

typedef enum {
    SOFT_I2C_OK = 0,
    SOFT_I2C_ERR_ARGUMENT,
    SOFT_I2C_ERR_NACK,
    SOFT_I2C_ERR_BUS_STUCK
} SoftI2C_Status;

/*
 * Software I2C bus used by the display:
 *   SDA = PA0 (MSP-LITO-G3507 J2 pin 23)
 *   SCL = PA1 (MSP-LITO-G3507 J2 pin 24)
 *
 * Both lines are open-drain. MSP-LITO-G3507 R7/R8 already fit one 2.2 kOhm
 * pull-up from each line to 3.3 V, so the MCU internal pull-ups remain disabled.
 */
void SoftI2C_Init(void);
SoftI2C_Status SoftI2C_RecoverBus(void);
SoftI2C_Status SoftI2C_Probe(uint8_t address7);
SoftI2C_Status SoftI2C_Write(
    uint8_t address7, const uint8_t *data, uint16_t length);

#endif
