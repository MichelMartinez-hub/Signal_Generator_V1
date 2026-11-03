# 50 kHz firmware patch

REMOTE accepts 1 Hz to 50 kHz. LOCAL retains its 1–100 Hz potentiometer range. Both backends select the largest supported sample count (256, 128, 64, 32 or 16) that fits the 800 kS/s ceiling when START is issued. This is not DDS. At 50 kHz the nominal plan uses 16 samples per cycle, 800 kS/s and TIM6 PSC=0, ARR=104.

The PLL uses the 16 MHz HSI to produce a 168 MHz system clock. APB1 runs at 42 MHz, TIM3/TIM6 at 84 MHz and APB2 at 84 MHz. LUT decimates its stored tables for circular DMA. MATH fills a 256-sample circular DMA buffer using half-transfer and transfer-complete interrupts and a recursive sine oscillator. At the maximum sample rate each 128-sample half has a 160 microsecond refill deadline.

## Verification

Build with `pio run -e nucleo_f446re` and run `tests_host/run.ps1` with native GCC. The supplied patch notes report passing host tests outside this environment. The patched ARM build and physical output have not been confirmed here. Earlier build confirmation applies to the previous firmware.

For a bench check, select SOURCE REMOTE in IDLE, set MODE LUT, WAVE SINE, AMP 1 and OFFSET 0. Start at FREQ 1000, then STOP before each frequency change. Test 10, 25 and 50 kHz and repeat in MATH. Check clock, UART, timer registers, DMA interrupts and DAC underrun. Measure frequency, amplitude, offset, distortion and ISR duration on PA4 before testing an external analog stage. At 800 kS/s each sample lasts 1.25 microseconds. Analog settling and usable signal quality remain unmeasured. WaveOutput exposes streaming error status; the application FSM does not yet report it through STATUS.
