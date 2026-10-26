#include "generator/generator_lut.h"
#include "config/board_config.h"
#include "bsp/wave_output.h"

#include <stdbool.h>
#include <stdint.h>
#include "config/generator_config.h"
#include <math.h>
#include <stddef.h>

/* The tables below are PRECOMPUTED constants, not generated at runtime. */
#if LUT_SIZE != 256U
#error "The stored LUT tables require LUT_SIZE == 256"
#endif

#define OUTPUT_MIN_VOLTAGE (-5.0f)
#define OUTPUT_MAX_VOLTAGE ( 5.0f)
#define OUTPUT_RANGE_VOLTAGE (10.0f)
#define NORMALIZED_FULL_SCALE (32767.0f)
#define DAC_FULL_SCALE (4095.0f)
#define DAC_MID_SCALE (2048U)

/* 256 samples each; signed Q15-like representation in Flash (.rodata).
 * Peak levels are -32767 and +32767, avoiding -32768 asymmetry.
 */

static const int16_t sine_table[LUT_SIZE] =
{
         0,    804,   1608,   2410,   3212,   4011,   4808,   5602,
      6393,   7179,   7962,   8739,   9512,  10278,  11039,  11793,
     12539,  13279,  14010,  14732,  15446,  16151,  16846,  17530,
     18204,  18868,  19519,  20159,  20787,  21403,  22005,  22594,
     23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,
     27245,  27683,  28105,  28510,  28898,  29268,  29621,  29956,
     30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
     32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,
     32767,  32757,  32728,  32678,  32609,  32521,  32412,  32285,
     32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,
     30273,  29956,  29621,  29268,  28898,  28510,  28105,  27683,
     27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,
     23170,  22594,  22005,  21403,  20787,  20159,  19519,  18868,
     18204,  17530,  16846,  16151,  15446,  14732,  14010,  13279,
     12539,  11793,  11039,  10278,   9512,   8739,   7962,   7179,
      6393,   5602,   4808,   4011,   3212,   2410,   1608,    804,
         0,   -804,  -1608,  -2410,  -3212,  -4011,  -4808,  -5602,
     -6393,  -7179,  -7962,  -8739,  -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530,
    -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
    -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971,
    -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285,
    -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
    -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
    -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868,
    -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278,  -9512,  -8739,  -7962,  -7179,
     -6393,  -5602,  -4808,  -4011,  -3212,  -2410,  -1608,   -804
};

static const int16_t triangle_table[LUT_SIZE] =
{
    -32767, -32255, -31743, -31231, -30719, -30207, -29695, -29183,
    -28671, -28159, -27647, -27135, -26623, -26111, -25599, -25087,
    -24575, -24063, -23551, -23039, -22527, -22015, -21503, -20991,
    -20479, -19967, -19455, -18943, -18431, -17919, -17407, -16895,
    -16384, -15872, -15360, -14848, -14336, -13824, -13312, -12800,
    -12288, -11776, -11264, -10752, -10240,  -9728,  -9216,  -8704,
     -8192,  -7680,  -7168,  -6656,  -6144,  -5632,  -5120,  -4608,
     -4096,  -3584,  -3072,  -2560,  -2048,  -1536,  -1024,   -512,
         0,    512,   1024,   1536,   2048,   2560,   3072,   3584,
      4096,   4608,   5120,   5632,   6144,   6656,   7168,   7680,
      8192,   8704,   9216,   9728,  10240,  10752,  11264,  11776,
     12288,  12800,  13312,  13824,  14336,  14848,  15360,  15872,
     16384,  16895,  17407,  17919,  18431,  18943,  19455,  19967,
     20479,  20991,  21503,  22015,  22527,  23039,  23551,  24063,
     24575,  25087,  25599,  26111,  26623,  27135,  27647,  28159,
     28671,  29183,  29695,  30207,  30719,  31231,  31743,  32255,
     32767,  32255,  31743,  31231,  30719,  30207,  29695,  29183,
     28671,  28159,  27647,  27135,  26623,  26111,  25599,  25087,
     24575,  24063,  23551,  23039,  22527,  22015,  21503,  20991,
     20479,  19967,  19455,  18943,  18431,  17919,  17407,  16895,
     16384,  15872,  15360,  14848,  14336,  13824,  13312,  12800,
     12288,  11776,  11264,  10752,  10240,   9728,   9216,   8704,
      8192,   7680,   7168,   6656,   6144,   5632,   5120,   4608,
      4096,   3584,   3072,   2560,   2048,   1536,   1024,    512,
         0,   -512,  -1024,  -1536,  -2048,  -2560,  -3072,  -3584,
     -4096,  -4608,  -5120,  -5632,  -6144,  -6656,  -7168,  -7680,
     -8192,  -8704,  -9216,  -9728, -10240, -10752, -11264, -11776,
    -12288, -12800, -13312, -13824, -14336, -14848, -15360, -15872,
    -16384, -16895, -17407, -17919, -18431, -18943, -19455, -19967,
    -20479, -20991, -21503, -22015, -22527, -23039, -23551, -24063,
    -24575, -25087, -25599, -26111, -26623, -27135, -27647, -28159,
    -28671, -29183, -29695, -30207, -30719, -31231, -31743, -32255
};

static const int16_t square_table[LUT_SIZE] =
{
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
     32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767,
    -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767
};


/* RAM buffer used by DMA. Only this module may modify it. */
static uint16_t lut_buffer[LUT_SIZE];
static bool lut_ready = false;

/* Private helper: pick the stored table; DC needs no lookup array. */
static const int16_t *GeneratorLUT_SelectTable(Waveform_t waveform)
{
    switch (waveform)
    {
        case WAVEFORM_SINE:     return sine_table;
        case WAVEFORM_TRIANGLE: return triangle_table;
        case WAVEFORM_SQUARE:   return square_table;
        case WAVEFORM_DC:       return NULL;
        default:                return NULL;
    }
}

/* Convert ideal bipolar output voltage [-5 V, +5 V]
 * into the DAC code [0, 4095] (assumes calibrated analog output stage).
 */
static uint16_t GeneratorLUT_VoltageToDAC(float voltage)
{
    if (voltage < OUTPUT_MIN_VOLTAGE)
    {
        voltage = OUTPUT_MIN_VOLTAGE;
    }
    else if (voltage > OUTPUT_MAX_VOLTAGE)
    {
        voltage = OUTPUT_MAX_VOLTAGE;
    }

    float scaled = ((voltage - OUTPUT_MIN_VOLTAGE) /
                    OUTPUT_RANGE_VOLTAGE) * DAC_FULL_SCALE;
    return (uint16_t)lroundf(scaled);
}

void GeneratorLUT_Init(void)
{
    /* Call after WaveOutput_Init(). */
    WaveOutput_Stop();

    for (uint16_t i = 0U; i < LUT_SIZE; ++i)
    {
        lut_buffer[i] = DAC_MID_SCALE;
    }
    lut_ready = false;
}

bool GeneratorLUT_Build(Waveform_t waveform, float amplitude, float offset)
{
    /* Explicit check is required: NULL also represents the DC waveform. */
    if (waveform != WAVEFORM_DC &&
        waveform != WAVEFORM_SINE &&
        waveform != WAVEFORM_TRIANGLE &&
        waveform != WAVEFORM_SQUARE)
    {
        return false;
    }

    if (!isfinite(amplitude) || !isfinite(offset))
    {
        return false;
    }

    if (amplitude < 0.0f || amplitude > 5.0f ||
        offset < -5.0f || offset > 5.0f ||
        offset + amplitude > OUTPUT_MAX_VOLTAGE ||
        offset - amplitude < OUTPUT_MIN_VOLTAGE)
    {
        return false;
    }

    /* Stop DMA before rewriting the buffer in RAM. */
    WaveOutput_Stop();
    lut_ready = false;

    const int16_t *table = GeneratorLUT_SelectTable(waveform);

    for (uint16_t i = 0U; i < LUT_SIZE; ++i)
    {
        /* DC waveform is normalized to zero and uses offset alone. */
        float normalized = (table == NULL)
                             ? 0.0f
                             : (float)table[i] / NORMALIZED_FULL_SCALE;
        float output_voltage = (amplitude * normalized) + offset;
        lut_buffer[i] = GeneratorLUT_VoltageToDAC(output_voltage);
    }

    lut_ready = true;
    return true;
}

bool GeneratorLUT_Start(float frequency)
{
    if (!lut_ready || !isfinite(frequency) || frequency < GENERATOR_MIN_FREQUENCY_HZ ||
        frequency > GENERATOR_MAX_FREQUENCY_HZ)
    {
        return false;
    }

    /* Frequencies are calculated once on start, not per DAC sample. */
    double sample_rate = (double)frequency * (double)LUT_SIZE;
    if (!isfinite(sample_rate) ||
        sample_rate < 1.0 ||
        sample_rate > ((double)UINT32_MAX - 0.5))
    {
        return false;
    }

    uint32_t sample_rate_hz = (uint32_t)(sample_rate + 0.5);
    return WaveOutput_StartBuffer(lut_buffer, (uint16_t)LUT_SIZE, sample_rate_hz);
}

void GeneratorLUT_Stop(void)
{
    WaveOutput_Stop();
}
