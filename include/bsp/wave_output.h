#ifndef WAVE_OUTPUT_H
#define WAVE_OUTPUT_H

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t (*WaveOutput_SampleProvider)(void);

void WaveOutput_Init(void);

bool WaveOutput_StartBuffer(
    const uint16_t *buffer,
    uint16_t length,
    uint32_t sampleRate
);

bool WaveOutput_StartRealtime(
    WaveOutput_SampleProvider provider,
    uint32_t sampleRate
);

void WaveOutput_SetDC(uint16_t value);
void WaveOutput_Stop(void);
bool WaveOutput_IsRunning(void);

#endif // WAVE_OUTPUT_H