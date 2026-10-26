#ifndef CONTROLS_H
#define CONTROLS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint16_t frequency_adc;  /* PC0, ADC1_IN10 */
    uint16_t amplitude_adc;  /* PC1, ADC1_IN11 */
    uint16_t offset_adc;     /* PB0, ADC1_IN8  */
    int32_t encoder_delta;   /* TIM2 quadrature counts since last read */
} ControlsRaw_t;

void Controls_Init(void);
bool Controls_ReadRaw(ControlsRaw_t *out);

#endif /* CONTROLS_H */