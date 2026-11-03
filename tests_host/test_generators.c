#include "generator/generator.h"
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
    puts("PASS: LUT16@50k, MATH DMA stream@50k, sample plan, generator facade, protocol");
    return 0;
}
