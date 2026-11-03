#include "bsp/system.h"
#include "stm32f446xx.h"

/* Clock initialization for STM32F446RE, HSI source, VDD in specified
 * operational range. Does not enable overdrive or rely on HSE crystal.
 * Check board supply and flash latency when adapting to other hardware.
 */
void System_Init(void)
{
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) { }

    /* Always transition to HSI before changing PLL. */
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != 0U) { }

    RCC->CR &= ~RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) != 0U) { }

    /* Voltage regulator scale 1 (VOS = 11), APB1 peripheral clock ON. */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;
    PWR->CR = (PWR->CR & ~PWR_CR_VOS_Msk) | PWR_CR_VOS_Msk;

    /* Set FLASH wait states BEFORE raising SYSCLK. */
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |
                 FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;
    while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != FLASH_ACR_LATENCY_5WS) { }

    /* HSI /16 *336 /2 = 168 MHz; PLLQ=7 (48 MHz nominal). */
    RCC->PLLCFGR = (16UL  << RCC_PLLCFGR_PLLM_Pos) |
                   (336UL << RCC_PLLCFGR_PLLN_Pos) |
                   (0UL   << RCC_PLLCFGR_PLLP_Pos) |
                   (7UL   << RCC_PLLCFGR_PLLQ_Pos);

    /* APB1 /4; APB2 /2; AHB /1. */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE_Msk |
                                RCC_CFGR_PPRE1_Msk |
                                RCC_CFGR_PPRE2_Msk)) |
                 RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) { }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL) { }

    SystemCoreClockUpdate();
}
