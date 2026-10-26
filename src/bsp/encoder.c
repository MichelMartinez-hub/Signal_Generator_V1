#include "bsp/encoder.h"
#include "stm32f446xx.h"

static uint32_t previous_count = 0U;

void Encoder_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1 &= ~TIM_CR1_CEN;

    /* PA0, PA1 -> alternate function AF1 (TIM2 CH1/CH2). */
    GPIOA->MODER = (GPIOA->MODER & ~((3UL << 0U) | (3UL << 2U)))
                 | (2UL << 0U) | (2UL << 2U);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~((15UL << 0U) | (15UL << 4U)))
                  | (1UL << 0U) | (1UL << 4U);
    GPIOA->PUPDR = (GPIOA->PUPDR & ~((3UL << 0U) | (3UL << 2U)))
                 | (1UL << 0U) | (1UL << 2U); /* pull-ups */

    TIM2->PSC = 0U;
    TIM2->ARR = 0xFFFFFFFFUL;
    TIM2->CNT = 0U;

    /* CC1S=01, CC2S=01. IC1F/IC2F=1111 digital input filters. */
    TIM2->CCMR1 = (1UL << TIM_CCMR1_CC1S_Pos)
                | (1UL << TIM_CCMR1_CC2S_Pos)
                | (15UL << TIM_CCMR1_IC1F_Pos)
                | (15UL << TIM_CCMR1_IC2F_Pos);

    /* Non-inverted inputs, both channels enabled. */
    TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;

    /* SMS=011: encoder mode 3, counts on TI1 and TI2 edges. */
    TIM2->SMCR = (TIM2->SMCR & ~TIM_SMCR_SMS_Msk)
               | (3UL << TIM_SMCR_SMS_Pos);

    previous_count = TIM2->CNT;
    TIM2->CR1 |= TIM_CR1_CEN;
}

int32_t Encoder_ReadDelta(void)
{
    uint32_t current = TIM2->CNT;
    uint32_t difference = current - previous_count;
    previous_count = current;

    /* Modular subtraction also handles a wrap-around on the 32-bit timer.
       Assumes < 2^31 encoder counts between polls (ARM GCC signed cast). */
    return (int32_t)difference;
}
