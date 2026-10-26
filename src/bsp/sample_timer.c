#include "bsp/sample_timer.h"
#include "config/board_config.h"
#include "stm32f446xx.h"
#include <stddef.h>

static SampleTimer_Callback update_callback = NULL;

void SampleTimer_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->CR1 &= ~TIM_CR1_CEN;
    TIM6->DIER &= ~TIM_DIER_UIE;
    TIM6->PSC = 0U;
    TIM6->ARR = 0xFFFFU;
    TIM6->CR2 = (TIM6->CR2 & ~TIM_CR2_MMS_Msk) | TIM_CR2_MMS_1;
    TIM6->EGR = TIM_EGR_UG;
    TIM6->SR &= ~TIM_SR_UIF;
    NVIC_DisableIRQ(TIM6_DAC_IRQn);
    NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
    update_callback = NULL;
}

bool SampleTimer_Set(uint32_t sampleFrequency_hz)
{
    if (sampleFrequency_hz == 0U) return false;
    /* Round nearest timer interval, with 16-bit ARR+1 in [1, 65536]. */
    const uint32_t ticks = (uint32_t)(
        ((uint64_t)APB1_TIMER_CLOCK_HZ + sampleFrequency_hz / 2U) /
        sampleFrequency_hz);
    if (ticks < 2U || ticks > 65536U) return false;
    TIM6->PSC = 0U;
    TIM6->ARR = ticks - 1U;
    TIM6->CNT = 0U;
    TIM6->EGR = TIM_EGR_UG;
    TIM6->SR &= ~TIM_SR_UIF;
    return true;
}
void SampleTimer_Start(void) { TIM6->CR1 |= TIM_CR1_CEN; }
void SampleTimer_Stop(void)  { TIM6->CR1 &= ~TIM_CR1_CEN; }
void SampleTimer_SetUpdateCallback(SampleTimer_Callback cb)
{
    update_callback = cb;
}
void SampleTimer_EnableUpdateIRQ(void)
{
    TIM6->SR &= ~TIM_SR_UIF;
    NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
    NVIC_SetPriority(TIM6_DAC_IRQn, 5U);
    TIM6->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
}
void SampleTimer_DisableUpdateIRQ(void)
{
    TIM6->DIER &= ~TIM_DIER_UIE;
    NVIC_DisableIRQ(TIM6_DAC_IRQn);
    NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
    TIM6->SR &= ~TIM_SR_UIF;
}
void TIM6_DAC_IRQHandler(void)
{
    if (((TIM6->SR & TIM_SR_UIF) != 0U) &&
        ((TIM6->DIER & TIM_DIER_UIE) != 0U))
    {
        TIM6->SR &= ~TIM_SR_UIF;
        if (update_callback != NULL) update_callback();
    }
    /* DAC DMA underrun interrupt remains disabled in this version. */
}
