#ifndef WAVEFORM_H
#define WAVEFORM_H

typedef enum
{
    WAVEFORM_DC,
    WAVEFORM_SINE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SQUARE

} Waveform_t;

/*
 * Evaluates a normalized waveform.
 *
 * phase: normalized phase [0, 1)
 * return: normalized amplitude [-1, 1]
 */
float Waveform_Evaluate(
    Waveform_t waveform,
    float phase
);

#endif