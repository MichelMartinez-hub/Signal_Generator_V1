#ifndef SCHEDULER_H
#define SCHEDULER_H

/* Cooperative foreground scheduler; no application logic in its timer IRQ. */
typedef void (*Scheduler_TaskCallback)(void);

/* Existing API: unchanged. */
void Scheduler_Init(void);
void Scheduler_Start(void);
void Scheduler_Stop(void);
void Scheduler_Run(void);

/* New API: application registers the control task executed every 50 ms. */
void Scheduler_SetControlsTask(Scheduler_TaskCallback callback);

#endif /* SCHEDULER_H */
