#ifndef CLI_H
#define CLI_H

/* CLI runs only in the foreground, never from USART2_IRQHandler. */
void CLI_Init(void);
void CLI_Run(void);

#endif /* CLI_H */
