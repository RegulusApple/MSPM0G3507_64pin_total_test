#ifndef SERIAL_PROTOCOL_H_
#define SERIAL_PROTOCOL_H_

#include <stdbool.h>
#include <stdint.h>

#include "analog_measure.h"

/*
 * Current meter UART binary frame:
 *
 * Byte0      : frame header, fixed 0xA5
 * Byte1      : current meter ID, uint8_t
 * Byte2~Byte5: current RMS value, IEEE-754 float, little-endian byte order
 * Byte6      : frame tail, fixed 0x5A
 */
#define SERIAL_PROTOCOL_FRAME_HEADER          (0xA5U)
#define SERIAL_PROTOCOL_FRAME_TAIL            (0x5AU)
#define SERIAL_PROTOCOL_CURRENT_FRAME_LEN     (7U)
#define SERIAL_PROTOCOL_FLOAT_SIZE            (4U)
#define SERIAL_PROTOCOL_VERSION               (1U)
#define SERIAL_PROTOCOL_TYPE_MEASUREMENT      (0x01U)
#define SERIAL_PROTOCOL_MEASUREMENT_PAYLOAD_LEN (34U)
#define SERIAL_PROTOCOL_MEASUREMENT_FRAME_LEN   (41U)

typedef struct {
    uint8_t meter_id;
    float current_rms;
} SerialProtocol_CurrentFrame;

typedef struct {
    uint8_t meter_id;
    uint32_t sequence;
    AnalogResult result;
} SerialProtocol_MeasurementFrame;

bool SerialProtocol_PackCurrentFrame(uint8_t meterId,
    float currentRms,
    uint8_t *outBuf,
    uint16_t outSize);

bool SerialProtocol_UnpackCurrentFrame(const uint8_t *frame,
    uint16_t frameLen,
    SerialProtocol_CurrentFrame *outData);

bool SerialProtocol_SendCurrentFrame(uint8_t meterId, float currentRms);
uint16_t SerialProtocol_Crc16Ccitt(const uint8_t *data, uint16_t length);
bool SerialProtocol_PackMeasurementFrame(uint8_t meterId,
    uint32_t sequence, const AnalogResult *result,
    uint8_t *outBuf, uint16_t outSize);
bool SerialProtocol_UnpackMeasurementFrame(const uint8_t *frame,
    uint16_t frameLen, SerialProtocol_MeasurementFrame *outData);
bool SerialProtocol_SendMeasurement(uint8_t meterId,
    uint32_t sequence, const AnalogResult *result);

#endif /* SERIAL_PROTOCOL_H_ */
