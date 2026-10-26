#ifndef MAP_H
#define MAP_H

#include <stdint.h>

/* Linear mapping from [0, input_max] into [output_min, output_max].
   Inputs above input_max are clamped. */
float Map_LinearU16(uint16_t input,
                    uint16_t input_max,
                    float output_min,
                    float output_max);

/* Quantize a floating point value to the nearest positive step. */
float Map_Quantize(float value, float step);

#endif /* MAP_H */
