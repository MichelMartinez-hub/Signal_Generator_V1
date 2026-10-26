# Protocolo binario opcional

Protocol_Encode y Protocol_Decode son un codec independiente. **No hay receptor binario en la UART, parser de stream, mapa de comandos ni enlace con App.** El canal de control implementado es la CLI de texto.

## Trama

| Posición | Campo |
|---|---|
| 0 | SOF = 0xAA |
| 1 | type, 1 byte |
| 2 | command, 1 byte |
| 3 | length, 0–32 bytes |
| 4… | payload |
| 4 + length | XOR de type, command, length y payload |

Tamaño total: `length + 5`, máximo 37 bytes. SOF no participa en el checksum. No hay escape de bytes: payload puede contener 0xAA. Decode espera una trama completa y de tamaño exacto.

Ejemplo para type=1, command=4 y payload=[2,3,5]:

```text
AA 01 04 03 02 03 05 02
```

El último 02 resulta de `01 XOR 04 XOR 03 XOR 02 XOR 03 XOR 05`.

El nombre CRC8 es heredado: el algoritmo implementado es XOR, **no un CRC polinómico** y no aporta autenticación. type y command son valores opacos para el codec; su semántica queda por definir.

Encode comprueba punteros, tamaño del payload y capacidad de salida. Decode comprueba SOF, tamaño exacto, longitud máxima, capacidad destino y checksum. No utilizar las longitudes de salida si la función devuelve false. Los buffers de entrada y salida deben ser independientes para evitar solapamientos con memcpy.

La API está en include/protocol/protocol.h y el checksum en include/protocol/crc8.h. Las pruebas de host incluyen roundtrip, checksum alterado, trama vacía, tamaño máximo, truncamiento y capacidad insuficiente.
