#include "app/app.h"

#include "app/controls.h"
#include "app/parameter_manager.h"
#include "app/scheduler.h"
#include "generator/generator.h"

static AppFault_t fault = APP_FAULT_NONE;
static AppControlSource_t control_source = APP_SOURCE_LOCAL;

static void App_EnterFault(AppFault_t cause)
{
    Generator_Stop();
    fault = cause;
    FSM_Fail();
}

/* Called by Scheduler_Run() in foreground context every 50 ms. */
static void App_ControlsTask(void)
{
    if ((FSM_GetState() != FSM_STATE_RUNNING) ||
        (control_source != APP_SOURCE_LOCAL))
    {
        return;
    }

    ControlsRaw_t raw;
    if (!Controls_ReadRaw(&raw))
    {
        App_EnterFault(APP_FAULT_CONTROLS_READ);
        return;
    }

    if (!ParameterManager_Update(&raw))
    {
        App_EnterFault(APP_FAULT_PARAMETER_UPDATE);
    }
}

void App_Init(void)
{
    FSM_Init();
    fault = APP_FAULT_NONE;
    control_source = APP_SOURCE_LOCAL;

    Generator_Init();
    Controls_Init();
    ParameterManager_Init();

    Scheduler_Init();
    Scheduler_SetControlsTask(App_ControlsTask);
    Scheduler_Start();
    (void)FSM_SetReady();
}

void App_Run(void)
{
    Scheduler_Run();
}

bool App_Start(void)
{
    if (FSM_GetState() != FSM_STATE_IDLE)
    {
        return false;
    }

    fault = APP_FAULT_NONE;
    if (control_source == APP_SOURCE_REMOTE)
    {
        /* Remote START applies the settings configured by CLI immediately. */
        if (!Generator_Start())
        {
            App_EnterFault(APP_FAULT_GENERATOR_START);
            return false;
        }
        return FSM_Start();
    }

    /* Local START waits for PM to settle on three ADC observations. */
    ParameterManager_Init();
    return FSM_Start();
}

void App_Stop(void)
{
    Generator_Stop();
    (void)FSM_Stop(); /* a FAULT is not cleared by STOP */
}

bool App_ClearFault(void)
{
    if (FSM_GetState() != FSM_STATE_FAULT)
    {
        return false;
    }

    Generator_Stop();
    if (!FSM_ResetFault())
    {
        return false;
    }
    fault = APP_FAULT_NONE;
    return true;
}

FSM_State_t App_GetState(void)
{
    return FSM_GetState();
}

AppFault_t App_GetFault(void)
{
    return fault;
}

bool App_SetControlSource(AppControlSource_t source)
{
    if (FSM_GetState() != FSM_STATE_IDLE ||
        (source != APP_SOURCE_LOCAL && source != APP_SOURCE_REMOTE))
    {
        return false;
    }

    if (source != control_source)
    {
        control_source = source;
        /* Relearn potentiometers if local control is selected again. */
        ParameterManager_Init();
    }
    return true;
}

AppControlSource_t App_GetControlSource(void)
{
    return control_source;
}

static bool App_CanConfigureRemote(void)
{
    return (FSM_GetState() == FSM_STATE_IDLE) &&
           (control_source == APP_SOURCE_REMOTE);
}

bool App_SetMode(GeneratorMode_t mode)
{
    return App_CanConfigureRemote() && Generator_SetMode(mode);
}

bool App_SetWaveform(Waveform_t waveform)
{
    return App_CanConfigureRemote() && Generator_SetWaveform(waveform);
}

bool App_SetFrequency(float frequency)
{
    return App_CanConfigureRemote() && Generator_SetFrequency(frequency);
}

bool App_SetAmplitude(float amplitude)
{
    return App_CanConfigureRemote() && Generator_SetAmplitude(amplitude);
}

bool App_SetOffset(float offset)
{
    return App_CanConfigureRemote() && Generator_SetOffset(offset);
}

bool App_GetGeneratorSettings(GeneratorSettings_t *out)
{
    return Generator_GetSettings(out);
}

