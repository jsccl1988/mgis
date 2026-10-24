# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
# Launches the editor in game mode for one demo.
# Usage: .\run-demo.ps1 globe|earth|groundwater|wind|geology|flood
# Google Earth-style globe (atmosphere + space FlyTo + Photorealistic when
# CESIUM_ION_TOKEN is set): .\run-demo.ps1 earth

param(
    [Parameter(Position = 0)]
    [ValidateSet("globe", "earth", "google", "photorealistic", "groundwater", "wind", "geology", "flood")]
    [string]$Demo = "earth"
)

$ErrorActionPreference = "Stop"
$engine = $env:UE_5_6_ROOT
if ([string]::IsNullOrWhiteSpace($engine)) {
    $engine = "C:\Program Files\Epic Games\UE_5.6"
}
$fbx = Join-Path $engine "Engine\Binaries\ThirdParty\FBX\2020.2\Win64"
$editor = Join-Path $engine "Engine\Binaries\Win64\UnrealEditor.exe"
$project = Join-Path $PSScriptRoot "MViewGlobe.uproject"
if (-not (Test-Path -LiteralPath (Join-Path $fbx "libfbxsdk.dll"))) {
    throw "libfbxsdk.dll not found: $fbx"
}
if (-not (Test-Path -LiteralPath $editor)) {
    throw "UnrealEditor.exe not found: $editor"
}
$env:PATH = "$fbx;$env:PATH"
# Force MViewGlobeGameMode on the engine Entry map (DefaultEngine.ini also sets it).
$map = "/Engine/Maps/Entry?game=/Script/MViewGlobe.MViewGlobeGameMode"
$args = @(
    $project,
    $map,
    "-game",
    "-windowed",
    "-resx=1280",
    "-resy=720",
    "-MViewDemo=$Demo",
    "-log"
)
Write-Host "Starting MViewGlobe demo=$Demo"
if ([string]::IsNullOrWhiteSpace($env:CESIUM_ION_TOKEN) -and
    ($Demo -eq "globe" -or $Demo -eq "earth" -or $Demo -eq "google" -or $Demo -eq "photorealistic")) {
    Write-Host "Note: CESIUM_ION_TOKEN unset — ellipsoid + MapLibre demotiles (MVT) + SunSky FlyTo."
    Write-Host "      Override: MVIEW_VECTOR_TILES_URL=<TileJSON> or MVIEW_IMAGERY_URL=<XYZ png template>."
}
Start-Process -FilePath $editor -ArgumentList $args -WorkingDirectory (Split-Path $editor)
