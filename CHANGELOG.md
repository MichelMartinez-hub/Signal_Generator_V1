# Historial de cambios

## 2026-10-26 — Integración inicial y documentación

### Integrado

- Codec binario opcional y checksum XOR antes vacíos.
- Mejoras de DAC, TIM6, coordinación WaveOutput, reloj y temporizador del scheduler.
- Tests de host para LUT/MATH y protocolo; ejecutor PowerShell y casos de límites.
- Guías de hardware, arquitectura, CLI, protocolo, desarrollo y plan de validación física.

### Corregido

- Uso de NULL sin incluir stddef.h en generator.c.
- Límites inconsistentes de frecuencia: 1–100 Hz en CLI, controles, API y backends.
- Frecuencia inicial reducida a 10 Hz.
- Reinicio de contador y redondeo de TIM6; admisión del intervalo de 65536 ticks.
- Inicialización y limpieza de underrun del DAC.
- Consistencia entre reloj real configurado, divisores y SystemCoreClock.

### Estado

- Compilación confirmada por el propietario.
- Pruebas de host: ejecución satisfactoria todavía sin confirmar.
- Validación física: pendiente.

El detalle de la integración está en docs/INTEGRACION.md. No se ha publicado una versión de firmware validada físicamente.
