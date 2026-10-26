#include "utils/map.h"
#include <math.h>

float Map_LinearU16(uint16_t input,
                    uint16_t input_max,
                    float output_min,
                    float output_max)
{
    if (input_max == 0U)
    {
        return output_min;
    }

    if (input > input_max)
    {
        input = input_max;
    }

    const float fraction = (float)input / (float)input_max;
    return output_min + fraction * (output_max - output_min);
}

float Map_Quantize(float value, float step)
{
    if (step <= 0.0f)
    {
        return value;
    }

    return roundf(value / step) * step;
}
