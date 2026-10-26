#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

/* USART2 on PA2/PA3, 115200 8N1, IRQ-driven RX, polled TX. */
void UART_Init(void);
bool UART_ReadByte(uint8_t *value);
void UART_Write(const char *text);
bool UART_TakeRxOverflow(void);

#endif /* UART_H */
