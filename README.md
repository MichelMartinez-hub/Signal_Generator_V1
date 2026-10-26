# Signal Generator V1 · STM32F446RE

Generador de señales en **C11**, PlatformIO y CMSIS, con acceso directo a registros, sin HAL ni RTOS. Ofrece generación por LUT/DMA y cálculo de muestras en tiempo real, control local y consola serie.

**Estado al 26 de octubre de 2026:** compilación completada y confirmada por el propietario. **Validación física pendiente**. La compilación no confirma frecuencia, amplitud, estabilidad ni calidad de señal. Las pruebas de host están incluidas; su ejecución todavía no está confirmada.

## Funciones

- Formas de onda DC, seno, triángulo y cuadrada.
- Modo LUT: tablas precalculadas de 256 muestras y transferencia circular por DMA.
- Modo MATH: evaluación de cada muestra desde la interrupción de TIM6.
- Frecuencia configurada: 1–100 Hz; límite provisional, especialmente en MATH.
- Amplitud de pico de 0–5 V y offset de −5 a +5 V, sujetos a `abs(offset) + amplitud <= 5 V`.
- Control LOCAL por tres potenciómetros y encoder; control REMOTE mediante CLI.
- FSM con estados INIT, IDLE, RUNNING y FAULT.

Las unidades bipolares describen la **salida ideal después de una etapa analógica externa calibrada**. PA4 es la salida unipolar del DAC, referida a VDDA; no entrega tensión negativa por sí sola.

## Compilar

Abre esta carpeta en PlatformIO (VS Code) y ejecuta Build, o utiliza:

```sh
pio run -e nucleo_f446re
```

La configuración utiliza `board = nucleo_f446re`, `framework = cmsis` y `platform = ststm32`. El firmware configura HSI a **16 MHz**, aunque la placa admita una frecuencia máxima mayor. No se requieren librerías de aplicación externas.

Consulta [desarrollo y pruebas](docs/DESARROLLO.md) para instalación, carga, monitor serie y pruebas de host.

## Ejemplo de consola

Puerto serie a **115200, 8N1**, sin control de flujo. Comandos terminados con CR o LF:

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

La salida esperada ideal del ejemplo es una senoide de 10 Hz, 2 V de pico y offset de 1 V, después de la etapa analógica. **No es una medición realizada.**

Los parámetros remotos se editan en IDLE. Antes de cambiar amplitud u offset a valores incompatibles con la configuración actual, reduce primero la amplitud. Más detalles en la [referencia CLI](docs/CLI.md).

## Documentación

| Documento | Contenido |
|---|---|
| [Arquitectura](docs/ARCHITECTURE.md) | Módulos, flujo, interrupciones y FSM |
| [Conexiones](docs/HARDWARE.md) | Pines, reloj y escala analógica |
| [CLI](docs/CLI.md) | Comandos, estados, ejemplos y errores |
| [Protocolo binario](docs/PROTOCOLO.md) | Tramas opcionales y checksum XOR |
| [Desarrollo](docs/DESARROLLO.md) | Build, carga, pruebas y diagnóstico |
| [Validación física pendiente](docs/VALIDACION.md) | Plan de banco y registro de resultados |
| [Historial de integración](docs/INTEGRACION.md) | Cambios y correcciones iniciales |
| [Changelog](CHANGELOG.md) | Evolución documentada |

## Estructura

```text
include/       Interfaces y configuración
src/app/       FSM, scheduler, controles y parámetros
src/bsp/       Drivers de periféricos por registros
src/generator/ LUT, MATH y formas de onda
src/protocol/  CLI y protocolo binario opcional
src/utils/     Filtrado y mapeo
tests_host/    Pruebas con WaveOutput simulado
docs/          Documentación técnica y plan de validación
platformio.ini Configuración de compilación
```

## Limitaciones conocidas

- MATH ejecuta `sinf()` en la ISR: a 100 Hz y 256 muestras/ciclo solicita 25.6 kHz de muestreo. Falta medir si cumple el presupuesto de tiempo a 16 MHz. Comenzar las pruebas de banco con 1–10 Hz.
- Los cambios de parámetros detienen y reinician la salida; puede haber discontinuidades. No hay doble buffer.
- El underrun del DAC se limpia al detener; no se comunica a la FSM ni se refleja necesariamente en STATUS.
- Las frecuencias reales están cuantizadas por el temporizador; STATUS muestra la frecuencia solicitada.
- El protocolo binario no está conectado a la UART ni a la CLI.
- No hay resultados físicos de precisión, THD, carga, calibración o tensión de salida.

No se ha elegido una licencia de distribución para este repositorio.
