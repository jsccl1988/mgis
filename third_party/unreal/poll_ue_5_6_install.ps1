# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
# Poll until UE 5.6 is installed, then set user env UE_5_6_ROOT.

$ErrorActionPreference = "Continue"
$deadline = (Get-Date).AddHours(6)
$logOut = Join-Path $PSScriptRoot "..\.build\ue56-install-poll.log"
$launcherLog = Join-Path $env:LOCALAPPDATA "EpicGamesLauncher\Saved\Logs\EpicGamesLauncher.log"

function Test-Ue56Complete([string]$root) {
  # Launcher binary installs need Engine\Source for Build.bat / C++ game modules.
  $editor = Join-Path $root "Engine\Binaries\Win64\UnrealEditor.exe"
  $source = Join-Path $root "Engine\Source"
  $plugins = Join-Path $root "Engine\Plugins"
  return ((Test-Path $editor) -and (Test-Path $source) -and (Test-Path $plugins))
}

function Find-Ue56Root {
  $candidates = @(
    "C:\Program Files\Epic Games\UE_5.6",
    "C:\Program Files\Epic Games\UE_5.6.1",
    "C:\Program Files (x86)\Epic Games\UE_5.6",
    "D:\Epic Games\UE_5.6",
    "E:\Epic Games\UE_5.6",
    "C:\UE_5.6"
  )
  foreach ($c in $candidates) {
    if (Test-Ue56Complete $c) {
      return $c
    }
  }
  foreach ($base in @("C:\Program Files\Epic Games", "C:\Program Files (x86)\Epic Games", "D:\Epic Games", "E:\Epic Games")) {
    if (-not (Test-Path $base)) { continue }
    $hit = Get-ChildItem $base -Directory -ErrorAction SilentlyContinue |
      Where-Object { $_.Name -like "UE_5.6*" -and (Test-Ue56Complete $_.FullName) } |
      Select-Object -First 1
    if ($hit) { return $hit.FullName }
  }
  return $null
}

function Get-ProgressNote {
  $parts = @()
  foreach ($base in @("C:\Program Files\Epic Games", "C:\Program Files (x86)\Epic Games")) {
    if (-not (Test-Path $base)) { continue }
    Get-ChildItem $base -Directory -ErrorAction SilentlyContinue |
      Where-Object { $_.Name -like "UE_5.6*" } |
      ForEach-Object {
        $bytes = 0L
        Get-ChildItem $_.FullName -Recurse -File -ErrorAction SilentlyContinue |
          ForEach-Object { $bytes += $_.Length }
        $gb = [math]::Round($bytes / 1GB, 2)
        $ed = Test-Path (Join-Path $_.FullName "Engine\Binaries\Win64\UnrealEditor.exe")
        $src = Test-Path (Join-Path $_.FullName "Engine\Source")
        $plg = Test-Path (Join-Path $_.FullName "Engine\Plugins")
        $parts += ("{0}~{1}GB editor={2} source={3} plugins={4}" -f $_.Name, $gb, $ed, $src, $plg)
      }
  }
  $launcherUp = [bool](Get-Process EpicGamesLauncher -ErrorAction SilentlyContinue)
  return ("[{0}] launcher={1}" -f (($parts -join "; "), $launcherUp))
}

"poll start $(Get-Date -Format o)" | Tee-Object -FilePath $logOut
while ((Get-Date) -lt $deadline) {
  $root = Find-Ue56Root
  if ($root) {
    [Environment]::SetEnvironmentVariable("UE_5_6_ROOT", $root, "User")
    $env:UE_5_6_ROOT = $root
    "FOUND $root $(Get-Date -Format o)" | Tee-Object -FilePath $logOut -Append
    Write-Host "UE_5_6_ROOT=$root"
    exit 0
  }
  $note = Get-ProgressNote
  "$(Get-Date -Format HH:mm:ss) waiting... $note" | Tee-Object -FilePath $logOut -Append
  Start-Sleep -Seconds 60
}

"TIMEOUT $(Get-Date -Format o)" | Tee-Object -FilePath $logOut -Append
exit 2
