#include "generator/generator_math.h"
#include "generator/waveform.h"
#include "config/board_config.h"
#include "bsp/wave_output.h"

#include <stdint.h>
#include <stdbool.h>
#include "config/generator_config.h"
#include <math.h>

#if LUT_SIZE == 0U || LUT_SIZE > 65535U
#error "Invalid sample count for mathematical generator"
#endif

#define OUTPUT_MIN_VOLTAGE (-5.0f)
#define OUTPUT_MAX_VOLTAGE (5.0f)
#define DAC_MAX_VALUE      (4095.0f)
#define OUTPUT_RANGE       (10.0f)

// Private generator configuration
static Waveform_t current_waveform = WAVEFORM_DC;
static float current_amplitude = 0.0f;
static float current_offset = 0.0f;

// Private generation state
static uint16_t sample_index = 0U;
static bool math_configured = false;


// --------------------------------------------------
// Validate waveform type
// --------------------------------------------------

static bool GeneratorMath_ValidWaveform(Waveform_t waveform)
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


// --------------------------------------------------
// Convert output voltage to DAC code
// --------------------------------------------------

static uint16_t GeneratorMath_VoltageToDAC(float voltage)
{
    if (voltage <= OUTPUT_MIN_VOLTAGE)
    {
        return 0U;
    }

    if (voltage >= OUTPUT_MAX_VOLTAGE)
    {
        return 4095U;
    }

    float dac_value =
        ((voltage - OUTPUT_MIN_VOLTAGE) / OUTPUT_RANGE)
        * DAC_MAX_VALUE;

    return (uint16_t)(dac_value + 0.5f);
}


// --------------------------------------------------
// Calculate next sample in real time
// This function runs in interrupt context
// --------------------------------------------------

static uint16_t GeneratorMath_NextSample(void)
{
    // Calculate normalized phase
    float phase =
        (float)sample_index / (float)LUT_SIZE;

    // Evaluate waveform equation
    float normalized =
        Waveform_Evaluate(current_waveform, phase);

    // Apply amplitude and DC offset
    float voltage =
        current_amplitude * normalized + current_offset;

    // Convert output voltage to DAC code
    uint16_t dac_code =
        GeneratorMath_VoltageToDAC(voltage);

    // Advance sample position
    sample_index++;

    if (sample_index >= LUT_SIZE)
    {
        sample_index = 0U;
    }

    return dac_code;
}


// --------------------------------------------------
// Initialize mathematical generator
// --------------------------------------------------

void GeneratorMath_Init(void)
{
    WaveOutput_Stop();

    current_waveform = WAVEFORM_DC;
    current_amplitude = 0.0f;
    current_offset = 0.0f;

    sample_index = 0U;
    math_configured = false;
}


// --------------------------------------------------
// Configure waveform parameters
// --------------------------------------------------

bool GeneratorMath_Configure(Waveform_t waveform,
                             float amplitude,
                             float offset)
{
    // Validate waveform
    if (!GeneratorMath_ValidWaveform(waveform))
    {
        return false;
    }

    // Reject NaN and infinity
    if (!isfinite(amplitude) || !isfinite(offset))
    {
        return false;
    }

    // Validate individual ranges
    if (amplitude < 0.0f || amplitude > 5.0f ||
        offset < -5.0f || offset > 5.0f)
    {
        return false;
    }

    // Validate combined voltage range
    if ((offset + amplitude > OUTPUT_MAX_VOLTAGE) ||
        (offset - amplitude < OUTPUT_MIN_VOLTAGE))
    {
        return false;
    }

    // Stop before changing parameters used by the ISR
    WaveOutput_Stop();

    current_waveform = waveform;
    current_amplitude = amplitude;
    current_offset = offset;

    sample_index = 0U;
    math_configured = true;

    return true;
}


// --------------------------------------------------
// Start real-time mathematical generation
// --------------------------------------------------

bool GeneratorMath_Start(float frequency)
{
    if (!math_configured)
    {
        return false;
    }

    if (!isfinite(frequency) || frequency < GENERATOR_MIN_FREQUENCY_HZ ||
        frequency > GENERATOR_MAX_FREQUENCY_HZ)
    {
        return false;
    }

    // Same nominal sample count as LUT mode
    double sample_rate =
        (double)frequency * (double)LUT_SIZE;

    // Protect conversion to uint32_t
    if (!isfinite(sample_rate) ||
        sample_rate < 1.0 ||
        sample_rate > ((double)UINT32_MAX - 0.5))
    {
        return false;
    }

    uint32_t sample_rate_hz =
        (uint32_t)(sample_rate + 0.5);

    // Stop before resetting state used in interrupt context
    WaveOutput_Stop();
    sample_index = 0U;

    // WaveOutput controls TIM6, DAC and interrupts
    return WaveOutput_StartRealtime(
        GeneratorMath_NextSample,
        sample_rate_hz
    );
}


// --------------------------------------------------
// Stop mathematical generation
// --------------------------------------------------

void GeneratorMath_Stop(void)
{
    WaveOutput_Stop();
}
