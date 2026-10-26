# Referencia de consola serie

115200 baudios, 8N1 y sin control de flujo. Finalizar cada comando con CR o LF. Comandos y tokens se convierten a mayúsculas; se admite notación exponencial en números. Usar punto decimal. La línea admite hasta 95 caracteres; una línea demasiado larga o con desbordamiento RX se descarta.

| Comando | Efecto / condición |
|---|---|
| HELP | Muestra ayuda |
| STATUS | Estado, origen, fallo y parámetros solicitados |
| START | Requiere IDLE; en REMOTE arranca; en LOCAL espera controles estables |
| STOP | Detiene salida; RUNNING → IDLE; no borra FAULT |
| RESET | Borra un FAULT y vuelve a IDLE |
| SOURCE LOCAL | Selecciona potenciómetros y encoder; requiere IDLE |
| SOURCE REMOTE | Selecciona control por consola; requiere IDLE |
| MODE LUT / MODE MATH | Selecciona backend; requiere REMOTE e IDLE |
| WAVE DC / SINE / TRIANGLE / SQUARE | Selecciona forma; requiere REMOTE e IDLE |
| FREQ valor | 1–100 Hz; requiere REMOTE e IDLE |
| AMP valor | 0–5 V de pico; requiere REMOTE e IDLE y margen de offset |
| OFFSET valor | −5 a +5 V; requiere REMOTE e IDLE y margen de amplitud |

Las tensiones representan la escala bipolar ideal de la etapa externa. Se exige `abs(offset) + amplitud <= 5`. AMP expresa pico, no pico a pico; para seno/triángulo/cuadrada, Vpp ideal = 2 × AMP. DC usa el offset; la validación de margen sigue aplicándose.

## Sesión remota

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

Probar físicamente MATH primero a baja frecuencia; el máximo configurado no confirma capacidad temporal.

## Cambiar offset sin un rechazo intermedio

Si la amplitud actual es 5 V, `OFFSET 3` se rechaza por falta de margen. Utilizar:

```text
STOP
SOURCE REMOTE
AMP 0
OFFSET 3
AMP 1
START
```

## Control local

```text
STOP
SOURCE LOCAL
START
```

El firmware vuelve a aprender los potenciómetros. Necesita tres observaciones cuantizadas estables, tomadas cada 50 ms; el filtrado puede aumentar el tiempo de asentamiento. El encoder selecciona DC → SINE → TRIANGLE → SQUARE cíclicamente. LOCAL arranca seleccionando SINE.

## Respuestas y diagnóstico

Los comandos de cambio aceptados responden `OK`; los rechazos responden `ERR: ...`. HELP y STATUS imprimen información. Los fallos de aplicación reportados son NONE, CONTROLS_READ, PARAMETER_UPDATE y GENERATOR_START.

Si aparece FAULT: ejecutar STATUS, corregir la causa, RESET y después configurar/START. STOP no borra fallos. STATUS muestra configuración y estado lógico; no mide tensión/frecuencia, ni informa actualmente un underrun del DAC.
