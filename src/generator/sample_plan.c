#include "generator/sample_plan.h"
#include "config/board_config.h"
#include "config/generator_config.h"
#include <math.h>
#include <stddef.h>

#if (LUT_SIZE != 256U) || (GENERATOR_MIN_SAMPLES_PER_CYCLE != 16U)
#error "Sample plan expects 256-entry tables and minimum 16 samples"
#endif

bool SamplePlan_Make(float frequency_hz, SamplePlan_t *out)
{
    if (out == NULL || !isfinite(frequency_hz) ||
        frequency_hz < GENERATOR_MIN_FREQUENCY_HZ ||
        frequency_hz > GENERATOR_MAX_FREQUENCY_HZ)
        return false;

    for (uint16_t n = LUT_SIZE; n >= GENERATOR_MIN_SAMPLES_PER_CYCLE; n >>= 1U)
    {
        const double rate = (double)frequency_hz * (double)n;
        if (rate > 0.0 && rate <= (double)GENERATOR_SAMPLE_RATE_MAX_HZ)
        {
            const uint32_t rounded = (uint32_t)(rate + 0.5);
            if (rounded == 0U || rounded > GENERATOR_SAMPLE_RATE_MAX_HZ)
                return false;
            out->samples_per_cycle = n;
            out->sample_rate_hz = rounded;
            return true;
        }
    }
    return false;
}
