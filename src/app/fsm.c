#include "app/fsm.h"

static FSM_State_t state = FSM_STATE_INIT;

void FSM_Init(void)
{
    state = FSM_STATE_INIT;
}

bool FSM_SetReady(void)
{
    if (state != FSM_STATE_INIT)
    {
        return false;
    }
    state = FSM_STATE_IDLE;
    return true;
}

bool FSM_Start(void)
{
    if (state != FSM_STATE_IDLE)
    {
        return false;
    }
    state = FSM_STATE_RUNNING;
    return true;
}

bool FSM_Stop(void)
{
    if (state != FSM_STATE_RUNNING)
    {
        return false;
    }
    state = FSM_STATE_IDLE;
    return true;
}

void FSM_Fail(void)
{
    state = FSM_STATE_FAULT;
}

bool FSM_ResetFault(void)
{
    if (state != FSM_STATE_FAULT)
    {
        return false;
    }
    state = FSM_STATE_IDLE;
    return true;
}

FSM_State_t FSM_GetState(void)
{
    return state;
}
