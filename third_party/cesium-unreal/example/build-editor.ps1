# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
# Builds MViewGlobeEditor (Development Win64).
# Usage: .\build-editor.ps1 [-MaxParallelActions 1] [-NoUBA]

param(
    [int]$MaxParallelActions = 0,
    [switch]$NoUBA
)

$ErrorActionPreference = "Stop"
$engine = $env:UE_5_6_ROOT
if ([string]::IsNullOrWhiteSpace($engine)) {
    $engine = "C:\Program Files\Epic Games\UE_5.6"
}
$buildBat = Join-Path $engine "Engine\Build\BatchFiles\Build.bat"
$project = Join-Path $PSScriptRoot "MViewGlobe.uproject"
if (-not (Test-Path -LiteralPath $buildBat)) {
    throw "Build.bat not found: $buildBat (set UE_5_6_ROOT)"
}
if (-not (Test-Path -LiteralPath $project)) {
    throw "Project not found: $project"
}
$args = @(
    "MViewGlobeEditor",
    "Win64",
    "Development",
    "-Project=$project",
    "-WaitMutex"
)
if ($MaxParallelActions -gt 0) {
    $args += "-MaxParallelActions=$MaxParallelActions"
}
if ($NoUBA) {
    $args += "-NoUBA"
}
& $buildBat @args
exit $LASTEXITCODE
