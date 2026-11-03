#include "app/parameter_manager.h"
#include "generator/generator.h"
#include "utils/filter.h"
#include "utils/map.h"
#include "config/generator_config.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define PM_ADC_MAX            4095U
#define PM_FREQ_FILTER_ALPHA  0.65f
#define PM_FREQ_DEADBAND      5U
#define PM_AMPLITUDE_STEP_V    0.1f
#define PM_OFFSET_STEP_V       0.1f
#define PM_MAX_PEAK_V          5.0f
#define PM_FILTER_ALPHA       0.18f
#define PM_STABLE_READINGS     3U
#define PM_ENCODER_COUNTS_PER_STEP 4

typedef struct
{
    float frequency_hz;
    float amplitude_v;
    float offset_v;
    Waveform_t waveform;
} ParameterManager_Settings_t;

static FilterIIR_t frequency_filter;
static FilterIIR_t amplitude_filter;
static FilterIIR_t offset_filter;

static ParameterManager_Settings_t pending_settings;
static ParameterManager_Settings_t applied_settings;
static bool pending_valid = false;
static bool applied_valid = false;
static uint8_t pending_count = 0U;

static uint8_t selected_waveform_index = 1U; /* SINE initially */
static int32_t encoder_remainder = 0;

static Waveform_t ParameterManager_WaveformFromIndex(uint8_t index)
{
    switch (index)
    {
        case 0U: return WAVEFORM_DC;
        case 1U: return WAVEFORM_SINE;
        case 2U: return WAVEFORM_TRIANGLE;
        default: return WAVEFORM_SQUARE;
    }
}

static bool ParameterManager_Equals(const ParameterManager_Settings_t *a,
                                    const ParameterManager_Settings_t *b)
{
    return a->waveform == b->waveform &&
           a->frequency_hz == b->frequency_hz &&
           a->amplitude_v == b->amplitude_v &&
           a->offset_v == b->offset_v;
}

static void ParameterManager_AdvanceWaveform(int32_t delta)
{
    /* Clip a single (potentially corrupted) reading to bound arithmetic. */
    if (delta > 128)  delta = 128;
    if (delta < -128) delta = -128;

    const int32_t accumulated = encoder_remainder + delta;
    const int32_t detents = accumulated / PM_ENCODER_COUNTS_PER_STEP;
    encoder_remainder = accumulated % PM_ENCODER_COUNTS_PER_STEP;

    /* C remainder can be negative: normalize into [0,3]. */
    const int32_t next = (int32_t)selected_waveform_index + (detents % 4);
    selected_waveform_index = (uint8_t)((next + 4) % 4);
}

/* Interpolate between decade anchors. All changes happen in foreground. */
static float FrequencyFromCode(uint16_t code)
{
    static const float hz[] = { GENERATOR_MIN_FREQUENCY_HZ, 10.0f, 100.0f,
                                1000.0f, 10000.0f, GENERATOR_MAX_FREQUENCY_HZ };
    if (code >= PM_ADC_MAX) return GENERATOR_MAX_FREQUENCY_HZ;
    const unsigned segment = code / 819U;
    const float fraction = (float)(code % 819U) / 819.0f;
    const float value = hz[segment] + fraction * (hz[segment + 1U] - hz[segment]);
    static const float step[] = { 1.0f, 1.0f, 10.0f, 100.0f, 250.0f };
    return Map_Quantize(value, step[segment]);
}

/* Latch a filtered ADC position. Snap the rails so filter settling and
   the deadband cannot prevent selecting either endpoint. */
static uint16_t frequency_anchor;
static bool frequency_anchor_valid;

static float LocalFrequency(float filtered)
{
    uint16_t code = (uint16_t)fminf(filtered + 0.5f, (float)PM_ADC_MAX);
    if (code <= 1U) code = 0U;
    if (code >= PM_ADC_MAX - 1U) code = PM_ADC_MAX;

    if (!frequency_anchor_valid || code == 0U || code == PM_ADC_MAX ||
        abs((int)code - (int)frequency_anchor) >= (int)PM_FREQ_DEADBAND)
    {
        frequency_anchor = code;
        frequency_anchor_valid = true;
    }
    return FrequencyFromCode(frequency_anchor);
}

static ParameterManager_Settings_t ParameterManager_FromRaw(
    const ControlsRaw_t *raw)
{
    const float raw_freq = FilterIIR_Update(
        &frequency_filter, (float)raw->frequency_adc, PM_FREQ_FILTER_ALPHA);
    const float raw_amp = FilterIIR_Update(
        &amplitude_filter, (float)raw->amplitude_adc, PM_FILTER_ALPHA);
    const float raw_offset = FilterIIR_Update(
        &offset_filter, (float)raw->offset_adc, PM_FILTER_ALPHA);

    ParameterManager_Settings_t settings;
    settings.frequency_hz = LocalFrequency(raw_freq);

    settings.offset_v = Map_Quantize(
        Map_LinearU16((uint16_t)raw_offset, PM_ADC_MAX,
                      -PM_MAX_PEAK_V, PM_MAX_PEAK_V),
        PM_OFFSET_STEP_V);

    /* Requested amplitude cannot exceed the available headroom. */
    const float requested_amplitude = Map_Quantize(
        Map_LinearU16((uint16_t)raw_amp, PM_ADC_MAX,
                      0.0f, PM_MAX_PEAK_V),
        PM_AMPLITUDE_STEP_V);

    const float headroom = PM_MAX_PEAK_V - fabsf(settings.offset_v);
    settings.amplitude_v = fminf(requested_amplitude, headroom);

    settings.waveform = ParameterManager_WaveformFromIndex(
        selected_waveform_index);

    return settings;
}

static bool ParameterManager_Apply(const ParameterManager_Settings_t *settings)
{
    /* The public Generator setters each stop output when a value changes.
       Set amplitude=0 first to keep all intermediate configurations valid
       when offset and amplitude change in the same update. */
    Generator_Stop();
    if (!Generator_SetAmplitude(0.0f) ||
        !Generator_SetOffset(settings->offset_v) ||
        !Generator_SetAmplitude(settings->amplitude_v) ||
        !Generator_SetFrequency(settings->frequency_hz) ||
        !Generator_SetWaveform(settings->waveform))
    {
        return false;
    }

    return Generator_Start();
}

void ParameterManager_Init(void)
{
    frequency_anchor_valid = false;
    FilterIIR_Reset(&frequency_filter);
    FilterIIR_Reset(&amplitude_filter);
    FilterIIR_Reset(&offset_filter);

    pending_valid = false;
    applied_valid = false;
    pending_count = 0U;
    encoder_remainder = 0;
    selected_waveform_index = 1U;
}

bool ParameterManager_Update(const ControlsRaw_t *raw)
{
    if (raw == NULL)
    {
        return false;
    }

    ParameterManager_AdvanceWaveform(raw->encoder_delta);
    const ParameterManager_Settings_t next = ParameterManager_FromRaw(raw);

    if (!pending_valid ||
        !ParameterManager_Equals(&next, &pending_settings))
    {
        pending_settings = next;
        pending_valid = true;
        pending_count = 1U;
        return true;
    }

    if (pending_count < PM_STABLE_READINGS)
    {
        ++pending_count;
    }

    if (pending_count < PM_STABLE_READINGS ||
        (applied_valid &&
         ParameterManager_Equals(&pending_settings, &applied_settings)))
    {
        return true;
    }

    if (!ParameterManager_Apply(&pending_settings))
    {
        return false;
    }

    applied_settings = pending_settings;
    applied_valid = true;
    return true;
}
