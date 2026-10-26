# Architecture

## Main execution flow

`main()` initializes System, GPIO, App, and CLI. The main loop runs `App_Run()` and `CLI_Run()` cooperatively.

```mermaid
flowchart TD
    Main[main] --> App[App / FSM]
    Main --> CLI[CLI]
    CLI --> App
    CLI <--> UART[USART2 / RX buffer]
    App --> Scheduler[Scheduler / TIM3]
    Scheduler --> Controls[Controls / ADC and encoder]
    Controls --> PM[ParameterManager]
    PM --> Generator[Generator]
    App --> Generator
    Generator --> LUT[GeneratorLUT]
    Generator --> Math[GeneratorMath]
    LUT --> Output[WaveOutput]
    Math --> Output
    Output --> DAC[DAC1 / TIM6 / DMA1]
```

## Module responsibilities

| Module | Contract |
|---|---|
| App | Coordinates the FSM, LOCAL/REMOTE ownership, and application faults |
| FSM | Validates state transitions; STOP does not clear FAULT |
| Scheduler | Reads TIM3 ticks and runs controls every 50 ms in foreground; skips missed periods |
| Controls | Reads frequency/amplitude/offset ADC channels and encoder delta |
| ParameterManager | Filters, quantizes, waits for three matching observations, then applies stable settings |
| Generator | Provides shared configuration, validation, and backend selection |
| GeneratorLUT | Owns constant tables and a private RAM buffer; stops output before modifying the buffer |
| GeneratorMath | Owns phase state and the sample computation callback |
| WaveOutput | Coordinates DAC, TIM6, and DMA; excludes simultaneous DMA and ISR generation |
| CLI | Parses text in foreground; the USART2 IRQ only captures received data |
| Protocol / CRC8 | Standalone binary codec, not integrated with the CLI |

## States

```mermaid
stateDiagram-v2
    [*] --> INIT
    INIT --> IDLE: initialization complete
    IDLE --> RUNNING: START accepted
    RUNNING --> IDLE: STOP
    INIT --> FAULT: failure
    IDLE --> FAULT: failure
    RUNNING --> FAULT: failure
    FAULT --> IDLE: RESET
```

In REMOTE mode, START begins generation with the configured settings. In LOCAL mode, START arms the application and stable readings start the waveform later; RUNNING alone does not prove the DAC is already generating. Changing SOURCE or editing remote parameters requires IDLE. STOP leaves FAULT active; RESET is only accepted in FAULT.

## Sampling and controls

- TIM6 uses a rounded interval: `ticks = round(16 MHz / Fs)` and `ARR = ticks - 1`, with PSC = 0. The implementation accepts 2–65536 ticks.
- Both backends request `Fs = round(frequency * 256)`. The actual waveform frequency depends on the integer interval accepted by TIM6.
- LUT transfers samples from RAM to the DAC through DMA1 Stream5 Channel7 in circular mode.
- MATH uses the TIM6 IRQ and a callback to prepare the next DAC sample. Latency and initial phase still require bench measurement.
- TIM3 generates 10 ms ticks. Controls are polled every five ticks; ADC conversion and application logic do not run inside the TIM3 IRQ.
- Potentiometer readings use an IIR filter with alpha 0.18 and quantization of 1 Hz / 0.1 V. Three stable readings reduce noise-induced changes; this does not establish immunity to real input noise.
- The encoder cycles through DC, SINE, TRIANGLE, and SQUARE, using four counts per logical step.

## Concurrency

Parameters and buffers are changed in foreground after stopping the output. TIM3 updates a volatile 32-bit tick counter. USART2 uses a 128-byte RX ring buffer with one reserved slot. TIM6 calls the sample provider only in MATH mode. Each interrupt handler must have exactly one definition.

Configured interrupt priorities are TIM6 = 5, TIM3 = 6, and USART2 = 7. Timing and sample loss require hardware validation; these priorities do not demonstrate that real-time deadlines are met.
