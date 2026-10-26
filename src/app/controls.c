#include "app/controls.h"
#include "bsp/adc.h"
#include "bsp/encoder.h"
#include <stddef.h>

#define FREQUENCY_ADC_CHANNEL 10U  /* PC0 */
#define AMPLITUDE_ADC_CHANNEL 11U  /* PC1 */
#define OFFSET_ADC_CHANNEL     8U  /* PB0 */

void Controls_Init(void)
{
    ADC_Init();
    Encoder_Init();
}

bool Controls_ReadRaw(ControlsRaw_t *out)
{
    if (out == NULL)
    {
        return false;
    }

    ControlsRaw_t sample = {0};

    if (!ADC_ReadChannel(FREQUENCY_ADC_CHANNEL, &sample.frequency_adc) ||
        !ADC_ReadChannel(AMPLITUDE_ADC_CHANNEL, &sample.amplitude_adc) ||
        !ADC_ReadChannel(OFFSET_ADC_CHANNEL, &sample.offset_adc))
    {
        return false;
    }

    sample.encoder_delta = Encoder_ReadDelta();
    *out = sample;
    return true;
}