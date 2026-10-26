#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* =========================================================
 * SYSTEM CLOCK
 * ========================================================= */

/*
 * Current system configuration:
 * HSI = 16 MHz
 *
 * system.c currently selects HSI directly as SYSCLK.
 */
#define SYSTEM_CLOCK_HZ            16000000UL


/* =========================================================
 * APB CLOCKS
 * ========================================================= */

/*
 * Current configuration uses the default APB1 prescaler = 1.
 *
 * Therefore:
 *
 * PCLK1 = 16 MHz
 * TIMxCLK (APB1 timers) = 16 MHz
 */
#define APB1_CLOCK_HZ              16000000UL
#define APB1_TIMER_CLOCK_HZ        16000000UL


/*
 * APB2 also currently runs without prescaling.
 */
#define APB2_CLOCK_HZ              16000000UL
#define APB2_TIMER_CLOCK_HZ        16000000UL


/* =========================================================
 * DAC
 * ========================================================= */

#define DAC_RESOLUTION_BITS        12U
#define DAC_MAX_CODE               4095U
#define DAC_MIDSCALE_CODE          2048U


/* =========================================================
 * SAMPLE GENERATION
 * ========================================================= */

/*
 * Number of samples used for one waveform period
 * in the initial LUT implementation.
 */
#define LUT_SIZE                   256U


#endif /* BOARD_CONFIG_H */