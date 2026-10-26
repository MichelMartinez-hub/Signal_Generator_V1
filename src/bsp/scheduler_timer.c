#include "bsp/scheduler_timer.h"
#include "config/board_config.h"
#include "stm32f446xx.h"

/* Prescale the APB1 timer clock to 10 kHz, then to 100 Hz. */
#define TIM3_COUNTER_HZ 10000UL
#define TIM3_UPDATE_HZ  (1000UL / SCHEDULER_TIMER_TICK_MS)

#if (1000UL % SCHEDULER_TIMER_TICK_MS) != 0UL
#error "Scheduler timer tick must divide 1000 ms exactly"
#endif

#if (TIM3_COUNTER_HZ % TIM3_UPDATE_HZ) != 0UL
#error "TIM3 counter rate must be divisible by update rate"
#endif

#if (APB1_TIMER_CLOCK_HZ < TIM3_COUNTER_HZ) || \
    ((APB1_TIMER_CLOCK_HZ % TIM3_COUNTER_HZ) != 0UL)
#error "APB1_TIMER_CLOCK_HZ must be a multiple of 10000 Hz"
#endif

#if ((APB1_TIMER_CLOCK_HZ / TIM3_COUNTER_HZ) > 65536UL)
#error "TIM3 prescaler does not fit in 16 bits"
#endif

static volatile uint32_t tick_count = 0U;

void SchedulerTimer_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;

    TIM3->CR1 &= ~TIM_CR1_CEN;
    TIM3->DIER &= ~TIM_DIER_UIE;
    NVIC_DisableIRQ(TIM3_IRQn);

    TIM3->PSC = (APB1_TIMER_CLOCK_HZ / TIM3_COUNTER_HZ) - 1UL;
    TIM3->ARR = (TIM3_COUNTER_HZ / TIM3_UPDATE_HZ) - 1UL;
    TIM3->CNT = 0U;
    TIM3->CR1 |= TIM_CR1_ARPE;

    /* Load PSC/ARR while interrupts are disabled; UG sets UIF. */
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR &= ~TIM_SR_UIF;
    NVIC_ClearPendingIRQ(TIM3_IRQn);

    tick_count = 0U;
    NVIC_SetPriority(TIM3_IRQn, 6U);
    TIM3->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM3_IRQn);
}

void SchedulerTimer_Start(void)
{
    TIM3->CR1 &= ~TIM_CR1_CEN;
    TIM3->CNT = 0U;
    TIM3->SR &= ~TIM_SR_UIF;
    NVIC_ClearPendingIRQ(TIM3_IRQn);
    TIM3->CR1 |= TIM_CR1_CEN;
}

void SchedulerTimer_Stop(void)
{
    TIM3->CR1 &= ~TIM_CR1_CEN;
    TIM3->SR &= ~TIM_SR_UIF;
    NVIC_ClearPendingIRQ(TIM3_IRQn);
}

uint32_t SchedulerTimer_GetTicks(void)
{
    return tick_count;
}

void TIM3_IRQHandler(void)
{
    if (((TIM3->SR & TIM_SR_UIF) != 0U) &&
        ((TIM3->DIER & TIM_DIER_UIE) != 0U))
    {
        TIM3->SR &= ~TIM_SR_UIF;
        ++tick_count;
    }
}
