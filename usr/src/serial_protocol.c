#include "serial_protocol.h"

#include <string.h>

#include "usr_uart.h"
#include "ti_msp_dl_config.h"
#include "system_time.h"

#define SERIAL_PROTOCOL_UART_WAIT_TIMEOUT_MS   (100U)

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

static bool SerialProtocol_WaitUartTxIdle(void)
{
    uint32_t timeout =
        SystemTime_CyclesFromMs(SERIAL_PROTOCOL_UART_WAIT_TIMEOUT_MS);

    while (((DL_UART_Main_isTXFIFOEmpty(UART_DEBUG_INST) == false) ||
               (DL_UART_Main_isBusy(UART_DEBUG_INST) == true)) &&
        (timeout > 0U)) {
        timeout--;
    }
    return timeout > 0U;
}

static void SerialProtocol_WriteU32LittleEndian(uint32_t value,
    uint8_t *outBytes)
{
    outBytes[0] = (uint8_t) value;
    outBytes[1] = (uint8_t) (value >> 8U);
    outBytes[2] = (uint8_t) (value >> 16U);
    outBytes[3] = (uint8_t) (value >> 24U);
}

static uint32_t SerialProtocol_ReadU32LittleEndian(const uint8_t *inBytes)
{
    return (uint32_t) inBytes[0] |
        ((uint32_t) inBytes[1] << 8U) |
        ((uint32_t) inBytes[2] << 16U) |
        ((uint32_t) inBytes[3] << 24U);
}

uint16_t SerialProtocol_Crc16Ccitt(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;

    if ((data == NULL) && (length != 0U)) {
        return 0U;
    }
    for (i = 0U; i < length; i++) {
        uint8_t bit;
        crc ^= (uint16_t) data[i] << 8U;
        for (bit = 0U; bit < 8U; bit++) {
            crc = ((crc & 0x8000U) != 0U) ?
                (uint16_t) ((crc << 1U) ^ 0x1021U) :
                (uint16_t) (crc << 1U);
        }
    }
    return crc;
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
        (frameLen != SERIAL_PROTOCOL_CURRENT_FRAME_LEN)) {
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
    return SerialProtocol_WaitUartTxIdle();
}

bool SerialProtocol_PackMeasurementFrame(uint8_t meterId,
    uint32_t sequence, const AnalogResult *result,
    uint8_t *outBuf, uint16_t outSize)
{
    uint8_t flags = 0U;
    uint16_t crc;

    if ((result == NULL) || (outBuf == NULL) ||
        (outSize < SERIAL_PROTOCOL_MEASUREMENT_FRAME_LEN)) {
        return false;
    }

    if (result->frequencyValid) {
        flags |= 0x01U;
    }
    if (result->phaseValid) {
        flags |= 0x02U;
    }
    if (result->clipped) {
        flags |= 0x04U;
    }
    if (result->thdValid) {
        flags |= 0x08U;
    }

    outBuf[0] = SERIAL_PROTOCOL_FRAME_HEADER;
    outBuf[1] = SERIAL_PROTOCOL_VERSION;
    outBuf[2] = SERIAL_PROTOCOL_TYPE_MEASUREMENT;
    outBuf[3] = SERIAL_PROTOCOL_MEASUREMENT_PAYLOAD_LEN;
    outBuf[4] = meterId;
    SerialProtocol_WriteU32LittleEndian(sequence, &outBuf[5]);
    SerialProtocol_WriteFloatLittleEndian(result->dcMv, &outBuf[9]);
    SerialProtocol_WriteFloatLittleEndian(result->rmsMv, &outBuf[13]);
    SerialProtocol_WriteFloatLittleEndian(result->peakMv, &outBuf[17]);
    SerialProtocol_WriteFloatLittleEndian(result->peakToPeakMv, &outBuf[21]);
    SerialProtocol_WriteFloatLittleEndian(result->frequencyHz, &outBuf[25]);
    SerialProtocol_WriteFloatLittleEndian(result->phaseDeg, &outBuf[29]);
    SerialProtocol_WriteFloatLittleEndian(result->thdPercent, &outBuf[33]);
    outBuf[37] = flags;
    crc = SerialProtocol_Crc16Ccitt(&outBuf[1], 37U);
    outBuf[38] = (uint8_t) crc;
    outBuf[39] = (uint8_t) (crc >> 8U);
    outBuf[40] = SERIAL_PROTOCOL_FRAME_TAIL;
    return true;
}

bool SerialProtocol_UnpackMeasurementFrame(const uint8_t *frame,
    uint16_t frameLen, SerialProtocol_MeasurementFrame *outData)
{
    uint16_t expectedCrc;
    uint16_t receivedCrc;
    uint8_t flags;

    if ((frame == NULL) || (outData == NULL) ||
        (frameLen != SERIAL_PROTOCOL_MEASUREMENT_FRAME_LEN) ||
        (frame[0] != SERIAL_PROTOCOL_FRAME_HEADER) ||
        (frame[1] != SERIAL_PROTOCOL_VERSION) ||
        (frame[2] != SERIAL_PROTOCOL_TYPE_MEASUREMENT) ||
        (frame[3] != SERIAL_PROTOCOL_MEASUREMENT_PAYLOAD_LEN) ||
        (frame[40] != SERIAL_PROTOCOL_FRAME_TAIL)) {
        return false;
    }

    expectedCrc = SerialProtocol_Crc16Ccitt(&frame[1], 37U);
    receivedCrc = (uint16_t) frame[38] | ((uint16_t) frame[39] << 8U);
    if (expectedCrc != receivedCrc) {
        return false;
    }

    outData->meter_id = frame[4];
    outData->sequence = SerialProtocol_ReadU32LittleEndian(&frame[5]);
    outData->result.dcMv = SerialProtocol_ReadFloatLittleEndian(&frame[9]);
    outData->result.rmsMv = SerialProtocol_ReadFloatLittleEndian(&frame[13]);
    outData->result.peakMv = SerialProtocol_ReadFloatLittleEndian(&frame[17]);
    outData->result.peakToPeakMv =
        SerialProtocol_ReadFloatLittleEndian(&frame[21]);
    outData->result.frequencyHz =
        SerialProtocol_ReadFloatLittleEndian(&frame[25]);
    outData->result.phaseDeg =
        SerialProtocol_ReadFloatLittleEndian(&frame[29]);
    outData->result.thdPercent =
        SerialProtocol_ReadFloatLittleEndian(&frame[33]);
    flags = frame[37];
    outData->result.frequencyValid = (flags & 0x01U) != 0U;
    outData->result.phaseValid = (flags & 0x02U) != 0U;
    outData->result.clipped = (flags & 0x04U) != 0U;
    outData->result.thdValid = (flags & 0x08U) != 0U;
    outData->result.sampleCount = 0U;
    outData->result.minCode = 0U;
    outData->result.maxCode = 0U;
    return true;
}

bool SerialProtocol_SendMeasurement(uint8_t meterId,
    uint32_t sequence, const AnalogResult *result)
{
    uint8_t frame[SERIAL_PROTOCOL_MEASUREMENT_FRAME_LEN];

    if (SerialProtocol_PackMeasurementFrame(meterId, sequence, result,
            frame, sizeof(frame)) == false) {
        return false;
    }
    USR_UART_sendBytes(frame, sizeof(frame));
    return SerialProtocol_WaitUartTxIdle();
}
