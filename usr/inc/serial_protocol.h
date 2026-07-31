#ifndef SERIAL_PROTOCOL_H_
#define SERIAL_PROTOCOL_H_

#include <stdbool.h>
#include <stdint.h>

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

typedef struct {
    uint8_t meter_id;
    float current_rms;
} SerialProtocol_CurrentFrame;

bool SerialProtocol_PackCurrentFrame(uint8_t meterId,
    float currentRms,
    uint8_t *outBuf,
    uint16_t outSize);

bool SerialProtocol_UnpackCurrentFrame(const uint8_t *frame,
    uint16_t frameLen,
    SerialProtocol_CurrentFrame *outData);

bool SerialProtocol_SendCurrentFrame(uint8_t meterId, float currentRms);

#endif /* SERIAL_PROTOCOL_H_ */
