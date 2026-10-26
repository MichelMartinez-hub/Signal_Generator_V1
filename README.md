# Signal Generator V1 · STM32F446RE

A signal generator written in **C11**, using PlatformIO and CMSIS with direct register access, without HAL or an RTOS. It provides LUT/DMA and real-time mathematical waveform generation, local controls, and a serial console.

**Status as of October 26, 2026:** firmware compilation completed and confirmed by the owner. **Physical hardware validation is pending.** A successful build does not verify frequency, amplitude, stability, or signal quality. Host tests are included, but a successful test run has not yet been confirmed.

## Features

- DC, sine, triangle, and square waveforms.
- LUT mode: 256-sample precomputed tables with circular DMA transfer.
- MATH mode: each sample is evaluated in the TIM6 interrupt handler.
- Configurable waveform frequency: 1–100 Hz; a provisional limit, particularly for MATH mode.
- Peak amplitude of 0–5 V and offset of −5 to +5 V, subject to `abs(offset) + amplitude <= 5 V`.
- LOCAL control using three potentiometers and an encoder; REMOTE control through the CLI.
- An application FSM with INIT, IDLE, RUNNING, and FAULT states.

Bipolar voltage units describe the **ideal output after a calibrated external analog stage**. PA4 is the unipolar DAC output referenced to VDDA; it cannot produce a negative voltage by itself.

## Build

Open this folder in PlatformIO IDE for VS Code and select Build, or run:

```sh
pio run -e nucleo_f446re
```

The configuration uses `board = nucleo_f446re`, `framework = cmsis`, and `platform = ststm32`. The firmware configures the HSI clock at **16 MHz**, even though the board supports higher clock rates. No external application libraries are required.

See the [development and testing guide](docs/DESARROLLO.md) for setup, upload, serial monitoring, and host tests.

## Serial console example

Use **115200 baud, 8N1**, with no flow control. Terminate commands with CR or LF:

```text
STATUS
SOURCE REMOTE
MODE LUT
WAVE SINE
FREQ 10
AMP 2
OFFSET 1
START
STATUS
STOP
```

The ideal expected output is a 10 Hz sine wave with a 2 V peak amplitude and a 1 V offset, after the analog stage. **This is an expected configuration, not a recorded measurement.**

Remote parameters can only be changed in IDLE. Before applying amplitude or offset values that conflict with the current configuration, reduce the amplitude first. See the [CLI reference](docs/CLI.md).

## Documentation

| Document | Contents |
|---|---|
| [Architecture](docs/ARCHITECTURE.md) | Modules, data flow, interrupts, and FSM |
| [Hardware connections](docs/HARDWARE.md) | Pin assignments, clocks, and analog scaling |
| [CLI reference](docs/CLI.md) | Commands, states, examples, and errors |
| [Binary protocol](docs/PROTOCOLO.md) | Optional frames and XOR checksum |
| [Development](docs/DESARROLLO.md) | Build, upload, tests, and troubleshooting |
| [Pending hardware validation](docs/VALIDACION.md) | Bench test plan and measurement records |
| [Integration history](docs/INTEGRACION.md) | Initial changes and fixes |
| [Changelog](CHANGELOG.md) | Documented project history |

Existing documentation filenames are retained to preserve links; all document contents are in English.

## Project structure

```text
include/       Public interfaces and configuration
src/app/       FSM, scheduler, controls, and parameter management
src/bsp/       Register-level peripheral drivers
src/generator/ LUT, MATH, and waveform evaluation
src/protocol/  CLI and optional binary protocol
src/utils/     Filtering and mapping
tests_host/    Tests with simulated WaveOutput functions
docs/          Technical documentation and validation plan
platformio.ini Build configuration
```

## Known limitations

- MATH calls `sinf()` in the ISR. At 100 Hz and 256 samples per cycle, the requested sample rate is 25.6 kHz. Whether this fits the ISR time budget at 16 MHz still needs measurement. Start bench testing at 1–10 Hz.
- Parameter changes stop and restart the output, which may introduce discontinuities. Double buffering is not implemented.
- DAC underrun is cleared when stopping, but is not reported to the FSM or necessarily reflected in STATUS.
- Actual frequencies are quantized by the timer; STATUS reports the requested frequency.
- The binary protocol is not connected to the UART or CLI.
- No physical measurements of accuracy, THD, output loading, calibration, or output voltage are available.

A distribution license has not yet been selected for this repository.
