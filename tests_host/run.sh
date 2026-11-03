#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I"$ROOT/include" \
    "$ROOT/tests_host/test_generators.c" \
    "$ROOT/src/generator/generator.c" \
    "$ROOT/src/generator/generator_lut.c" \
    "$ROOT/src/generator/generator_math.c" \
    "$ROOT/src/generator/sample_plan.c" \
    "$ROOT/src/app/parameter_manager.c" \
    "$ROOT/src/app/app.c" \
    "$ROOT/src/app/fsm.c" \
    "$ROOT/src/app/scheduler.c" \
    "$ROOT/src/protocol/cli.c" \
    "$ROOT/src/utils/filter.c" \
    "$ROOT/src/utils/map.c" \
    "$ROOT/src/generator/waveform.c" \
    "$ROOT/src/protocol/protocol.c" \
    "$ROOT/src/protocol/crc8.c" \
    -lm -o "$TMP/test_generators"
"$TMP/test_generators"
