# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
# Editor startup needs libfbxsdk.dll on the loader search path. The engine
# keeps it under Binaries\ThirdParty\FBX, which is not searched during
# LdrpInitializeProcess unless this directory is on PATH.

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
Start-Process -FilePath $editor -ArgumentList @($project) -WorkingDirectory (Split-Path $editor)
