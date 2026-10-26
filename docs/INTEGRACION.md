# Signal_Generator_V1 integration history

Date: October 26, 2026. Project: Signal_Generator_V1. Source package: Signal_Generator_V1_COMPLETO.zip.

## Integrated files and code

- Added README.md, docs/ARCHITECTURE.md, and tests_host/.
- Completed the four empty protocol files: protocol.c/.h and crc8.c/.h. The checksum is inherited XOR; the binary protocol remains optional and is not connected to the CLI.
- Incorporated functional ZIP improvements for DAC, TIM6, WaveOutput, clock setup, and the scheduler timer.
- Preserved equivalent local modules, comments, APIs, and public constants. No alternative main functions, interrupt handlers, libraries, or nested project copy were added.
- Preserved the board, platform, and framework in platformio.ini; added C11 and compiler warning flags. .gitignore combines existing and package rules.

## Fixes

1. **Undefined NULL in generator.c:** added <stddef.h> rather than relying on indirect includes.
2. **Inconsistent frequency limits:** the CLI and controls previously accepted 1–100 Hz, generator_config.h declared up to 10000 Hz, and APIs accepted any positive frequency. The CLI, controls, Generator, and both backends now share 1–100 Hz limits and reject NaN, infinity, and out-of-range values before starting. Public amplitude and offset constants were retained. This explicitly reduces the maximum previously advertised by configuration; it does not certify MATH performance.
3. **Initial frequency:** reduced from 100 Hz to 10 Hz, matching the integrated package.
4. **TIM6:** resets CNT when setting frequency, rounds to the nearest interval, accepts the valid 65536-tick interval, and clears IRQ/callback state during initialization. This avoids residual-counter starts and an incorrect exclusion of the upper ARR endpoint.
5. **DAC and mode changes:** initializes output at midscale, disables the unhandled underrun interrupt, and clears underrun on initialization/stop. WaveOutput stops the timer and IRQ, disables DMA/trigger, and clears state before restarting. This allows restart after an underrun; it does not add FSM fault detection.
6. **Clock:** explicitly sets AHB/APB dividers to 1 and updates SystemCoreClock to match the 16 MHz constants; the previous code only selected HSI.
7. **Scheduler:** derives interrupt frequency from SCHEDULER_TIMER_TICK_MS and checks dividers at compile time instead of maintaining a separate fixed 100 Hz value.
8. **Tests:** incorporated package tests and expanded coverage for frequency limits, NULL, empty frames, maximum payload, invalid checksum, insufficient capacity, and truncated frames. Added tests_host/run.ps1 for Windows with GCC.

## Current validation status

**Owner update (2026-10-26): the project compiled successfully. Physical hardware validation remains pending.** A successful host-test run has not yet been confirmed.

The integration static review checked:

- One definition each of main, TIM3_IRQHandler, TIM6_DAC_IRQHandler, and USART2_IRQHandler.
- All internal includes resolved.
- Added register macros present in installed CMSIS headers.
- No original project files deleted.

### Historical agent environment diagnostics

The initial agent-side ARM build could not complete. PlatformIO resolved ST STM32 19.7.1, CMSIS, and GCC ARM 7.2.1, but Windows failed to launch the compiler with `{Illegal System DLL Relocation}` / `The system cannot execute the specified program`. The failure occurred while compiling CMSIS startup files, before compiling project code. Installed STM32CubeIDE and Clang alternatives failed to launch with the same DLL issue. No tool installations were replaced.

The native MinGW GCC also failed to start, preventing host-test execution. These historical environment failures do not override the owner's later confirmation of a successful firmware build.

To reproduce software checks in a working environment:

```powershell
cd Signal_Generator_V1
pio run
powershell -ExecutionPolicy Bypass -File tests_host/run.ps1
```

No firmware was uploaded or board tested by the agent. Oscilloscope checks and MATH load measurements remain pending: sinf() per sample at 16 MHz may exceed the ISR budget, particularly near a 100 Hz waveform. PA4 is unipolar; bipolar scaling assumes the external analog stage described in README. The FSM still does not report DAC underrun.

## Initial integration inventory

Modified: .gitignore, the local .vscode/c_cpp_properties.json, include/bsp/dac.h, include/config/board_config.h, include/config/generator_config.h, include/protocol/crc8.h, include/protocol/protocol.h, platformio.ini, src/app/parameter_manager.c, src/bsp/dac.c, src/bsp/sample_timer.c, src/bsp/scheduler_timer.c, src/bsp/system.c, src/bsp/wave_output.c, src/generator/generator.c, src/generator/generator_lut.c, src/generator/generator_math.c, src/protocol/cli.c, src/protocol/crc8.c, and src/protocol/protocol.c.

Added: README.md, docs/ARCHITECTURE.md, docs/INTEGRACION.md, tests_host/run.ps1, tests_host/run.sh, and tests_host/test_generators.c. The local .vscode configuration is excluded from Git.

Additional technical guides and a validation issue template were added when preparing the GitHub repository. Their English translation changes documentation and tool messages only; firmware behavior remains unchanged.
