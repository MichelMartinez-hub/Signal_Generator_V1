#ifndef DMA_H
#define DMA_H
#include <stdint.h>
#include <stdbool.h>

void DMA_Init(void);
bool DMA_StartDac(const uint16_t *data,
                             uint32_t length);

void DMA_StopDac(void);

#endif // DMA_H