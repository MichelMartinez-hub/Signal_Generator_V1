#include "generator/generator.h"
#include "app/parameter_manager.h"
#include "app/app.h"
#include "bsp/scheduler_timer.h"
#include "bsp/uart.h"
#include "protocol/cli.h"
#include "generator/generator_lut.h"
#include "generator/generator_math.h"
#include "generator/sample_plan.h"
#include "generator/waveform.h"
#include "bsp/wave_output.h"
#include "config/board_config.h"
#include "protocol/protocol.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>

static uint16_t sample_copy[LUT_SIZE];
static uint16_t sample_length;
static uint32_t saved_rate;
static WaveOutput_RefillCallback refill_cb;
static int stop_count;

/* No peripheral register accesses in host tests. */
void WaveOutput_Init(void) { }
void WaveOutput_Stop(void) { ++stop_count; refill_cb = NULL; }
void WaveOutput_SetDC(uint16_t value) { (void)value; }
bool WaveOutput_IsRunning(void) { return false; }
bool WaveOutput_HasError(void) { return false; }
bool WaveOutput_StartBuffer(const uint16_t *buf, uint16_t n, uint32_t fs)
{
    sample_length = n;
    saved_rate = fs;
    for (uint16_t i = 0; i < n; ++i) sample_copy[i] = buf[i];
    return true;
}
bool WaveOutput_StartStreaming(uint16_t *buf, uint16_t n,
                               uint32_t fs, WaveOutput_RefillCallback cb)
{
    refill_cb = cb;
    sample_length = n;
    saved_rate = fs;
    for (uint16_t i = 0; i < n; ++i) sample_copy[i] = buf[i];
    return true;
}
bool WaveOutput_StartRealtime(WaveOutput_SampleProvider provider,
                              uint32_t fs)
{
    (void)provider;
    saved_rate = fs;
    return true;
}

static GeneratorSettings_t observe(ControlsRaw_t *raw, unsigned repeats)
{
    GeneratorSettings_t settings;
    for (unsigned i = 0; i < repeats; ++i) assert(ParameterManager_Update(raw));
    assert(Generator_GetSettings(&settings));
    return settings;
}

static void test_local(void)
{
    ControlsRaw_t raw = {0, 4095, 2048, 0};
    Generator_Init();
    float previous = 0.0f;
    /* Fresh acquisition at each code checks the unlatched transfer curve. */
    for (unsigned code = 0; code <= 4095; ++code)
    {
        ParameterManager_Init();
        raw.frequency_adc = (uint16_t)code;
        float hz = observe(&raw, 3).frequency;
        assert(isfinite(hz) && hz >= 1 && hz <= 50000 && hz >= previous);
        previous = hz;
    }
    const uint16_t codes[] = {0,819,1638,2457,3276,3583,4095};
    const float expected[] = {1,10,100,1000,10000,25000,50000};
    for (int mode = 0; mode < 2; ++mode)
    {
        assert(Generator_SetMode(mode ? GENERATOR_MODE_MATH : GENERATOR_MODE_LUT));
        ParameterManager_Init();
        for (unsigned i = 0; i < sizeof(codes)/sizeof(codes[0]); ++i)
        {
            raw.frequency_adc = codes[i];
            GeneratorSettings_t settings = observe(&raw, 24);
            assert(settings.frequency == expected[i]);
            assert(settings.amplitude == 5 && settings.offset == 0);
            SamplePlan_t plan;
            assert(SamplePlan_Make(expected[i], &plan) && saved_rate == plan.sample_rate_hz);
            assert((refill_cb != NULL) == (mode != 0));
        }
        raw.frequency_adc = 0;
        assert(observe(&raw, 24).frequency == 1);
        /* Start at a quantization boundary, then hold either side. */
        raw.frequency_adc = 3281;
        ParameterManager_Init();
        float held = observe(&raw, 3).frequency;
        int stops = stop_count;
        for (int i = 0; i < 24; ++i)
        {
            raw.frequency_adc = (i % 2) ? 3279 : 3283;
            assert(observe(&raw, 4).frequency == held);
        }
        assert(stop_count == stops);
        raw.frequency_adc = 4095;
        assert(observe(&raw, 24).frequency == 50000 && stop_count > stops);
        raw.amplitude_adc = 0;
        raw.offset_adc = 4095;
        GeneratorSettings_t changed = observe(&raw, 64);
        assert(changed.amplitude == 0 && changed.offset == 5);
        raw.amplitude_adc = 4095;
        raw.offset_adc = 2048;
    }
}

/* Exercise the production CLI and scheduler with a stationary PC0 at zero. */
static const char *rx;
static uint32_t ticks;
static unsigned control_reads;
void Controls_Init(void) {}
bool Controls_ReadRaw(ControlsRaw_t *out)
{
    ++control_reads;
    *out = (ControlsRaw_t){0, 2048, 2048, 0};
    return true;
}
void SchedulerTimer_Init(void) { ticks = 0; }
void SchedulerTimer_Start(void) {}
void SchedulerTimer_Stop(void) {}
uint32_t SchedulerTimer_GetTicks(void) { return ticks; }
void UART_Init(void) {}
void UART_Write(const char *text) { (void)text; }
bool UART_TakeRxOverflow(void) { return false; }
bool UART_ReadByte(uint8_t *value)
{
    if (!rx || !*rx) return false;
    *value = (uint8_t)*rx++;
    return true;
}
static void test_source_selection(void)
{
    App_Init();
    CLI_Init();
    rx = "SOURCE REMOTE\nMODE MATH\nFREQ 50000\nSTART\n";
    CLI_Run();
    assert(App_GetState() == FSM_STATE_RUNNING && saved_rate == 800000 && refill_cb);
    ticks += 5;
    App_Run();
    assert(control_reads == 0);
    rx = "STOP\nSOURCE LOCAL\nSTART\n";
    CLI_Run();
    for (unsigned i = 0; i < 3; ++i) { ticks += 5; App_Run(); }
    GeneratorSettings_t cfg;
    assert(control_reads == 3 && App_GetGeneratorSettings(&cfg));
    assert(cfg.frequency == 1 && cfg.mode == GENERATOR_MODE_MATH && saved_rate == 256);
}

int main(void)
{
    assert(fabsf(Waveform_Evaluate(WAVEFORM_SINE, 0.25f)-1.0f) < 0.00001f);
    assert(Waveform_Evaluate(WAVEFORM_TRIANGLE, 0.0f) == -1.0f);
    SamplePlan_t plan;
    assert(SamplePlan_Make(10.0f, &plan));
    assert(plan.samples_per_cycle == 256 && plan.sample_rate_hz == 2560);
    assert(SamplePlan_Make(50000.0f, &plan));
    assert(plan.samples_per_cycle == 16 && plan.sample_rate_hz == 800000);
    assert(SamplePlan_Make(25000.0f, &plan));
    assert(plan.samples_per_cycle == 32 && plan.sample_rate_hz == 800000);
    assert(!SamplePlan_Make(50000.1f, &plan));

    Generator_Init();
    assert(Generator_SetWaveform(WAVEFORM_SINE));
    assert(Generator_SetFrequency(10.0f));
    assert(Generator_SetAmplitude(5.0f));
    assert(!Generator_SetOffset(1.0f));
    assert(Generator_Start());
    assert(sample_length == 256 && saved_rate == 2560);
    assert(sample_copy[0] == 2048 && sample_copy[64] == 4095 && sample_copy[192] == 0);

    assert(Generator_SetFrequency(50000.0f));
    assert(Generator_Start());
    assert(sample_length == 16 && saved_rate == 800000);
    assert(sample_copy[0] == 2048 && sample_copy[4] == 4095 && sample_copy[12] == 0);
    GeneratorSettings_t settings;
    assert(Generator_GetSettings(&settings));
    assert(settings.frequency == 50000.0f && settings.mode == GENERATOR_MODE_LUT);

    assert(Generator_SetMode(GENERATOR_MODE_MATH));
    assert(Generator_Start());
    assert(sample_length == 256 && saved_rate == 800000 && refill_cb != NULL);
    assert(sample_copy[0] == 2048 && sample_copy[4] == 4095 && sample_copy[12] == 0);
    uint16_t regenerated[128];
    refill_cb(regenerated, 128);
    assert(regenerated[0] == 2048 && regenerated[4] == 4095 && regenerated[12] == 0);

    assert(!Generator_SetFrequency(0.0f));
    assert(!Generator_SetFrequency(NAN));
    assert(!Generator_SetFrequency(INFINITY));
    assert(!Generator_SetFrequency(50001.0f));
    assert(!Generator_GetSettings(NULL));

    uint8_t frame[PROTOCOL_MAX_FRAME], data[PROTOCOL_MAX_PAYLOAD];
    uint8_t payload[] = {2,3,5};
    uint8_t typ = 0, cmd = 0, count = 0;
    size_t written = 0;
    assert(Protocol_Encode(1,4,payload,3,frame,sizeof(frame),&written));
    assert(Protocol_Decode(frame,written,&typ,&cmd,data,sizeof(data),&count));
    assert(typ == 1 && cmd == 4 && count == 3 && data[2] == 5);
    assert(stop_count > 0);
    test_local();
    test_source_selection();
    puts("PASS: local PC0 sweep, jitter, both backends; LUT16@50k, MATH DMA stream@50k, sample plan, generator facade, protocol");
    return 0;
}
