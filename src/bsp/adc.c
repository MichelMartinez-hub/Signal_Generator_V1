#include "bsp/adc.h"
#include "stm32f446xx.h"
#include <stddef.h>

#define ADC_TIMEOUT_ITERATIONS 100000UL

void ADC_Init(void)
{
    /* PC0 = ADC1_IN10, PC1 = ADC1_IN11, PB0 = ADC1_IN8. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    GPIOC->MODER |= (3UL << (0U * 2U)) | (3UL << (1U * 2U));
    GPIOC->PUPDR &= ~((3UL << (0U * 2U)) | (3UL << (1U * 2U)));
    GPIOB->MODER |= (3UL << (0U * 2U));
    GPIOB->PUPDR &= ~(3UL << (0U * 2U));

    ADC1->CR2 = 0U; /* ADC off during configuration. */
    ADC1->CR1 = 0U; /* 12-bit, no scan, no interrupts. */

    /* ADCCLK = PCLK2 / 4. */
    ADC->CCR = (ADC->CCR & ~ADC_CCR_ADCPRE_Msk)
             | (1UL << ADC_CCR_ADCPRE_Pos);

    /* Single regular conversion (sequence length = 1). */
    ADC1->SQR1 &= ~ADC_SQR1_L_Msk;
    ADC1->SQR2 = 0U;
    ADC1->SQR3 = 0U;

    /* Long sample time: 480 ADC cycles for high-impedance pots. */
    ADC1->SMPR1 = (ADC1->SMPR1 & ~((7UL << 0U) | (7UL << 3U)))
                | (7UL << 0U) | (7UL << 3U); /* CH10, CH11 */
    ADC1->SMPR2 = (ADC1->SMPR2 & ~(7UL << 24U))
                | (7UL << 24U);             /* CH8 */

    /* Software trigger, right alignment, single conversion. */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Allow analog ADC circuitry to stabilize; project runs at HSI 16 MHz. */
    for (volatile uint32_t i = 0U; i < 256U; ++i)
    {
        __NOP();
    }
}

bool ADC_ReadChannel(uint8_t channel, uint16_t *result)
{
    if (result == NULL ||
        (channel != 8U && channel != 10U && channel != 11U))
    {
        return false;
    }

    /* The ADC is owned by this driver; callers must not use it concurrently. */
    ADC1->SQR3 = (uint32_t)channel; /* SQ1 is bits [4:0]. */
    ADC1->SR &= ~(ADC_SR_EOC | ADC_SR_OVR);
    ADC1->CR2 |= ADC_CR2_SWSTART;

    uint32_t timeout = ADC_TIMEOUT_ITERATIONS;
    while ((ADC1->SR & ADC_SR_EOC) == 0U)
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }

    *result = (uint16_t)(ADC1->DR & 0x0FFFU);
    return true;
}
