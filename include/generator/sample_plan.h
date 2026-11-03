#ifndef SAMPLE_PLAN_H
#define SAMPLE_PLAN_H
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t samples_per_cycle;
    uint32_t sample_rate_hz;
} SamplePlan_t;

/* Select highest power-of-two samples per cycle in [16, 256] subject
 * to sample_rate <= 800 kS/s. Shared by LUT and MATH.
 * No phase accumulator/DDS in this version.
 */
bool SamplePlan_Make(float frequency_hz, SamplePlan_t *out);

#endif
