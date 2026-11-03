# Signal Generator V1 · STM32F446RE

The current firmware targets up to 50 kHz through the remote CLI. The PC0 potentiometer covers the same 1 Hz–50 kHz range. Both waveform backends use DMA and an adaptive sample plan. The NUCLEO-F446RE firmware build and native host tests passed on November 2, 2026. Physical validation remains pending.

See [50 kHz patch and bench procedure](README_PATCH_50KHZ.md) for the current clock, sampling plan, build commands and test sequence. The guides below document the original 100 Hz baseline where their clock, rate or backend details differ.

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


