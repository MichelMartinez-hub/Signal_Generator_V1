#include "protocol/crc8.h"
#include <stddef.h>
uint8_t CRC8_Xor(const uint8_t *data, size_t length)
{
    uint8_t check = 0U;
    if (data == NULL) return check;
    for (size_t i = 0U; i < length; ++i) check ^= data[i];
    return check;
}
