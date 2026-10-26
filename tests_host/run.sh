#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
TEMP="$(mktemp -d)"
trap 'rm -rf "$TEMP"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I"$ROOT/include" \
    "$ROOT/tests_host/test_generators.c" \
    "$ROOT/src/generator/generator.c" \
    "$ROOT/src/generator/generator_lut.c" \
    "$ROOT/src/generator/generator_math.c" \
    "$ROOT/src/generator/waveform.c" \
    "$ROOT/src/protocol/protocol.c" \
    "$ROOT/src/protocol/crc8.c" \
    -lm -o "$TEMP/test_generators"
"$TEMP/test_generators"
