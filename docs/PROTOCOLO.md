# Optional binary protocol

Protocol_Encode and Protocol_Decode implement a standalone codec. **There is no binary UART receiver, stream parser, command map, or connection to App.** The implemented control interface is the text CLI.

## Frame layout

| Position | Field |
|---|---|
| 0 | SOF = 0xAA |
| 1 | type, one byte |
| 2 | command, one byte |
| 3 | length, 0–32 bytes |
| 4… | payload |
| 4 + length | XOR of type, command, length, and payload |

Total size is `length + 5`, at most 37 bytes. SOF is excluded from the checksum. No byte escaping is used: payload may contain 0xAA. Decode expects one complete frame of the exact specified size.

Example with type=1, command=4, and payload=[2,3,5]:

```text
AA 01 04 03 02 03 05 02
```

The final 02 is `01 XOR 04 XOR 03 XOR 02 XOR 03 XOR 05`.

The CRC8 name is inherited: the implemented algorithm is XOR, **not a polynomial CRC**, and does not provide authentication. type and command are opaque values to the codec; their application semantics remain undefined.

Encode checks pointers, payload length, and output capacity. Decode checks SOF, exact size, maximum length, destination capacity, and checksum. Do not use returned output lengths if the function returns false. Input and output buffers must be independent to avoid overlapping memcpy operations.

The API is declared in include/protocol/protocol.h and the checksum in include/protocol/crc8.h. Host tests cover roundtrip encoding, altered checksum, empty frames, maximum payload, truncation, and insufficient capacity.
