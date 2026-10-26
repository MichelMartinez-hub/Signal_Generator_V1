#include "generator/generator.h"
#include "generator/generator_lut.h"
#include "generator/generator_math.h"
#include "generator/waveform.h"
#include "bsp/wave_output.h"
#include "config/board_config.h"
#include "protocol/protocol.h"
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

static uint16_t buf_copy[LUT_SIZE];
static uint16_t length_saved;
static uint32_t rate_saved;
static WaveOutput_SampleProvider callback_saved;
static int stop_calls;
void WaveOutput_Init(void) { }
void WaveOutput_Stop(void) { stop_calls++; callback_saved = NULL; }
void WaveOutput_SetDC(uint16_t value) { (void)value; }
bool WaveOutput_IsRunning(void) { return false; }
bool WaveOutput_StartBuffer(const uint16_t *buffer, uint16_t length, uint32_t sampleRate)
{
    length_saved = length;
    rate_saved = sampleRate;
    for (unsigned i=0; i<length; ++i) buf_copy[i]=buffer[i];
    return true;
}
bool WaveOutput_StartRealtime(WaveOutput_SampleProvider provider, uint32_t sampleRate)
{
    callback_saved = provider;
    rate_saved = sampleRate;
    return true;
}
int main(void)
{
    assert(fabsf(Waveform_Evaluate(WAVEFORM_SINE,.25f)-1.0f) < .00001f);
    assert(Waveform_Evaluate(WAVEFORM_TRIANGLE,0.0f) == -1.0f);
    assert(Waveform_Evaluate(WAVEFORM_SQUARE,.75f) == -1.0f);
    Generator_Init();
    assert(Generator_SetWaveform(WAVEFORM_SINE));
    assert(Generator_SetFrequency(10.0f));
    assert(Generator_SetAmplitude(5.0f));
    assert(!Generator_SetOffset(1.0f));
    assert(Generator_Start());
    assert(length_saved==LUT_SIZE && rate_saved == 2560);
    assert(buf_copy[0]==2048 && buf_copy[64]==4095 && buf_copy[192]==0);
    GeneratorSettings_t cfg;
    assert(Generator_GetSettings(&cfg));
    assert(cfg.mode == GENERATOR_MODE_LUT && cfg.frequency == 10.0f);
    assert(Generator_SetWaveform(WAVEFORM_TRIANGLE) && Generator_Start());
    assert(buf_copy[0] == 0 && buf_copy[128] == 4095);
    assert(Generator_SetMode(GENERATOR_MODE_MATH));
    assert(Generator_SetWaveform(WAVEFORM_SINE) && Generator_Start());
    assert(callback_saved != NULL && rate_saved == 2560);
    assert(callback_saved() == 2048);
    uint16_t v=0;
    for (int i=0;i<64;i++) v=callback_saved();
    assert(v==4095);
    assert(!Generator_SetFrequency(0.0f));
    /* Check unsupported rates in the facade and both public backends. */
    assert(!Generator_SetFrequency(NAN));
    assert(!Generator_SetFrequency(INFINITY));
    assert(!Generator_SetFrequency(0.5f));
    assert(!Generator_SetFrequency(101.0f));
    assert(Generator_SetFrequency(1.0f));
    assert(Generator_SetFrequency(100.0f));
    assert(!GeneratorLUT_Start(0.5f));
    assert(!GeneratorLUT_Start(101.0f));
    assert(!GeneratorMath_Start(0.5f));
    assert(!GeneratorMath_Start(101.0f));
    assert(!Generator_GetSettings(NULL));
    uint8_t frame[PROTOCOL_MAX_FRAME];
    size_t written=0;
    uint8_t payload[]={2,3,5};
    uint8_t rtype=0,cmd=0,out[PROTOCOL_MAX_PAYLOAD],n=0;
    assert(Protocol_Encode(1,4,payload,3,frame,sizeof(frame),&written));
    assert(Protocol_Decode(frame,written,&rtype,&cmd,out,sizeof(out),&n));
    assert(rtype==1 && cmd==4 && n==3 && out[2]==5);
    frame[5] ^= 1;
    assert(!Protocol_Decode(frame,written,&rtype,&cmd,out,sizeof(out),&n));
    assert(Protocol_Encode(1,4,NULL,0,frame,sizeof(frame),&written));
    assert(Protocol_Decode(frame,written,&rtype,&cmd,NULL,0,&n) && n==0);
    uint8_t max_payload[PROTOCOL_MAX_PAYLOAD];
    for (unsigned i=0; i<sizeof(max_payload); ++i) max_payload[i]=(uint8_t)i;
    assert(!Protocol_Encode(1,4,NULL,1,frame,sizeof(frame),&written));
    assert(!Protocol_Encode(1,4,max_payload,sizeof(max_payload),frame,5,&written));
    assert(Protocol_Encode(1,4,max_payload,sizeof(max_payload),frame,sizeof(frame),&written));
    assert(written==PROTOCOL_MAX_FRAME);
    assert(!Protocol_Decode(frame,written-1,&rtype,&cmd,out,sizeof(out),&n));
    assert(!Protocol_Decode(frame,written,&rtype,&cmd,out,1,&n));
    assert(Protocol_Decode(frame,written,&rtype,&cmd,out,sizeof(out),&n));
    for (unsigned i=0; i<n; ++i) assert(out[i]==max_payload[i]);
    assert(stop_calls>0);
    puts("PASS: LUT/MATH, output limits, sampling, generator API, optional protocol");
    return 0;
}
