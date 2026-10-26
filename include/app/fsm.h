#ifndef FSM_H
#define FSM_H

#include <stdbool.h>

typedef enum
{
    FSM_STATE_INIT = 0,
    FSM_STATE_IDLE,
    FSM_STATE_RUNNING,
    FSM_STATE_FAULT
} FSM_State_t;

/* FSM handles state transitions only; no GPIO, timers, or generator calls. */
void FSM_Init(void);
bool FSM_SetReady(void);      /* INIT -> IDLE */
bool FSM_Start(void);         /* IDLE -> RUNNING */
bool FSM_Stop(void);          /* RUNNING -> IDLE */
void FSM_Fail(void);          /* any state -> FAULT */
bool FSM_ResetFault(void);    /* FAULT -> IDLE */
FSM_State_t FSM_GetState(void);

#endif /* FSM_H */
