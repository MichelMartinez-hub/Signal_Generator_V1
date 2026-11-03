param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root '.pio/host-tests'
New-Item -ItemType Directory -Force -Path $build | Out-Null
$sources = @(
    'tests_host/test_generators.c',
    'src/generator/generator.c',
    'src/generator/generator_lut.c',
    'src/generator/generator_math.c',
    'src/generator/sample_plan.c',
    'src/app/parameter_manager.c',
    'src/app/app.c',
    'src/app/fsm.c',
    'src/app/scheduler.c',
    'src/protocol/cli.c',
    'src/utils/filter.c',
    'src/utils/map.c',
    'src/generator/waveform.c',
    'src/protocol/protocol.c',
    'src/protocol/crc8.c'
) | ForEach-Object { Join-Path $root $_ }
$binary = Join-Path $build 'test_generators.exe'
& $Compiler -std=c11 -Wall -Wextra -Werror "-I$root/include" @sources -lm -o $binary
if ($LASTEXITCODE -ne 0) { throw 'Host compile failed' }
& $binary
if ($LASTEXITCODE -ne 0) { throw 'Host tests failed' }
