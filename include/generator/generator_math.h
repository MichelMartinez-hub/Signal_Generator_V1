
#ifndef GENERATOR_MATH_H
#define GENERATOR_MATH_H

#include <stdbool.h>
#include "generator/waveform.h"

/**
 * @brief Initialize the mathematical waveform generator.
 */
void GeneratorMath_Init(void);

/**
 * @brief Configure the mathematical generator.
 *
 * @param waveform   Waveform type.
 * @param amplitude  Peak amplitude in volts [0, 5].
 * @param offset     DC offset in volts [-5, 5].
 *
 * @return true if configuration is valid.
 */
bool GeneratorMath_Configure(Waveform_t waveform,
                             float amplitude,
                             float offset);

/**
 * @brief Start real-time waveform generation.
 *
 * @param frequency  Waveform frequency in Hz.
 *
 * @return true if generation started successfully.
 */
bool GeneratorMath_Start(float frequency);

/**
 * @brief Stop mathematical waveform generation.
 */
void GeneratorMath_Stop(void);

#endif /* GENERATOR_MATH_H */
