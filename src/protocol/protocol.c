#include "protocol/protocol.h"
#include "protocol/crc8.h"
#include <string.h>
#include <stddef.h>

bool Protocol_Encode(uint8_t type, uint8_t command,
                     const uint8_t *payload, uint8_t length,
                     uint8_t *out, size_t capacity, size_t *written)
{
    if (out == NULL || written == NULL || length > PROTOCOL_MAX_PAYLOAD ||
        (payload == NULL && length != 0U) || capacity < (size_t)length + 5U)
        return false;
    out[0] = PROTOCOL_SOF;
    out[1] = type;
    out[2] = command;
    out[3] = length;
    if (length != 0U) memcpy(&out[4], payload, length);
    out[4U + length] = CRC8_Xor(&out[1], (size_t)length + 3U);
    *written = (size_t)length + 5U;
    return true;
}

bool Protocol_Decode(const uint8_t *frame, size_t size,
                     uint8_t *type, uint8_t *command,
                     uint8_t *payload, size_t payload_capacity, uint8_t *length)
{
    if (frame == NULL || type == NULL || command == NULL || length == NULL ||
        size < 5U || frame[0] != PROTOCOL_SOF ||
        payload_capacity < frame[3] ||
        frame[3] > PROTOCOL_MAX_PAYLOAD || size != (size_t)frame[3] + 5U ||
        (payload == NULL && frame[3] != 0U)) return false;
    if (CRC8_Xor(&frame[1], size - 2U) != frame[size - 1U]) return false;
    *type = frame[1];
    *command = frame[2];
    *length = frame[3];
    if (*length != 0U) memcpy(payload, &frame[4], *length);
    return true;
}
