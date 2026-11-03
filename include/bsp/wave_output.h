#ifndef WAVE_OUTPUT_H
#define WAVE_OUTPUT_H
#include <stdint.h>
#include <stdbool.h>

/* Existing callbacks/APIs retained for backwards compatibility. */
typedef uint16_t (*WaveOutput_SampleProvider)(void);
typedef void (*WaveOutput_RefillCallback)(uint16_t *target, uint16_t count);

void WaveOutput_Init(void);
bool WaveOutput_StartBuffer(const uint16_t *buffer,
                            uint16_t length, uint32_t sampleRate);
bool WaveOutput_StartRealtime(WaveOutput_SampleProvider provider,
                              uint32_t sampleRate);

/* New: 2-half circular DMA buffer. Caller pre-fills buffer before start.
 * Refill callback is invoked in DMA IRQ, must be bounded, nonblocking,
 * and write ONLY the indicated inactive half.
 */
bool WaveOutput_StartStreaming(uint16_t *buffer, uint16_t length,
                               uint32_t sampleRate,
                               WaveOutput_RefillCallback refill);

void WaveOutput_SetDC(uint16_t value);
void WaveOutput_Stop(void);
bool WaveOutput_IsRunning(void);
bool WaveOutput_HasError(void);
#endif
