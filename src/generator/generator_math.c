#include "generator/generator_math.h"
#include "generator/sample_plan.h"
#include "config/board_config.h"
#include "bsp/wave_output.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#if (MATH_DMA_BUFFER_SIZE != 256U)
#error "This implementation uses a 256-sample DMA buffer"
#endif

#define OUTPUT_MIN_V (-5.0f)
#define OUTPUT_MAX_V ( 5.0f)
#define PI2          (6.2831853071795864769f)

static Waveform_t waveform = WAVEFORM_DC;
static float amplitude = 0.0f;
static float offset = 0.0f;
static bool configured = false;

/* All mutable sample state is private to the math engine. */
static uint16_t samples_per_cycle = LUT_SIZE;
static uint16_t phase_index = 0U;
static float previous = 0.0f;
static float current = 0.0f;
static float sine_coefficient = 0.0f;
static float sine_first = 0.0f;
static uint16_t dma_buffer[MATH_DMA_BUFFER_SIZE];

static bool ValidWaveform(Waveform_t w)
{
    return w == WAVEFORM_DC || w == WAVEFORM_SINE ||
           w == WAVEFORM_TRIANGLE || w == WAVEFORM_SQUARE;
}

static uint16_t VoltageToDAC(float voltage)
{
    if (voltage <= OUTPUT_MIN_V) return 0U;
    if (voltage >= OUTPUT_MAX_V) return 4095U;
    return (uint16_t)((voltage + 5.0f) * 409.5f + 0.5f);
}

/* Numerical sine oscillator: y[n+1] = 2cos(w)*y[n] - y[n-1].
 * Reset at each whole cycle to prevent cumulative roundoff drift.
 * sinf/cosf are used only at START, never inside a DMA IRQ.
 */
static float NextNormalized(void)
{
    float value;
    const uint16_t i = phase_index;
    switch (waveform)
    {
        case WAVEFORM_SINE:
            if (i == 0U)
            {
                previous = -sine_first;
                current = 0.0f;
                value = 0.0f;
            }
            else
            {
                const float next = sine_coefficient * current - previous;
                previous = current;
                current = next;
                value = next;
            }
            break;
        case WAVEFORM_TRIANGLE:
            if (i < samples_per_cycle / 2U)
                value = (4.0f * (float)i / (float)samples_per_cycle) - 1.0f;
            else
                value = 3.0f - (4.0f * (float)i / (float)samples_per_cycle);
            break;
        case WAVEFORM_SQUARE:
            value = i < samples_per_cycle / 2U ? 1.0f : -1.0f;
            break;
        default:
            value = 0.0f;
            break;
    }
    ++phase_index;
    if (phase_index >= samples_per_cycle) phase_index = 0U;
    return value;
}

static void FillBlock(uint16_t *dest, uint16_t count)
{
    for (uint16_t i = 0U; i < count; ++i)
    {
        const float normalized = NextNormalized();
        dest[i] = VoltageToDAC(offset + amplitude * normalized);
    }
}

void GeneratorMath_Init(void)
{
    WaveOutput_Stop();
    waveform = WAVEFORM_DC;
    amplitude = 0.0f;
    offset = 0.0f;
    samples_per_cycle = LUT_SIZE;
    phase_index = 0U;
    configured = false;
}

bool GeneratorMath_Configure(Waveform_t shape, float peak, float dc_offset)
{
    if (!ValidWaveform(shape) || !isfinite(peak) || !isfinite(dc_offset) ||
        peak < 0.0f || peak > 5.0f ||
        dc_offset < -5.0f || dc_offset > 5.0f ||
        dc_offset + peak > OUTPUT_MAX_V ||
        dc_offset - peak < OUTPUT_MIN_V)
        return false;
    WaveOutput_Stop();
    waveform = shape;
    amplitude = peak;
    offset = dc_offset;
    configured = true;
    phase_index = 0U;
    return true;
}

bool GeneratorMath_Start(float frequency)
{
    if (!configured) return false;
    SamplePlan_t plan;
    if (!SamplePlan_Make(frequency, &plan)) return false;

    WaveOutput_Stop();
    samples_per_cycle = plan.samples_per_cycle;
    phase_index = 0U;
    const float angle = PI2 / (float)samples_per_cycle;
    sine_first = sinf(angle);
    sine_coefficient = 2.0f * cosf(angle);
    previous = -sine_first;
    current = 0.0f;

    /* Initial block must be completely ready before enabling DMA. */
    FillBlock(dma_buffer, MATH_DMA_BUFFER_SIZE);
    return WaveOutput_StartStreaming(dma_buffer, MATH_DMA_BUFFER_SIZE,
                                     plan.sample_rate_hz, FillBlock);
}

void GeneratorMath_Stop(void) { WaveOutput_Stop(); }
