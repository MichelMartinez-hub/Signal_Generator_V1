
#ifndef SCHEDULER_TIMER_H
#define SCHEDULER_TIMER_H

#include <stdint.h>

/* TIM3 timebase: 10 ms per tick (100 Hz) */
#define SCHEDULER_TIMER_TICK_MS 10U

void SchedulerTimer_Init(void);
void SchedulerTimer_Start(void);
void SchedulerTimer_Stop(void);

uint32_t SchedulerTimer_GetTicks(void);

#endif /* SCHEDULER_TIMER_H */
