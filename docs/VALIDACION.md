# Validación: estado y plan de banco

**Validación física pendiente. No hay resultados de medición registrados.**

## Evidencia disponible

| Verificación | Estado al 26-10-2026 | Evidencia |
|---|---|---|
| Compilación del firmware | Confirmada por el propietario | Confirmación en conversación; log no archivado |
| Revisión estática de integración | Realizada | Includes internos, símbolos únicos y macros CMSIS revisados |
| Suite tests_host | Pendiente de ejecución confirmada | Código disponible; compiladores no arrancaron en el entorno del agente |
| Carga mediante ST-Link | Pendiente | Sin registro |
| CLI en placa | Pendiente | Sin registro |
| ADC y encoder | Pendiente | Sin registro |
| Salida LUT y MATH | Pendiente | Sin mediciones |
| Etapa analógica / calibración | Pendiente | Sin esquema validado ni medidas |

## Preparación

Registrar revisión de placa, commit exacto, versiones de PlatformIO/plataforma/toolchain, alimentación, esquema de la etapa externa e instrumentos. Revisar niveles eléctricos y referencias según HARDWARE.md. Empezar con baja frecuencia y amplitud, y medir primero PA4 antes de evaluar la etapa bipolar.

## Secuencia propuesta

- [ ] Compilar de nuevo y archivar log, versiones y commit.
- [ ] Ejecutar tests_host y guardar salida completa.
- [ ] Cargar mediante ST-Link y comprobar arranque repetible.
- [ ] Verificar consola: HELP, STATUS, SOURCE, START, STOP y restricciones IDLE/REMOTE.
- [ ] Comprobar recuperación de FAULT cuando pueda provocarse de forma controlada; no considerar RESET prueba de causa resuelta.
- [ ] Medir media escala en PA4 tras STOP; distinguirla del cero bipolar externo.
- [ ] Probar DC por offset, y seno/triángulo/cuadrada en LUT a 1 y 10 Hz.
- [ ] Registrar frecuencia, niveles mínimo/máximo, offset y Vpp reales.
- [ ] Repetir en MATH a 1 y 10 Hz, observando latencia y regularidad de muestras.
- [ ] Aumentar gradualmente hasta 100 Hz solo después de comprobar tiempos de ISR y estabilidad.
- [ ] Verificar transiciones LUT/MATH, STOP/START y cambios de configuración.
- [ ] Validar los tres potenciómetros: extremos, ruido, asentamiento y límites combinados.
- [ ] Validar sentido y pasos del encoder, incluido cambio entre las cuatro formas.
- [ ] Comprobar comportamiento de CLI ante líneas largas, comandos inválidos y recepción continua.
- [ ] Medir carga de CPU/ISR y buscar underrun del DAC; STATUS actualmente no lo detecta.
- [ ] Calibrar y caracterizar la etapa externa con su carga prevista.

## Registro por ensayo

| Campo | Valor |
|---|---|
| Fecha y operador | Pendiente |
| Commit y versiones | Pendiente |
| Placa / esquema / alimentación | Pendiente |
| Instrumento y configuración | Pendiente |
| Modo / onda / frecuencia solicitada | Pendiente |
| Amplitud / offset solicitados | Pendiente |
| Frecuencia / Vmin / Vmax / Vpp / offset medidos | Pendiente |
| Carga y punto de medida | Pendiente |
| Captura / log | Pendiente |
| Resultado e incidencia | Pendiente |

Definir las tolerancias de aceptación antes de cerrar los ensayos. No se han especificado objetivos de precisión, THD ni capacidad de carga; no convertir valores nominales en garantías. Cerrar la validación física solo con resultados trazables por commit e instrumentos.
