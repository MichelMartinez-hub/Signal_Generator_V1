#include "bsp/wave_output.h"
#include "bsp/dac.h"
#include "bsp/dma.h"
#include "bsp/sample_timer.h"
#include "config/board_config.h"
#include <stddef.h>

static bool output_running = false;
static WaveOutput_SampleProvider sample_provider = NULL;

static void WaveOutput_OnTimerUpdate(void)
{
    if (sample_provider != NULL)
    {
        DAC_SetValue(sample_provider());
    }
}

void WaveOutput_Stop(void)
{
    SampleTimer_Stop();
    SampleTimer_DisableUpdateIRQ();
    SampleTimer_SetUpdateCallback(NULL);
    DAC_DisableDMA();
    DAC_DisableTrigger();
    DMA_StopDac();
    DAC_ClearUnderrun();
    sample_provider = NULL;
    DAC_SetValue(DAC_MIDSCALE_CODE);
    output_running = false;
}

void WaveOutput_Init(void)
{
    DAC_Init();
    DMA_Init();
    SampleTimer_Init();
    WaveOutput_Stop();
}

bool WaveOutput_StartBuffer(const uint16_t *buffer,
                            uint16_t length,
                            uint32_t sampleRate)
{
    if (buffer == NULL || length == 0U || sampleRate == 0U)
        return false;

    WaveOutput_Stop();
    if (!SampleTimer_Set(sampleRate)) return false;
    DAC_EnableTrigger();
    DAC_SetValue(buffer[length - 1U]);
    if (!DMA_StartDac(buffer, length))
    {
        WaveOutput_Stop();
        return false;
    }
    DAC_EnableDMA();
    SampleTimer_Start();
    output_running = true;
    return true;
}

bool WaveOutput_StartRealtime(WaveOutput_SampleProvider provider,
                              uint32_t sampleRate)
{
    if (provider == NULL || sampleRate == 0U) return false;
    WaveOutput_Stop();
    if (!SampleTimer_Set(sampleRate)) return false;
    sample_provider = provider;
    DAC_EnableTrigger();
    DAC_SetValue(sample_provider());
    SampleTimer_SetUpdateCallback(WaveOutput_OnTimerUpdate);
    SampleTimer_EnableUpdateIRQ();
    SampleTimer_Start();
    output_running = true;
    return true;
}

void WaveOutput_SetDC(uint16_t value)
{
    WaveOutput_Stop();
    DAC_SetValue(value);
}

bool WaveOutput_IsRunning(void)
{
    return output_running;
}
