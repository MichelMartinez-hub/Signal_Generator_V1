#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include "app/fsm.h"
#include "generator/generator.h"

typedef enum
{
    APP_FAULT_NONE = 0,
    APP_FAULT_CONTROLS_READ,
    APP_FAULT_PARAMETER_UPDATE,
    APP_FAULT_GENERATOR_START /* NEW: remote START failure */
} AppFault_t;

typedef enum
{
    APP_SOURCE_LOCAL = 0,
    APP_SOURCE_REMOTE
} AppControlSource_t;

/* Original public API: unchanged. */
void App_Init(void);
void App_Run(void);
bool App_Start(void);
void App_Stop(void);
bool App_ClearFault(void);
FSM_State_t App_GetState(void);
AppFault_t App_GetFault(void);

/* New API for remote/local ownership and remote-only configuration. */
bool App_SetControlSource(AppControlSource_t source);
AppControlSource_t App_GetControlSource(void);

bool App_SetMode(GeneratorMode_t mode);
bool App_SetWaveform(Waveform_t waveform);
bool App_SetFrequency(float frequency);
bool App_SetAmplitude(float amplitude);
bool App_SetOffset(float offset);
bool App_GetGeneratorSettings(GeneratorSettings_t *out);

#endif /* APP_H */
