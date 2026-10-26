# Development and testing

## Environment

- PlatformIO Core or the PlatformIO IDE extension for VS Code.
- Target board: NUCLEO-F446RE, with the CMSIS framework and ststm32 platform.
- Optional native GCC for tests_host. The ARM firmware compiler does not produce host executables.

Open the folder containing platformio.ini. Do not commit .pio or generated IDE files. The platform version is not pinned in platformio.ini; record tool versions when reproducing validation results.

## Build, upload, and serial console

```sh
pio run -e nucleo_f446re
pio run -e nucleo_f446re -t upload
pio device list
pio device monitor --baud 115200 --port COMx
```

COMx is a placeholder; replace it with the actual port. Upload and serial monitoring require hardware and remain **pending physical validation**. If pio is not on PATH, use the VS Code PlatformIO terminal. For a clean rebuild, run `pio run -e nucleo_f446re -t clean`, then Build.

## Host tests

Linux/macOS with a C compiler and libm:

```sh
sh tests_host/run.sh
```

Windows with native GCC:

```powershell
powershell -ExecutionPolicy Bypass -File tests_host/run.ps1
# Or select a compatible GCC explicitly:
.\tests_host\run.ps1 -Compiler 'C:\path\to\gcc.exe'
```

WaveOutput is replaced with simulated functions. Tests exercise tables, mathematical samples, parameter limits, backend selection, and the protocol. They do not execute real peripherals, interrupts, ADC, DMA, or electrical behavior, and do not cover the entire App/CLI flow. They use assert; do not compile them with NDEBUG.

**Status:** the suite is prepared, but a successful run has not yet been confirmed. Firmware compilation was confirmed by the owner on October 26, 2026.

## Troubleshooting

| Symptom | Check |
|---|---|
| FREQ rejected | Select REMOTE and IDLE; use a finite value between 1 and 100 |
| AMP/OFFSET rejected | Check combined headroom; reduce AMP before moving OFFSET |
| START rejected | Check STATUS; stop, or resolve and reset FAULT |
| No CLI response | Check port, 115200 8N1, cable, VCP routing/bridges, and ground |
| No signal in LOCAL | Confirm START and stable readings; check ADC and wiring |
| STATUS RUNNING without a measured waveform | State is logical; measure the DAC and check mode/hardware |
| Compiler fails to start with a DLL error | This is a Windows/toolchain issue before source compilation; use the working PlatformIO environment |

## Future changes

Keep register-level drivers in BSP and application decisions in App. Stop output before modifying buffers or settings shared with ISR/DMA. Avoid duplicate main functions or interrupt handlers. Update documentation and tests when changing limits, pins, protocols, or states. Measure MATH execution time before increasing frequencies.
