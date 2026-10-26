#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/* KY-040 CLK = PA0 (TIM2_CH1), DT = PA1 (TIM2_CH2). */
void Encoder_Init(void);

/* Signed quadrature counts since previous call (not detents). */
int32_t Encoder_ReadDelta(void);

#endif /* ENCODER_H */
