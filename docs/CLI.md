# Serial CLI reference

Use 115200 baud, 8N1, and no flow control. Terminate each command with CR or LF. Commands and tokens are converted to uppercase; numeric exponent notation is accepted. Use a decimal point. A line can contain up to 95 characters; an overlong line or RX overflow causes the affected command to be discarded.

| Command | Effect / condition |
|---|---|
| HELP | Print help |
| STATUS | Print state, source, fault, and requested parameters |
| START | Requires IDLE; starts immediately in REMOTE, waits for stable controls in LOCAL |
| STOP | Stop output; RUNNING → IDLE; does not clear FAULT |
| RESET | Clear FAULT and return to IDLE |
| SOURCE LOCAL | Select potentiometers and encoder; requires IDLE |
| SOURCE REMOTE | Select console control; requires IDLE |
| MODE LUT / MODE MATH | Select backend; requires REMOTE and IDLE |
| WAVE DC / SINE / TRIANGLE / SQUARE | Select waveform; requires REMOTE and IDLE |
| FREQ value | 1–50,000 Hz; requires REMOTE and IDLE |
| AMP value | 0–5 V peak; requires REMOTE, IDLE, and offset headroom |
| OFFSET value | −5 to +5 V; requires REMOTE, IDLE, and amplitude headroom |

Voltages represent the external stage's ideal bipolar scale. The required constraint is `abs(offset) + amplitude <= 5`. AMP specifies peak amplitude, not peak-to-peak; ideal Vpp = 2 × AMP for sine, triangle, and square waves. DC uses offset alone, but the same headroom validation still applies.

## Remote session

```text
STOP
SOURCE REMOTE
MODE LUT
WAVE SINE
FREQ 10
AMP 2
OFFSET 1
START
STATUS
STOP
MODE MATH
START
STOP
```

Begin hardware testing of MATH mode at a low frequency; the configured maximum does not establish its timing capability.

## Changing offset without an intermediate rejection

If the current amplitude is 5 V, `OFFSET 3` is rejected because there is no headroom. Use:

```text
STOP
SOURCE REMOTE
AMP 0
OFFSET 3
AMP 1
START
```

## Local control

```text
STOP
SOURCE LOCAL
START
```

Firmware relearns the potentiometer readings. It requires three matching quantized observations taken every 50 ms; filtering can extend settling time. PC0 now covers 1–50,000 Hz in both backends. See [local mapping and bench procedure](../README_PATCH_50KHZ.md#local-frequency-control). The encoder cycles through DC → SINE → TRIANGLE → SQUARE. LOCAL starts with SINE selected.

## Responses and troubleshooting

Accepted mutation commands return `OK`; rejected commands return `ERR: ...`. HELP and STATUS print information. Reported application faults are NONE, CONTROLS_READ, PARAMETER_UPDATE, and GENERATOR_START.

If FAULT appears, run STATUS, address the cause, run RESET, then configure and START again. STOP does not clear faults. STATUS reports requested settings and logical state; it does not measure voltage/frequency or currently report DAC underrun.
