# 50 kHz firmware patch

REMOTE accepts 1 Hz to 50 kHz. LOCAL uses the same range through PC0. Both backends select the largest supported sample count (256, 128, 64, 32 or 16) that fits the 800 kS/s ceiling when START is issued. This is not DDS. At 50 kHz the nominal plan uses 16 samples per cycle, 800 kS/s and TIM6 PSC=0, ARR=104.

The PLL uses the 16 MHz HSI to produce a 168 MHz system clock. APB1 runs at 42 MHz, TIM3/TIM6 at 84 MHz and APB2 at 84 MHz. LUT decimates its stored tables for circular DMA. MATH fills a 256-sample circular DMA buffer using half-transfer and transfer-complete interrupts and a recursive sine oscillator. At the maximum sample rate each 128-sample half has a 160 microsecond refill deadline.

## Verification

Build with `pio run -e nucleo_f446re` and run `tests_host/run.ps1` with native GCC. On November 2, 2026 the native GCC host suite and PlatformIO nucleo_f446re build passed. Host checks cover the complete 12-bit PC0 range, jitter, transitions, both backends and the REMOTE-to-LOCAL CLI path. Physical output still requires bench validation.

For a bench check, select SOURCE REMOTE in IDLE, set MODE LUT, WAVE SINE, AMP 1 and OFFSET 0. Start at FREQ 1000, then STOP before each frequency change. Test 10, 25 and 50 kHz and repeat in MATH. Check clock, UART, timer registers, DMA interrupts and DAC underrun. Measure frequency, amplitude, offset, distortion and ISR duration on PA4 before testing an external analog stage. At 800 kS/s each sample lasts 1.25 microseconds. Analog settling and usable signal quality remain unmeasured. WaveOutput exposes streaming error status; the application FSM does not yet report it through STATUS.

## Local frequency control

PC0 uses piecewise linear interpolation at ADC codes 0, 819, 1638, 2457,
3276 and 4095 corresponding to 1, 10, 100, 1000, 10000 and 50000 Hz.
Each interval has 819 counts. The steps are respectively 1, 1, 10, 100
and 250 Hz; this allocates most of the travel to low and mid frequencies.
Every 50 ms a foreground IIR (alpha 0.65) filters PC0. A five-count
deadband holds the last accepted ADC position; codes within one count
of either rail snap to that rail. Three consecutive matching effective
settings are required before applying a change. An unchanged configuration
does not restart the output. Amplitude and offset retain their own filters.

To test, STOP, SOURCE REMOTE, MODE LUT, SOURCE LOCAL, START. Turn PC0
from minimum to maximum and back while measuring PA4 with a high-impedance
oscilloscope. Read STATUS at settled positions and hold near a quantization
boundary to check for repeated interruptions. STOP and repeat with MODE MATH
selected in REMOTE before returning to LOCAL. No FREQ command is needed.
At 10 kHz the plan selects 64 samples, with 131 timer ticks per sample:
84 MHz / (131 × 64) ≈ 10019.1 Hz (+0.19%) before HSI tolerance.
At 1 Hz the prescaler accommodates the 256 S/s request. At 50 kHz the
nominal timer division is exact. Measure PA4 amplitude and distortion;
these are not established by the firmware frequency range.
