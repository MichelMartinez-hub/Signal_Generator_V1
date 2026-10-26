#include "bsp/gpio.h"
#include "stm32f446xx.h"

void GPIO_Init(void)
{
    // Enable GPIOA clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configure PA4 as output (for example, an LED)
    GPIOA->MODER &= ~GPIO_MODER_MODE4; // Clear mode bits for PA4
    GPIOA->MODER |= GPIO_MODER_MODE4;  // Set PA4 to analog mode
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4; // No pull-up, no pull-down

    // Optionally configure other settings like speed, pull-up/pull-down, etc.
}
