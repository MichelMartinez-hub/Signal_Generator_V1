#include "bsp/uart.h"
#include "config/board_config.h"
#include "stm32f446xx.h"

#include <stddef.h>

#define UART_BAUDRATE      115200UL
#define UART_RX_BUFFER_LEN 128U
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_LEN - 1U)

#if (UART_RX_BUFFER_LEN & UART_RX_BUFFER_MASK) != 0U
#error "UART_RX_BUFFER_LEN must be a power of two"
#endif

#if !defined(APB1_CLOCK_HZ)
#error "APB1_CLOCK_HZ must be declared in board_config.h"
#endif

static volatile uint8_t rx_buffer[UART_RX_BUFFER_LEN];
static volatile uint16_t rx_head = 0U;
static volatile uint16_t rx_tail = 0U;
static volatile bool rx_overflow = false;

void UART_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    /* PA2 USART2_TX / PA3 USART2_RX (AF7). */
    GPIOA->MODER &= ~((3UL << (2U * 2U)) | (3UL << (3U * 2U)));
    GPIOA->MODER |=  ((2UL << (2U * 2U)) | (2UL << (3U * 2U)));
    GPIOA->AFR[0] &= ~((15UL << (2U * 4U)) | (15UL << (3U * 4U)));
    GPIOA->AFR[0] |=  ((7UL << (2U * 4U)) | (7UL << (3U * 4U)));
    GPIOA->OTYPER &= ~((1UL << 2U) | (1UL << 3U));
    GPIOA->OSPEEDR |= (3UL << (2U * 2U)) | (3UL << (3U * 2U));
    GPIOA->PUPDR &= ~((3UL << (2U * 2U)) | (3UL << (3U * 2U)));
    GPIOA->PUPDR |= (1UL << (3U * 2U)); /* weak RX pull-up */

    USART2->CR1 = 0U;
    USART2->CR2 = 0U; /* 1 stop bit */
    USART2->CR3 = 0U; /* no flow control */

    /* OVER8 = 0 (16x oversampling): BRR approximately PCLK / baud. */
    USART2->BRR = (APB1_CLOCK_HZ + (UART_BAUDRATE / 2UL)) / UART_BAUDRATE;

    rx_head = 0U;
    rx_tail = 0U;
    rx_overflow = false;
    NVIC_ClearPendingIRQ(USART2_IRQn);
    NVIC_SetPriority(USART2_IRQn, 7U);
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;
    NVIC_EnableIRQ(USART2_IRQn);
}

bool UART_ReadByte(uint8_t *value)
{
    if (value == NULL || rx_tail == rx_head)
    {
        return false;
    }

    *value = rx_buffer[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1U) & UART_RX_BUFFER_MASK);
    return true;
}

void UART_Write(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    while (*text != '\0')
    {
        while ((USART2->SR & USART_SR_TXE) == 0U)
        {
        }
        USART2->DR = (uint8_t)*text++;
    }
}

bool UART_TakeRxOverflow(void)
{
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    const bool had_overflow = rx_overflow;
    rx_overflow = false;
    if (primask == 0U)
    {
        __enable_irq();
    }
    return had_overflow;
}

void USART2_IRQHandler(void)
{
    /* On STM32F4, reading SR then DR clears RXNE and UART error flags. */
    const uint32_t status = USART2->SR;
    if ((status & (USART_SR_RXNE | USART_SR_ORE |
                   USART_SR_FE | USART_SR_NE | USART_SR_PE)) == 0U)
    {
        return;
    }

    const uint8_t data = (uint8_t)USART2->DR;
    if ((status & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0U)
    {
        rx_overflow = true;
        return;
    }

    if ((status & USART_SR_RXNE) != 0U)
    {
        const uint16_t next = (uint16_t)((rx_head + 1U) & UART_RX_BUFFER_MASK);
        if (next == rx_tail)
        {
            rx_overflow = true;
        }
        else
        {
            rx_buffer[rx_head] = data;
            rx_head = next;
        }
    }
}
