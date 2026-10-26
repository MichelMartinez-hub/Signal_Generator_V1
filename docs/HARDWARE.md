# Hardware y conexiones previstas

Estas conexiones se obtienen de la configuración del firmware. **El montaje físico no está validado.** Confirmar pinout y puentes de la revisión concreta de la NUCLEO antes del ensayo.

| Pin / recurso | Uso en firmware |
|---|---|
| PA4 | DAC canal 1, salida analógica unipolar |
| PC0 / ADC1_IN10 | Potenciómetro de frecuencia |
| PC1 / ADC1_IN11 | Potenciómetro de amplitud |
| PB0 / ADC1_IN8 | Potenciómetro de offset |
| PA0 / AF1 TIM2_CH1 | Encoder A, pull-up interno |
| PA1 / AF1 TIM2_CH2 | Encoder B, pull-up interno |
| PA2 / AF7 USART2_TX | TX serie |
| PA3 / AF7 USART2_RX | RX serie |
| TIM6 | Base de muestreo y trigger del DAC |
| DMA1 Stream5 Channel7 | Memoria → DAC1 |
| TIM3 | Tick del scheduler de 10 ms |

## Entradas

La lectura de potenciómetros presupone códigos ADC de 0–4095 y entradas dentro de 0–3.3 V nominales. Conectar cada cursor al canal indicado y los extremos a la referencia de 3.3 V y GND del montaje. No aplicar 5 V a estas entradas ADC. Verificar referencias y masa común antes de conectar instrumentos o una etapa externa.

El encoder se lee como delta del contador de TIM2. El firmware aplica filtros digitales y espera cuatro cuentas por paso lógico. El sentido y las cuentas por detente dependen del encoder y del cableado; comprobar ambos en banco.

## DAC y etapa externa

La conversión ideal implementada es:

`código DAC = round((Vobjetivo + 5) * 4095 / 10)`

Así, −5 V corresponde a 0, 0 V a aproximadamente 2048 y +5 V a 4095. Estos valores de tensión son objetivos **después de una etapa analógica bipolar calibrada**. El DAC de PA4 permanece entre sus referencias de alimentación, nominalmente 0–3.3 V; su buffer y carga afectan el recorrido útil real.

STOP escribe 2048: corresponde aproximadamente a media escala en PA4 y solo representa 0 V bipolar si la etapa externa está correctamente ajustada.

El repositorio no contiene un esquema eléctrico aprobado, valores de componentes, diseño de PCB ni calibración de la etapa analógica. Cualquier etapa TL072 u otra debe diseñarse y verificarse aparte; no se afirma que entregue ±5 V por el solo hecho de compilar este firmware.

## Reloj y enlace serie

System_Init selecciona HSI de 16 MHz, sin PLL, con AHB/APB1/APB2 sin división; después actualiza SystemCoreClock. Las constantes en board_config.h deben mantenerse consistentes con ese reloj.

USART2: 115200 baudios, 8 bits, sin paridad, 1 stop bit, sin control de flujo. El uso mediante ST-Link VCP depende de los puentes de la placa. No conectar directamente una interfaz RS-232 de niveles bipolares a PA2/PA3.

La frecuencia nominal del HSI y la cuantización de TIM6 no sustituyen una medición de frecuencia real.
