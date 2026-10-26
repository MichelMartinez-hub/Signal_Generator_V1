#ifndef GENERATOR_LUT_H
#define GENERATOR_LUT_H


#include <stdint.h>
#include <stdbool.h>
#include "generator/waveform.h"

/**
 * @brief Initialize the LUT generator.
 */
void GeneratorLUT_Init(void);

/**
 * @brief Generate the waveform lookup table.
 *
 * @param waveform    Waveform type.
 * @param amplitude   Peak amplitude in volts [0, 5].
 * @param offset      DC offset in volts [-5, 5].
 *
 * @return true if configuration is valid.
 */
bool GeneratorLUT_Build(Waveform_t waveform,
                        float amplitude,
                        float offset);

/**
 * @brief Start waveform generation using DMA.
 *
 * @param frequency   Output waveform frequency in Hz.
 *
 * @return true if generation started successfully.
 */
bool GeneratorLUT_Start(float frequency);

/**
 * @brief Stop LUT waveform generation.
 */
void GeneratorLUT_Stop(void);

#endif /* GENERATOR_LUT_H */