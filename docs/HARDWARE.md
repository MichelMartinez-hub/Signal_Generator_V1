# Hardware and planned connections

These assignments come from the firmware configuration. **The physical assembly has not been validated.** Check the pinout and solder bridges of the specific NUCLEO board revision before testing.

| Pin / resource | Firmware assignment |
|---|---|
| PA4 | DAC channel 1, unipolar analog output |
| PC0 / ADC1_IN10 | Frequency potentiometer |
| PC1 / ADC1_IN11 | Amplitude potentiometer |
| PB0 / ADC1_IN8 | Offset potentiometer |
| PA0 / AF1 TIM2_CH1 | Encoder A, internal pull-up |
| PA1 / AF1 TIM2_CH2 | Encoder B, internal pull-up |
| PA2 / AF7 USART2_TX | Serial TX |
| PA3 / AF7 USART2_RX | Serial RX |
| TIM6 | Sampling timebase and DAC trigger |
| DMA1 Stream5 Channel7 | Memory → DAC1 |
| TIM3 | 10 ms scheduler tick |

## Inputs

Potentiometer readings assume ADC codes of 0–4095 and inputs within a nominal 0–3.3 V range. Connect each wiper to its assigned channel and its endpoints to the assembly's 3.3 V reference and GND. Do not apply 5 V to these ADC inputs. Check voltage references and the common ground before connecting instruments or an external stage.

The encoder is read as the TIM2 counter delta. Firmware enables digital input filters and expects four counts per logical step. Direction and counts per detent depend on the encoder and wiring; verify both on the bench.

## DAC and external analog stage

The implemented ideal conversion is:

`DAC code = round((target voltage + 5) * 4095 / 10)`

Thus, −5 V maps to 0, 0 V to approximately 2048, and +5 V to 4095. These voltage values are targets **after a calibrated external bipolar analog stage**. PA4 remains between the DAC supply references, nominally 0–3.3 V; the output buffer and load affect its actual usable range.

STOP writes 2048, approximately midscale at PA4. This only represents a bipolar output of 0 V when the external stage is correctly adjusted.

The repository does not include an approved electrical schematic, component values, PCB design, or analog-stage calibration. Any TL072 or other external stage must be designed and verified separately; compiling this firmware does not establish a ±5 V output.

## Clocks and serial link

System_Init selects the 16 MHz HSI, disables PLL use, leaves AHB/APB1/APB2 undivided, and updates SystemCoreClock. Constants in board_config.h must remain consistent with this configuration.

USART2 uses 115200 baud, 8 data bits, no parity, one stop bit, and no flow control. ST-Link VCP routing depends on the board's solder bridges. Do not connect a bipolar-voltage RS-232 interface directly to PA2/PA3.

The nominal HSI frequency and TIM6 interval calculation do not replace a measurement of actual waveform frequency.
