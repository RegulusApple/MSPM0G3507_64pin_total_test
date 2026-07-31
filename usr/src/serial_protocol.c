#include "serial_protocol.h"

#include <string.h>

#include "usr_uart.h"
#include "ti_msp_dl_config.h"

#define SERIAL_PROTOCOL_UART_WAIT_TIMEOUT      (CPUCLK_FREQ / 10U)

static void SerialProtocol_WriteFloatLittleEndian(float value, uint8_t *outBytes)
{
    uint8_t raw[SERIAL_PROTOCOL_FLOAT_SIZE];

    (void) memcpy(raw, &value, SERIAL_PROTOCOL_FLOAT_SIZE);

    /*
     * MSPM0 is little-endian. Keeping the wire order explicitly documented
     * makes the upper-computer parser stable.
     */
    outBytes[0] = raw[0];
    outBytes[1] = raw[1];
    outBytes[2] = raw[2];
    outBytes[3] = raw[3];
}

static float SerialProtocol_ReadFloatLittleEndian(const uint8_t *inBytes)
{
    float value;
    uint8_t raw[SERIAL_PROTOCOL_FLOAT_SIZE];

    raw[0] = inBytes[0];
    raw[1] = inBytes[1];
    raw[2] = inBytes[2];
    raw[3] = inBytes[3];

    (void) memcpy(&value, raw, SERIAL_PROTOCOL_FLOAT_SIZE);
    return value;
}

static void SerialProtocol_WaitUartTxIdle(void)
{
    uint32_t timeout = SERIAL_PROTOCOL_UART_WAIT_TIMEOUT;

    while (((DL_UART_Main_isTXFIFOEmpty(UART_DEBUG_INST) == false) ||
               (DL_UART_Main_isBusy(UART_DEBUG_INST) == true)) &&
        (timeout > 0U)) {
        timeout--;
    }
}

bool SerialProtocol_PackCurrentFrame(uint8_t meterId,
    float currentRms,
    uint8_t *outBuf,
    uint16_t outSize)
{
    if ((outBuf == NULL) || (outSize < SERIAL_PROTOCOL_CURRENT_FRAME_LEN)) {
        return false;
    }

    outBuf[0] = SERIAL_PROTOCOL_FRAME_HEADER;
    outBuf[1] = meterId;
    SerialProtocol_WriteFloatLittleEndian(currentRms, &outBuf[2]);
    outBuf[6] = SERIAL_PROTOCOL_FRAME_TAIL;

    return true;
}

bool SerialProtocol_UnpackCurrentFrame(const uint8_t *frame,
    uint16_t frameLen,
    SerialProtocol_CurrentFrame *outData)
{
    if ((frame == NULL) || (outData == NULL) ||
        (frameLen < SERIAL_PROTOCOL_CURRENT_FRAME_LEN)) {
        return false;
    }

    if ((frame[0] != SERIAL_PROTOCOL_FRAME_HEADER) ||
        (frame[6] != SERIAL_PROTOCOL_FRAME_TAIL)) {
        return false;
    }

    outData->meter_id = frame[1];
    outData->current_rms = SerialProtocol_ReadFloatLittleEndian(&frame[2]);

    return true;
}

bool SerialProtocol_SendCurrentFrame(uint8_t meterId, float currentRms)
{
    uint8_t frame[SERIAL_PROTOCOL_CURRENT_FRAME_LEN];

    if (SerialProtocol_PackCurrentFrame(meterId,
            currentRms,
            frame,
            SERIAL_PROTOCOL_CURRENT_FRAME_LEN) == false) {
        return false;
    }

    USR_UART_sendBytes(frame, SERIAL_PROTOCOL_CURRENT_FRAME_LEN);
    SerialProtocol_WaitUartTxIdle();

    return true;
}
