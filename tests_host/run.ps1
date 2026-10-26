param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildDirectory = Join-Path $projectRoot '.pio/host-tests'
New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null
$sources = @(
    'tests_host/test_generators.c',
    'src/generator/generator.c',
    'src/generator/generator_lut.c',
    'src/generator/generator_math.c',
    'src/generator/waveform.c',
    'src/protocol/protocol.c',
    'src/protocol/crc8.c'
) | ForEach-Object { Join-Path $projectRoot $_ }
$executable = Join-Path $buildDirectory 'test_generators.exe'
& $Compiler -std=c11 -Wall -Wextra -Werror "-I$projectRoot/include" @sources -lm -o $executable
if ($LASTEXITCODE -ne 0) { throw 'Falló la compilación de pruebas.' }
& $executable
if ($LASTEXITCODE -ne 0) { throw 'Fallaron las pruebas.' }
