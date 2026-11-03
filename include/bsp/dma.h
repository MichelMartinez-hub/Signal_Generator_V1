#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    DMA_DAC_HALF,
    DMA_DAC_FULL,
    DMA_DAC_ERROR
} DMA_DAC_Event_t;

typedef void (*DMA_DAC_Callback)(DMA_DAC_Event_t event);

/* Existing API retained. */
void DMA_Init(void);
bool DMA_StartDac(const uint16_t *data, uint32_t length);
void DMA_StopDac(void);

/* New streaming API; callback runs from DMA1_Stream5_IRQHandler. */
bool DMA_StartDacWithCallback(const uint16_t *data,
                              uint32_t length,
                              DMA_DAC_Callback callback);
#endif
