#ifndef DAC_H
#define DAC_H

#include <stdint.h>

void DAC_Init(void);
void DAC_SetValue(uint16_t value);

void DAC_EnableTrigger(void);
void DAC_DisableTrigger(void);

void DAC_EnableDMA(void);
void DAC_DisableDMA(void);
void DAC_ClearUnderrun(void);

#endif // DAC_H