#include "generator/waveform.h"
#include <math.h>

#define TWO_PI 6.28318530718f

float Waveform_Evaluate(Waveform_t waveform,
                                float phase)
{
    switch (waveform)
    {
        case WAVEFORM_DC:
            // Return normalized DC level
            return 0.0f;

        case WAVEFORM_SINE:
            // Calculate sine wave
            return sinf(TWO_PI * phase);
        

        case WAVEFORM_TRIANGLE:
            // Calculate triangular wave
            if (phase < 0.5f)
            {
                return 4.0f * phase - 1.0f;
            }
            else
            {
                return 3.0f - 4.0f * phase;
            }
            break;

        case WAVEFORM_SQUARE:
            // Calculate square wave
            if (phase < 0.5f)
            {
                return 1.0f;
            }
            else
            {
                return -1.0f;
            }
            break;

        default:
            return 0.0f;
    }

    return 0.0f;
}