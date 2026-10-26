#ifndef ADC_H
#define ADC_H

#include <stdbool.h>
#include <stdint.h>

/* ADC1, regular single conversions, 12-bit resolution. */
void ADC_Init(void);
bool ADC_ReadChannel(uint8_t channel, uint16_t *result);

#endif /* ADC_H */
