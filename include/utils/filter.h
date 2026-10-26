#ifndef FILTER_H
#define FILTER_H

#include <stdbool.h>

typedef struct
{
    float state;
    bool initialized;
} FilterIIR_t;

void FilterIIR_Reset(FilterIIR_t *filter);
float FilterIIR_Update(FilterIIR_t *filter, float input, float alpha);

#endif /* FILTER_H */
