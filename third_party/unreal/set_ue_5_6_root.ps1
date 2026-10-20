# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
# Detect UE 5.6 install root and set user env UE_5_6_ROOT (persistent).
$ErrorActionPreference = "Stop"
$candidates = @(
  "C:\Program Files\Epic Games\UE_5.6",
  "D:\Epic Games\UE_5.6",
  "E:\Epic Games\UE_5.6",
  "C:\UE_5.6"
)
$root = $null
foreach ($c in $candidates) {
  if (Test-Path (Join-Path $c "Engine\Binaries\Win64\UnrealEditor.exe")) {
    $root = $c
    break
  }
}
if (-not $root) {
  $found = Get-ChildItem "C:\Program Files\Epic Games","D:\Epic Games","E:\Epic Games" -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '^UE_5\.6' -and (Test-Path (Join-Path $_.FullName "Engine\Binaries\Win64\UnrealEditor.exe")) } |
    Select-Object -First 1
  if ($found) { $root = $found.FullName }
}
if (-not $root) {
  Write-Error "UE 5.6 not found. Install via Epic Games Launcher (Unreal Engine tab), then re-run."
}
[Environment]::SetEnvironmentVariable("UE_5_6_ROOT", $root, "User")
$env:UE_5_6_ROOT = $root
Write-Host "UE_5_6_ROOT=$root"
Test-Path (Join-Path $root "Engine\Binaries\Win64\UnrealEditor.exe")
