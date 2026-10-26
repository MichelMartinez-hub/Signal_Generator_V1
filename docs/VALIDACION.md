# Validation status and bench test plan

**Physical hardware validation is pending. No measurement results have been recorded.**

## Available evidence

| Check | Status on 2026-10-26 | Evidence |
|---|---|---|
| Firmware build | Confirmed by the owner | Confirmation in conversation; build log not archived |
| Integration static review | Completed | Internal includes, unique symbols, and CMSIS macros checked |
| tests_host suite | Successful execution not yet confirmed | Test sources available; compilers failed to start in the agent environment |
| ST-Link upload | Pending | No record |
| On-board CLI | Pending | No record |
| ADC and encoder | Pending | No record |
| LUT and MATH output | Pending | No measurements |
| Analog stage / calibration | Pending | No validated schematic or measurements |

## Preparation

Record board revision, exact commit, PlatformIO/platform/toolchain versions, power supply, external-stage schematic, and instruments. Check electrical levels and references using HARDWARE.md. Start with low frequency and amplitude, and measure PA4 before evaluating the bipolar stage.

## Proposed sequence

- [ ] Rebuild and archive the log, tool versions, and commit.
- [ ] Run tests_host and save the complete output.
- [ ] Upload through ST-Link and check repeatable startup.
- [ ] Check HELP, STATUS, SOURCE, START, STOP, and IDLE/REMOTE restrictions.
- [ ] Test FAULT recovery where the cause can be introduced in a controlled manner; RESET alone is not evidence that the underlying cause was resolved.
- [ ] Measure PA4 midscale after STOP; distinguish this from the external bipolar zero.
- [ ] Test DC through offset, and sine/triangle/square in LUT mode at 1 and 10 Hz.
- [ ] Record actual frequency, minimum/maximum levels, offset, and Vpp.
- [ ] Repeat in MATH mode at 1 and 10 Hz, observing latency and sample regularity.
- [ ] Increase gradually to 100 Hz only after checking ISR timing and stability.
- [ ] Check LUT/MATH transitions, STOP/START, and configuration changes.
- [ ] Validate all three potentiometers: endpoints, noise, settling, and combined limits.
- [ ] Validate encoder direction and steps, including all four waveform selections.
- [ ] Check CLI behavior with long lines, invalid commands, and continuous reception.
- [ ] Measure CPU/ISR load and look for DAC underrun; STATUS does not currently detect it.
- [ ] Calibrate and characterize the external stage with its intended load.

## Per-test record

| Field | Value |
|---|---|
| Date and operator | Pending |
| Commit and tool versions | Pending |
| Board / schematic / power supply | Pending |
| Instrument and configuration | Pending |
| Mode / waveform / requested frequency | Pending |
| Requested amplitude / offset | Pending |
| Measured frequency / Vmin / Vmax / Vpp / offset | Pending |
| Load and measurement point | Pending |
| Screenshot / log | Pending |
| Result and issues | Pending |

Define acceptance tolerances before closing tests. Accuracy, THD, and load capability targets have not been specified; nominal values must not be treated as guarantees. Close hardware validation only with traceable results identifying the commit and instruments.
