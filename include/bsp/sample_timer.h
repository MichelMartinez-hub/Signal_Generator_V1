#ifndef SAMPLE_TIMER_H
#define SAMPLE_TIMER_H

#include <stdint.h>
#include <stdbool.h>

// Original API
void SampleTimer_Init(void);
bool SampleTimer_Set(uint32_t sampleFrequency_hz);
void SampleTimer_Start(void);
void SampleTimer_Stop(void);

// New API: interrupt support
typedef void (*SampleTimer_Callback)(void);

void SampleTimer_SetUpdateCallback(
    SampleTimer_Callback callback
);

void SampleTimer_EnableUpdateIRQ(void);
void SampleTimer_DisableUpdateIRQ(void);

#endif // SAMPLE_TIMER_H