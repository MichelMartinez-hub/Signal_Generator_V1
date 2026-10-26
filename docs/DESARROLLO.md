# Desarrollo y pruebas

## Entorno

- PlatformIO Core o extensión PlatformIO IDE en VS Code.
- Placa objetivo NUCLEO-F446RE; framework CMSIS y plataforma ststm32.
- GCC nativo opcional para tests_host. El compilador ARM del firmware no genera ejecutables de host.

Abrir la carpeta que contiene platformio.ini. No incluir .pio ni archivos generados del IDE en commits. La versión de plataforma no está fijada en platformio.ini: registrar versiones al repetir una validación.

## Compilar, cargar y abrir consola

```sh
pio run -e nucleo_f446re
pio run -e nucleo_f446re -t upload
pio device list
pio device monitor --baud 115200 --port COMx
```

COMx es un ejemplo: reemplazarlo por el puerto real. Carga y monitor requieren hardware y están **pendientes de validación física**. Si pio no está en PATH, usar la terminal PlatformIO de VS Code. Para una reconstrucción limpia: `pio run -e nucleo_f446re -t clean` y después Build.

## Pruebas de host

Linux/macOS con compilador C y libm:

```sh
sh tests_host/run.sh
```

Windows con GCC nativo:

```powershell
powershell -ExecutionPolicy Bypass -File tests_host/run.ps1
# O seleccionar explícitamente un GCC compatible:
.	ests_hostun.ps1 -Compiler 'C:uta\gcc.exe'
```

WaveOutput se sustituye por funciones simuladas. Las pruebas ejercitan tablas, muestras matemáticas, límites de parámetros, selección de backend y protocolo. No ejecutan periféricos, interrupciones reales, ADC, DMA ni comportamiento eléctrico; tampoco cubren todo el flujo App/CLI. Usan assert: no compilarlas con NDEBUG.

**Estado:** suite preparada; no hay una ejecución satisfactoria confirmada todavía. La compilación del firmware sí fue confirmada por el propietario el 26 de octubre de 2026.

## Diagnóstico

| Síntoma | Comprobación |
|---|---|
| FREQ rechazado | Usar REMOTE e IDLE; valor finito entre 1 y 100 |
| AMP/OFFSET rechazado | Revisar margen combinado; reducir AMP antes de mover OFFSET |
| START rechazado | Revisar STATUS; detener o resolver y resetear FAULT |
| Sin CLI | Revisar puerto, 115200 8N1, cable, VCP/puentes y masa |
| Sin señal en LOCAL | Confirmar START y lecturas estables; revisar ADC y conexiones |
| STATUS RUNNING sin señal real | El estado es lógico; medir DAC y revisar modo/hardware |
| Error de arranque del compilador por DLL | Problema de herramientas/Windows, anterior a compilar el código; probar Build en el entorno PlatformIO funcional |

## Cambios futuros

Mantener los drivers de registros en BSP y las decisiones de aplicación en App. Detener salida antes de modificar buffers o parámetros compartidos con ISR/DMA. Evitar duplicar main y handlers. Actualizar documentación y pruebas al cambiar rangos, pines, protocolo o estados. Medir el coste temporal de MATH antes de ampliar frecuencias.
