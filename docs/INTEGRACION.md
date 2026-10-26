# Integración de Signal_Generator_V1

Fecha: 26 de octubre de 2026.
Proyecto actualizado directamente: Signal_Generator_V1
Origen: Signal_Generator_V1_COMPLETO.zip.

## Archivos y código integrados

- Añadidos README.md, docs/ARCHITECTURE.md y tests_host/.
- Completados los cuatro archivos vacíos de protocolo: protocol.c/.h y crc8.c/.h. El checksum es XOR heredado; el protocolo binario sigue siendo opcional y no se ha conectado a la CLI.
- Incorporadas las mejoras funcionales del ZIP en DAC, TIM6, WaveOutput, reloj y temporizador del scheduler.
- Conservados los módulos locales equivalentes, comentarios, APIs y constantes públicas. No se copiaron versiones alternativas de main, ISR, librerías ni una carpeta de proyecto anidada.
- Conservados placa, plataforma y framework de platformio.ini; añadidas opciones C11 y advertencias. .gitignore combina las reglas previas y las del ZIP.

## Correcciones

1. **NULL sin definición en generator.c:** añadido <stddef.h>, evitando depender de inclusiones indirectas.
2. **Frecuencias inconsistentes:** antes la CLI y los controles admitían 1–100 Hz, generator_config.h declaraba hasta 10000 Hz y las API aceptaban cualquier frecuencia positiva. Ahora CLI, controles, Generator y los dos backends usan el mismo rango de 1–100 Hz y rechazan NaN, infinito y valores fuera de rango antes de iniciar la salida. Se conservan las constantes públicas de amplitud y offset. Este cambio reduce expresamente el máximo anunciado por la configuración anterior; no certifica rendimiento de MATH.
3. **Frecuencia inicial:** reducida de 100 Hz a 10 Hz, conforme al paquete integrado.
4. **TIM6:** reinicia CNT al configurar la frecuencia, redondea al intervalo más cercano, acepta el intervalo válido de 65536 ticks y limpia IRQ/callback al inicializar. Evita arranques con contador residual y la exclusión incorrecta del extremo superior de ARR.
5. **DAC y cambios de modo:** inicializa la salida a media escala, deshabilita la interrupción de underrun no atendida y limpia la bandera de underrun al inicializar/detener. WaveOutput detiene primero el temporizador y la IRQ, desactiva DMA/trigger y limpia el estado antes de reiniciar. Esto permite recuperar un arranque tras underrun; no añade detección de fallo en la FSM.
6. **Reloj:** configura explícitamente los divisores AHB/APB en 1 y actualiza SystemCoreClock para concordar con las constantes de 16 MHz; antes solo seleccionaba HSI.
7. **Scheduler:** su frecuencia de interrupción se deriva de SCHEDULER_TIMER_TICK_MS y se comprueban los divisores en compilación, en vez de mantener un valor separado fijo de 100 Hz.
8. **Pruebas:** añadidas las del ZIP, ampliadas para límites de frecuencia, NULL, tramas vacías, payload máximo, checksum incorrecto, capacidad insuficiente y tramas truncadas. Añadido tests_host/run.ps1 para Windows con GCC.

## Validación y límites reales

**Actualización del propietario (26-10-2026): el proyecto compiló correctamente. La validación física permanece pendiente.** El diagnóstico de DLL que sigue describe únicamente el intento histórico del agente; no representa el estado actual de compilación del proyecto. Las pruebas de host siguen sin una ejecución satisfactoria confirmada.

La revisión estática pasó:
- main: definición única
- TIM3_IRQHandler: definición única
- TIM6_DAC_IRQHandler: definición única
- USART2_IRQHandler: definición única
- Includes internos: todos resueltos
- Macros de registros añadidas: presentes en CMSIS instalado
- No se eliminó ningún archivo original

**Compilación ARM: no completada.** Se ejecutó PlatformIO para nucleo_f446re; resolvió la plataforma ST STM32 19.7.1, CMSIS y GCC ARM 7.2.1, pero Windows no pudo iniciar el compilador: `{Illegal System DLL Relocation}` / `The system cannot execute the specified program`. Falló al compilar los archivos de arranque de CMSIS, antes de compilar el código del proyecto. Las herramientas alternativas instaladas de STM32CubeIDE y Clang fallaron al iniciarse con el mismo error de DLL. No se reemplazó ninguna instalación.

**Pruebas de host: no ejecutadas.** El GCC de MinGW local también falla al iniciar con `{Illegal System DLL Relocation}`. Los tests quedan preparados, sin afirmar que pasaron.

Para verificar tras resolver el entorno:

```powershell
cd Signal_Generator_V1
& pio run
powershell -ExecutionPolicy Bypass -File tests_host/run.ps1
```

No se cargó firmware ni se probó una placa. Sigue pendiente comprobar las señales con osciloscopio y medir la carga de MATH: sinf() por muestra a 16 MHz puede exceder el presupuesto de la ISR, especialmente cerca de 100 Hz de onda. El DAC PA4 entrega una señal unipolar; la escala bipolar supone la etapa analógica externa indicada en el README. La FSM aún no informa underrun de DAC.

## Inventario exacto de cambios

- modificado: `.gitignore`
- modificado: `.vscode/c_cpp_properties.json`
- añadido: `docs/ARCHITECTURE.md`
- modificado: `include/bsp/dac.h`
- modificado: `include/config/board_config.h`
- modificado: `include/config/generator_config.h`
- modificado: `include/protocol/crc8.h`
- modificado: `include/protocol/protocol.h`
- modificado: `platformio.ini`
- añadido: `README.md`
- modificado: `src/app/parameter_manager.c`
- modificado: `src/bsp/dac.c`
- modificado: `src/bsp/sample_timer.c`
- modificado: `src/bsp/scheduler_timer.c`
- modificado: `src/bsp/system.c`
- modificado: `src/bsp/wave_output.c`
- modificado: `src/generator/generator.c`
- modificado: `src/generator/generator_lut.c`
- modificado: `src/generator/generator_math.c`
- modificado: `src/protocol/cli.c`
- modificado: `src/protocol/crc8.c`
- modificado: `src/protocol/protocol.c`
- añadido: `tests_host/run.ps1`
- añadido: `tests_host/run.sh`
- añadido: `tests_host/test_generators.c`
- añadido: `docs/INTEGRACION.md` (este informe).
