#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdbool.h>
#include "generator/waveform.h"

typedef enum
{
    GENERATOR_MODE_LUT,
    GENERATOR_MODE_MATH
} GeneratorMode_t;

/* NEW: read-only snapshot for STATUS. Original setters remain unchanged. */
typedef struct
{
    GeneratorMode_t mode;
    Waveform_t waveform;
    float frequency;
    float amplitude;
    float offset;
} GeneratorSettings_t;

void Generator_Init(void);
bool Generator_Start(void);
void Generator_Stop(void);
bool Generator_SetMode(GeneratorMode_t mode);
bool Generator_SetWaveform(Waveform_t waveform);
bool Generator_SetFrequency(float frequency);
bool Generator_SetAmplitude(float amplitude);
bool Generator_SetOffset(float offset);

/* NEW: returns a copy of current settings; no hardware mutation. */
bool Generator_GetSettings(GeneratorSettings_t *out);

#endif /* GENERATOR_H */

