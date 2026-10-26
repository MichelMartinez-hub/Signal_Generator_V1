#include "protocol/cli.h"
#include "bsp/uart.h"
#include "app/app.h"
#include "config/generator_config.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLI_LINE_CAPACITY 96U

static char command_line[CLI_LINE_CAPACITY];
static uint16_t line_length = 0U;
static bool discard_line = false;

static void CLI_Ok(void)
{
    UART_Write("OK\r\n");
}

static void CLI_Error(const char *message)
{
    UART_Write("ERR: ");
    UART_Write(message);
    UART_Write("\r\n");
}

/* Explicit fixed-point formatting avoids reliance on newlib-nano %f support. */
static void CLI_WriteFloat2(float number)
{
    if (!isfinite(number) || fabsf(number) > 1000000.0f)
    {
        UART_Write("N/A");
        return;
    }

    const bool negative = number < 0.0f;
    const uint32_t hundredths = (uint32_t)(fabsf(number) * 100.0f + 0.5f);
    char output[24];
    (void)snprintf(output, sizeof(output), "%s%lu.%02lu",
                   negative ? "-" : "",
                   (unsigned long)(hundredths / 100U),
                   (unsigned long)(hundredths % 100U));
    UART_Write(output);
}

static const char *CLI_StateName(FSM_State_t state)
{
    switch (state)
    {
        case FSM_STATE_INIT:    return "INIT";
        case FSM_STATE_IDLE:    return "IDLE";
        case FSM_STATE_RUNNING: return "RUNNING";
        case FSM_STATE_FAULT:   return "FAULT";
        default:                return "UNKNOWN";
    }
}

static const char *CLI_FaultName(AppFault_t fault)
{
    switch (fault)
    {
        case APP_FAULT_NONE:            return "NONE";
        case APP_FAULT_CONTROLS_READ:   return "CONTROLS_READ";
        case APP_FAULT_PARAMETER_UPDATE:return "PARAMETER_UPDATE";
        case APP_FAULT_GENERATOR_START: return "GENERATOR_START";
        default:                        return "UNKNOWN";
    }
}

static const char *CLI_WaveformName(Waveform_t waveform)
{
    switch (waveform)
    {
        case WAVEFORM_DC:       return "DC";
        case WAVEFORM_SINE:     return "SINE";
        case WAVEFORM_TRIANGLE: return "TRIANGLE";
        case WAVEFORM_SQUARE:   return "SQUARE";
        default:                return "UNKNOWN";
    }
}

static bool CLI_ParseFloat(const char *arg, float *value)
{
    if (arg == NULL || value == NULL)
    {
        return false;
    }

    char *end = NULL;
    errno = 0;
    const float parsed = strtof(arg, &end);
    if (end == arg || *end != '\0' || errno == ERANGE || !isfinite(parsed))
    {
        return false;
    }

    *value = parsed;
    return true;
}

static bool CLI_RequireRemoteIdle(void)
{
    if (App_GetControlSource() != APP_SOURCE_REMOTE)
    {
        CLI_Error("select SOURCE REMOTE first");
        return false;
    }
    if (App_GetState() != FSM_STATE_IDLE)
    {
        CLI_Error("STOP and clear any FAULT before configuration");
        return false;
    }
    return true;
}

static void CLI_PrintStatus(void)
{
    GeneratorSettings_t settings;
    UART_Write("STATE=");
    UART_Write(CLI_StateName(App_GetState()));
    UART_Write(" SOURCE=");
    UART_Write(App_GetControlSource() == APP_SOURCE_REMOTE ? "REMOTE" : "LOCAL");
    UART_Write(" FAULT=");
    UART_Write(CLI_FaultName(App_GetFault()));
    UART_Write("\r\n");

    if (!App_GetGeneratorSettings(&settings))
    {
        CLI_Error("generator not initialized");
        return;
    }

    UART_Write("MODE=");
    UART_Write(settings.mode == GENERATOR_MODE_MATH ? "MATH" : "LUT");
    UART_Write(" WAVE=");
    UART_Write(CLI_WaveformName(settings.waveform));
    UART_Write(" FREQ_HZ=");
    CLI_WriteFloat2(settings.frequency);
    UART_Write(" AMP_VPK=");
    CLI_WriteFloat2(settings.amplitude);
    UART_Write(" OFFSET_V=");
    CLI_WriteFloat2(settings.offset);
    UART_Write("\r\n");
}

static void CLI_ProcessLine(char *line)
{
    /* Uppercase command and enum tokens; numeric exponent 'e' is also valid as 'E'. */
    for (char *p = line; *p != '\0'; ++p)
    {
        *p = (char)toupper((unsigned char)*p);
    }

    char *command = strtok(line, " \t");
    if (command == NULL)
    {
        return;
    }
    char *argument = strtok(NULL, " \t");
    char *extra = strtok(NULL, " \t");
    if (extra != NULL)
    {
        CLI_Error("too many arguments");
        return;
    }

    if (strcmp(command, "HELP") == 0 && argument == NULL)
    {
        UART_Write("HELP STATUS START STOP RESET\r\n");
        UART_Write("SOURCE LOCAL|REMOTE (only while IDLE)\r\n");
        UART_Write("MODE LUT|MATH; WAVE DC|SINE|TRIANGLE|SQUARE\r\n");
        UART_Write("FREQ <Hz>; AMP <Vpk>; OFFSET <V>\r\n");
        UART_Write("Settings: SOURCE REMOTE + IDLE; use START after edits.\r\n");
        return;
    }

    if (strcmp(command, "STATUS") == 0 && argument == NULL)
    {
        CLI_PrintStatus();
        return;
    }

    if (strcmp(command, "START") == 0 && argument == NULL)
    {
        if (App_Start()) CLI_Ok();
        else CLI_Error("START rejected; check STATUS");
        return;
    }

    if (strcmp(command, "STOP") == 0 && argument == NULL)
    {
        App_Stop();
        CLI_Ok();
        return;
    }

    if (strcmp(command, "RESET") == 0 && argument == NULL)
    {
        if (App_ClearFault()) CLI_Ok();
        else CLI_Error("RESET only valid in FAULT");
        return;
    }

    if (strcmp(command, "SOURCE") == 0 && argument != NULL)
    {
        AppControlSource_t source;
        if (strcmp(argument, "LOCAL") == 0) source = APP_SOURCE_LOCAL;
        else if (strcmp(argument, "REMOTE") == 0) source = APP_SOURCE_REMOTE;
        else { CLI_Error("SOURCE must be LOCAL or REMOTE"); return; }

        if (App_SetControlSource(source)) CLI_Ok();
        else CLI_Error("SOURCE change requires IDLE");
        return;
    }

    if (strcmp(command, "MODE") == 0 && argument != NULL)
    {
        if (!CLI_RequireRemoteIdle()) return;
        GeneratorMode_t mode;
        if (strcmp(argument, "LUT") == 0) mode = GENERATOR_MODE_LUT;
        else if (strcmp(argument, "MATH") == 0) mode = GENERATOR_MODE_MATH;
        else { CLI_Error("MODE must be LUT or MATH"); return; }

        if (App_SetMode(mode)) CLI_Ok();
        else CLI_Error("MODE rejected");
        return;
    }

    if (strcmp(command, "WAVE") == 0 && argument != NULL)
    {
        if (!CLI_RequireRemoteIdle()) return;
        Waveform_t waveform;
        if (strcmp(argument, "DC") == 0) waveform = WAVEFORM_DC;
        else if (strcmp(argument, "SINE") == 0) waveform = WAVEFORM_SINE;
        else if (strcmp(argument, "TRIANGLE") == 0) waveform = WAVEFORM_TRIANGLE;
        else if (strcmp(argument, "SQUARE") == 0) waveform = WAVEFORM_SQUARE;
        else { CLI_Error("unknown waveform"); return; }

        if (App_SetWaveform(waveform)) CLI_Ok();
        else CLI_Error("WAVE rejected");
        return;
    }

    if ((strcmp(command, "FREQ") == 0 ||
         strcmp(command, "AMP") == 0 ||
         strcmp(command, "OFFSET") == 0) && argument != NULL)
    {
        if (!CLI_RequireRemoteIdle()) return;
        float value;
        if (!CLI_ParseFloat(argument, &value))
        {
            CLI_Error("invalid numeric value");
            return;
        }

        bool accepted = false;
        if (strcmp(command, "FREQ") == 0)
        {
            /* Match current PM range until both modes are benchmarked. */
            if (value >= GENERATOR_MIN_FREQUENCY_HZ &&
                value <= GENERATOR_MAX_FREQUENCY_HZ)
                accepted = App_SetFrequency(value);
        }
        else if (strcmp(command, "AMP") == 0)
        {
            accepted = App_SetAmplitude(value);
        }
        else
        {
            accepted = App_SetOffset(value);
        }
        if (accepted) CLI_Ok();
        else CLI_Error("parameter out of range or incompatible with amplitude/offset");
        return;
    }

    CLI_Error("unknown command or wrong argument count; type HELP");
}

void CLI_Init(void)
{
    line_length = 0U;
    discard_line = false;
    UART_Init();
    UART_Write("\r\nSTM32 Signal Generator CLI (115200 8N1)\r\n");
    UART_Write("Type HELP. Default input source: LOCAL.\r\n");
}

void CLI_Run(void)
{
    if (UART_TakeRxOverflow())
    {
        discard_line = true;
    }

    uint8_t byte;
    while (UART_ReadByte(&byte))
    {
        if (byte == '\r' || byte == '\n')
        {
            /* Catch an RX overflow occurring while this invocation runs. */
            if (UART_TakeRxOverflow())
            {
                discard_line = true;
            }
            if (discard_line)
            {
                CLI_Error("receive buffer or line overflow; command discarded");
            }
            else if (line_length != 0U)
            {
                command_line[line_length] = '\0';
                CLI_ProcessLine(command_line);
            }
            line_length = 0U;
            discard_line = false;
            continue;
        }

        if (discard_line)
        {
            continue;
        }

        if (byte == 8U || byte == 127U)
        {
            if (line_length != 0U) --line_length;
            continue;
        }

        if (byte < 32U || byte > 126U)
        {
            continue;
        }

        if (line_length >= (CLI_LINE_CAPACITY - 1U))
        {
            discard_line = true;
            continue;
        }
        command_line[line_length++] = (char)byte;
    }
}
