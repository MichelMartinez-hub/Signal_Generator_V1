
#include "generator/generator.h"

#include "generator/generator_lut.h"
#include "generator/generator_math.h"
#include "bsp/wave_output.h"

#include "config/generator_config.h"
#include <stddef.h>
#include <math.h>
#include <stdbool.h>

#define OUTPUT_MIN_V (-5.0f)
#define OUTPUT_MAX_V ( 5.0f)

// Private configuration
typedef struct
{
    GeneratorMode_t mode;
    Waveform_t waveform;

    float frequency;
    float amplitude;
    float offset;

} GeneratorConfig_t;

static GeneratorConfig_t config;
static bool initialized = false;


// ---------------------------------------------
// Private validation functions
// ---------------------------------------------

static bool Generator_ValidWaveform(Waveform_t waveform)
{
    switch (waveform)
    {
        case WAVEFORM_DC:
        case WAVEFORM_SINE:
        case WAVEFORM_TRIANGLE:
        case WAVEFORM_SQUARE:
            return true;

        default:
            return false;
    }
}

static bool Generator_ValidOutputRange(
    float amplitude,
    float offset)
{
    if (!isfinite(amplitude) || !isfinite(offset))
    {
        return false;
    }

    if (amplitude < 0.0f || amplitude > 5.0f)
    {
        return false;
    }

    if (offset < OUTPUT_MIN_V ||
        offset > OUTPUT_MAX_V)
    {
        return false;
    }

    if ((offset + amplitude > OUTPUT_MAX_V) ||
        (offset - amplitude < OUTPUT_MIN_V))
    {
        return false;
    }

    return true;
}


// ---------------------------------------------
// Initialize generator
// ---------------------------------------------

void Generator_Init(void)
{
    WaveOutput_Init();

    GeneratorLUT_Init();
    GeneratorMath_Init();

    config.mode = GENERATOR_MODE_LUT;
    config.waveform = WAVEFORM_SINE;

    config.frequency = 10.0f;
    config.amplitude = 1.0f;
    config.offset = 0.0f;

    initialized = true;
}


// ---------------------------------------------
// Stop generator
// ---------------------------------------------

void Generator_Stop(void)
{
    if (!initialized)
    {
        return;
    }

    switch (config.mode)
    {
        case GENERATOR_MODE_LUT:
            GeneratorLUT_Stop();
            break;

        case GENERATOR_MODE_MATH:
            GeneratorMath_Stop();
            break;

        default:
            break;
    }
}


// ---------------------------------------------
// Start selected generator
// ---------------------------------------------

bool Generator_Start(void)
{
    if (!initialized)
    {
        return false;
    }

    Generator_Stop();

    switch (config.mode)
    {
        case GENERATOR_MODE_LUT:

            if (!GeneratorLUT_Build(
                    config.waveform,
                    config.amplitude,
                    config.offset))
            {
                return false;
            }

            return GeneratorLUT_Start(
                config.frequency
            );

        case GENERATOR_MODE_MATH:

            if (!GeneratorMath_Configure(
                    config.waveform,
                    config.amplitude,
                    config.offset))
            {
                return false;
            }

            return GeneratorMath_Start(
                config.frequency
            );

        default:
            return false;
    }
}


// ---------------------------------------------
// Select generation mode
// ---------------------------------------------

bool Generator_SetMode(GeneratorMode_t mode)
{
    if (!initialized)
    {
        return false;
    }

    if (mode != GENERATOR_MODE_LUT &&
        mode != GENERATOR_MODE_MATH)
    {
        return false;
    }

    if (config.mode != mode)
    {
        Generator_Stop();
        config.mode = mode;
    }

    return true;
}


// ---------------------------------------------
// Select waveform
// ---------------------------------------------

bool Generator_SetWaveform(Waveform_t waveform)
{
    if (!initialized ||
        !Generator_ValidWaveform(waveform))
    {
        return false;
    }

    if (config.waveform != waveform)
    {
        Generator_Stop();
        config.waveform = waveform;
    }

    return true;
}


// ---------------------------------------------
// Set frequency
// ---------------------------------------------

bool Generator_SetFrequency(float frequency)
{
    if (!initialized ||
        !isfinite(frequency) ||
        frequency < GENERATOR_MIN_FREQUENCY_HZ ||
        frequency > GENERATOR_MAX_FREQUENCY_HZ)
    {
        return false;
    }

    if (config.frequency != frequency)
    {
        Generator_Stop();
        config.frequency = frequency;
    }

    return true;
}


// ---------------------------------------------
// Set peak amplitude
// ---------------------------------------------

bool Generator_SetAmplitude(float amplitude)
{
    if (!initialized ||
        !Generator_ValidOutputRange(
            amplitude,
            config.offset))
    {
        return false;
    }

    if (config.amplitude != amplitude)
    {
        Generator_Stop();
        config.amplitude = amplitude;
    }

    return true;
}


// ---------------------------------------------
// Set DC offset
// ---------------------------------------------

bool Generator_SetOffset(float offset)
{
    if (!initialized ||
        !Generator_ValidOutputRange(
            config.amplitude,
            offset))
    {
        return false;
    }

    if (config.offset != offset)
    {
        Generator_Stop();
        config.offset = offset;
    }

    return true;
}
bool Generator_GetSettings(GeneratorSettings_t *out)
{
    if (!initialized || out == NULL)
    {
        return false;
    }

    out->mode = config.mode;
    out->waveform = config.waveform;
    out->frequency = config.frequency;
    out->amplitude = config.amplitude;
    out->offset = config.offset;
    return true;
}
