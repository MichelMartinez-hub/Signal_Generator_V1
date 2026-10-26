#include "utils/filter.h"
#include <stddef.h>

void FilterIIR_Reset(FilterIIR_t *filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->state = 0.0f;
    filter->initialized = false;
}

float FilterIIR_Update(FilterIIR_t *filter, float input, float alpha)
{
    if (filter == NULL)
    {
        return input;
    }

    if (!filter->initialized)
    {
        filter->state = input;
        filter->initialized = true;
        return input;
    }

    if (alpha < 0.0f)
    {
        alpha = 0.0f;
    }
    else if (alpha > 1.0f)
    {
        alpha = 1.0f;
    }

    filter->state += alpha * (input - filter->state);
    return filter->state;
}
