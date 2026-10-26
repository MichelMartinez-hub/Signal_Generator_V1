# Changelog

## 2026-10-26 — English documentation

- Translated README, technical guides, integration history, changelog, validation issue template, and host-test messages into English.
- Updated the repository description to English.
- Preserved existing documentation paths and firmware behavior.
- Firmware build remains owner-confirmed; physical hardware validation remains pending.

## 2026-10-26 — Initial integration and documentation

### Added

- Previously empty optional binary codec and XOR checksum modules.
- DAC, TIM6, WaveOutput coordination, clock, and scheduler timer improvements.
- LUT/MATH and protocol host tests, a PowerShell runner, and boundary cases.
- Hardware, architecture, CLI, protocol, development, and hardware-validation guides.

### Fixed

- Use of NULL without including stddef.h in generator.c.
- Inconsistent frequency limits: unified to 1–100 Hz in the CLI, controls, API, and both backends.
- Initial frequency reduced to 10 Hz.
- TIM6 counter reset, interval rounding, and support for the 65536-tick interval.
- DAC initialization and underrun clearing.
- Consistency between the configured clock, bus dividers, and SystemCoreClock.

### Validation status

- Firmware build confirmed by the owner.
- Host tests: a successful run has not yet been confirmed.
- Physical hardware validation: pending.

See docs/INTEGRACION.md for integration details. No physically validated firmware release has been published.
