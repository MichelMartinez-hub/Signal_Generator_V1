#include "app/scheduler.h"
#include "bsp/scheduler_timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define CONTROLS_PERIOD_MS 50U

#if (CONTROLS_PERIOD_MS % SCHEDULER_TIMER_TICK_MS) != 0U
#error "Controls period must be an integer number of scheduler ticks"
#endif

#define CONTROLS_PERIOD_TICKS \
    (CONTROLS_PERIOD_MS / SCHEDULER_TIMER_TICK_MS)

static bool scheduler_running = false;
static uint32_t last_controls_tick = 0U;
static Scheduler_TaskCallback controls_task = NULL;

void Scheduler_Init(void)
{
    SchedulerTimer_Init();
    last_controls_tick = SchedulerTimer_GetTicks();
    scheduler_running = false;
    controls_task = NULL;
}

void Scheduler_SetControlsTask(Scheduler_TaskCallback callback)
{
    controls_task = callback;
}

void Scheduler_Start(void)
{
    last_controls_tick = SchedulerTimer_GetTicks();
    scheduler_running = true;
    SchedulerTimer_Start();
}

void Scheduler_Stop(void)
{
    SchedulerTimer_Stop();
    scheduler_running = false;
}

void Scheduler_Run(void)
{
    if (!scheduler_running)
    {
        return;
    }

    const uint32_t now = SchedulerTimer_GetTicks();

    if ((uint32_t)(now - last_controls_tick) >= CONTROLS_PERIOD_TICKS)
    {
        /* Skip missed periods rather than executing a burst of tasks. */
        last_controls_tick = now;
        if (controls_task != NULL)
        {
            controls_task();
        }
    }
}
