#include "bsp/dac.h"
#include "config/board_config.h"
#include "stm32f446xx.h"

void DAC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~DAC_CR_EN1;
    DAC->CR &= ~(DAC_CR_BOFF1 | DAC_CR_TEN1 | DAC_CR_DMAEN1 |
                 DAC_CR_TSEL1_Msk | DAC_CR_DMAUDRIE1);
    DAC->CR |= DAC_CR_EN1; /* BOFF1=0: internal output buffer ON */
    DAC_SetValue(DAC_MIDSCALE_CODE);
    DAC_ClearUnderrun();
}

void DAC_SetValue(uint16_t value)
{
    if (value > DAC_MAX_CODE) value = DAC_MAX_CODE;
    DAC->DHR12R1 = (uint32_t)value;
}

void DAC_EnableTrigger(void)
{
    DAC->CR &= ~DAC_CR_TSEL1_Msk; /* 000: TIM6 TRGO */
    DAC->CR |= DAC_CR_TEN1;
}
void DAC_DisableTrigger(void) { DAC->CR &= ~DAC_CR_TEN1; }
void DAC_EnableDMA(void)     { DAC->CR |= DAC_CR_DMAEN1; }
void DAC_DisableDMA(void)    { DAC->CR &= ~DAC_CR_DMAEN1; }
void DAC_ClearUnderrun(void) { DAC->SR = DAC_SR_DMAUDR1; } /* W1C */
