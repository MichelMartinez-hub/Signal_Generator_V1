#include "bsp/system.h"
#include "stm32f446xx.h"

void System_Init(void)
{
    /* HSI 16 MHz, no PLL; APB1, APB2 and AHB prescalers = 1. */
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) { }

    RCC->CFGR &= ~(RCC_CFGR_SW_Msk | RCC_CFGR_HPRE_Msk |
                   RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk);
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != 0U) { }
    RCC->CR &= ~RCC_CR_PLLON;
    SystemCoreClockUpdate();
}
