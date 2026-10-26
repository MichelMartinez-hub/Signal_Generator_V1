#ifndef PROTOCOL_BINARY_H
#define PROTOCOL_BINARY_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PROTOCOL_SOF          0xAAU
#define PROTOCOL_MAX_PAYLOAD  32U
#define PROTOCOL_MAX_FRAME    (5U + PROTOCOL_MAX_PAYLOAD)
/* [SOF, type, command, length, data..., XOR(type..data)] */
bool Protocol_Encode(uint8_t type, uint8_t command,
                     const uint8_t *payload, uint8_t length,
                     uint8_t *out, size_t capacity, size_t *written);
bool Protocol_Decode(const uint8_t *frame, size_t size,
                     uint8_t *type, uint8_t *command,
                     uint8_t *payload, size_t payload_capacity, uint8_t *length);
#endif
