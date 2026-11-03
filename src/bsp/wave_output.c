#include "bsp/wave_output.h"
#include "bsp/dac.h"
#include "bsp/dma.h"
#include "bsp/sample_timer.h"
#include "config/board_config.h"
#include <stddef.h>

typedef enum { OUTPUT_IDLE, OUTPUT_LUT, OUTPUT_ISR, OUTPUT_STREAM } OutputMode_t;
static volatile OutputMode_t output_mode = OUTPUT_IDLE;
static volatile bool output_error = false;
static WaveOutput_SampleProvider sample_provider = NULL;
static WaveOutput_RefillCallback refill_callback = NULL;
static uint16_t *stream_buffer = NULL;
static uint16_t stream_length = 0U;

static void WaveOutput_OnTimerUpdate(void)
{
    if (output_mode == OUTPUT_ISR && sample_provider != NULL)
        DAC_SetValue(sample_provider());
}

void WaveOutput_Stop(void)
{
    SampleTimer_Stop();
    SampleTimer_DisableUpdateIRQ();
    SampleTimer_SetUpdateCallback(NULL);
    /* Disable requests before disabling DMA to avoid a new request. */
    DAC_DisableDMA();
    DAC_DisableTrigger();
    DMA_StopDac();
    DAC_ClearUnderrun();
    sample_provider = NULL;
    refill_callback = NULL;
    stream_buffer = NULL;
    stream_length = 0U;
    DAC_SetValue(DAC_MIDSCALE_CODE);
    output_mode = OUTPUT_IDLE;
}

void WaveOutput_Init(void)
{
    DAC_Init();
    DMA_Init();
    SampleTimer_Init();
    WaveOutput_Stop();
    output_error = false;
}

static void WaveOutput_OnDMA(DMA_DAC_Event_t event)
{
    if (event == DMA_DAC_ERROR || output_mode != OUTPUT_STREAM ||
        stream_buffer == NULL || refill_callback == NULL)
    {
        WaveOutput_Stop();
        output_error = true;
        return;
    }
    const uint16_t half = (uint16_t)(stream_length / 2U);
    uint16_t *target = (event == DMA_DAC_HALF) ? stream_buffer : (stream_buffer + half);
    refill_callback(target, half);
}

bool WaveOutput_StartBuffer(const uint16_t *buffer, uint16_t length,
                            uint32_t sampleRate)
{
    if (buffer == NULL || length == 0U || sampleRate == 0U) return false;
    WaveOutput_Stop();
    output_error = false;
    if (!SampleTimer_Set(sampleRate)) return false;
    DAC_EnableTrigger();
    DAC_SetValue(buffer[length - 1U]);
    if (!DMA_StartDac(buffer, length))
    {
        WaveOutput_Stop();
        return false;
    }
    DAC_EnableDMA();
    output_mode = OUTPUT_LUT;
    SampleTimer_Start();
    return true;
}

bool WaveOutput_StartStreaming(uint16_t *buffer, uint16_t length,
                               uint32_t sampleRate,
                               WaveOutput_RefillCallback refill)
{
    if (buffer == NULL || length < 4U || (length & 1U) != 0U ||
        sampleRate == 0U || refill == NULL) return false;
    WaveOutput_Stop();
    output_error = false;
    if (!SampleTimer_Set(sampleRate)) return false;
    stream_buffer = buffer;
    stream_length = length;
    refill_callback = refill;
    DAC_EnableTrigger();
    DAC_SetValue(buffer[length - 1U]);
    if (!DMA_StartDacWithCallback(buffer, length, WaveOutput_OnDMA))
    {
        WaveOutput_Stop();
        return false;
    }
    output_mode = OUTPUT_STREAM;
    DAC_EnableDMA();
    SampleTimer_Start();
    return true;
}

bool WaveOutput_StartRealtime(WaveOutput_SampleProvider provider,
                              uint32_t sampleRate)
{
    if (provider == NULL || sampleRate == 0U) return false;
    WaveOutput_Stop();
    output_error = false;
    if (!SampleTimer_Set(sampleRate)) return false;
    sample_provider = provider;
    DAC_EnableTrigger();
    DAC_SetValue(sample_provider());
    SampleTimer_SetUpdateCallback(WaveOutput_OnTimerUpdate);
    output_mode = OUTPUT_ISR;
    SampleTimer_EnableUpdateIRQ();
    SampleTimer_Start();
    return true;
}

void WaveOutput_SetDC(uint16_t value)
{
    WaveOutput_Stop();
    DAC_SetValue(value);
}

bool WaveOutput_IsRunning(void) { return output_mode != OUTPUT_IDLE; }
bool WaveOutput_HasError(void) { return output_error; }
