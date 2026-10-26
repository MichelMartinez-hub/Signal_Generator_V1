#ifndef PROTOCOL_CRC8_H
#define PROTOCOL_CRC8_H
#include <stdint.h>
#include <stddef.h>
/* Legacy XOR checksum for the optional binary protocol (not polynomial CRC8). */
uint8_t CRC8_Xor(const uint8_t *data, size_t length);
#endif
