#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* HSI 16 MHz -> PLLM=16, PLLN=336, PLLP=2 -> SYSCLK=168 MHz.
 * AHB=168 MHz, APB1=/4 (42 MHz), APB2=/2 (84 MHz).
 * APB timer clock doubles when APB prescaler != 1.
 * Requires nominal 3.3 V power supply and correct flash wait states.
 */
#define SYSTEM_CLOCK_HZ             168000000UL
#define APB1_CLOCK_HZ                42000000UL
#define APB1_TIMER_CLOCK_HZ          84000000UL
#define APB2_CLOCK_HZ                84000000UL
#define APB2_TIMER_CLOCK_HZ         168000000UL

#define DAC_RESOLUTION_BITS                12U
#define DAC_MAX_CODE                    4095U
#define DAC_MIDSCALE_CODE               2048U

/* Base flash table remains 256 entries; high-frequency playback uses
 * an integer-power-of-two decimated table without regenerating it.
 */
#define LUT_SIZE                        256U
#define GENERATOR_MIN_SAMPLES_PER_CYCLE  16U
#define GENERATOR_SAMPLE_RATE_MAX_HZ 800000UL
#define MATH_DMA_BUFFER_SIZE             256U

#endif /* BOARD_CONFIG_H */
