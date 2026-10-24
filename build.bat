@echo off
REM Copyright (c) 2026 The Mogu Authors.
REM All rights reserved.
REM Engineering entry matches smartgis: gn gen + ninja under out\Debug and out\Release.
REM No compile lock. Sources are BUILD.gn. Do not add a GYP entry.
setlocal EnableExtensions EnableDelayedExpansion

pushd "%~dp0" || (
  echo ERROR: failed to enter "%~dp0"
  exit /b 1
)

if exist "%LocalAppData%\Programs\Python\Python312\python.exe" (
  set "PATH=%LocalAppData%\Programs\Python\Python312;%PATH%"
) else if exist "%LocalAppData%\Programs\Python\Launcher\py.exe" (
  set "PATH=%LocalAppData%\Programs\Python\Launcher;%PATH%"
)

set "GN=%~dp0build\bin\gn.exe"
set "NINJA=%~dp0build\bin\ninja.exe"
if not exist "%GN%" (
  echo ERROR: gn.exe not found at "%GN%"
  popd
  exit /b 1
)
if not exist "%NINJA%" (
  where ninja >nul 2>&1
  if errorlevel 1 (
    echo ERROR: ninja not found at "%NINJA%" or on PATH.
    popd
    exit /b 1
  )
  set "NINJA=ninja"
)

if /I "%~1"=="sln" (
  echo ERROR: MSBuild/sln is not an engineering entry. Use build.bat ^(GN^).>&2
  popd
  exit /b 2
)

REM third_party fetch. Not a ninja target.
if /I "%~1"=="t" (
  py -3 "%~dp0third_party\tools\fetch.py" --all
  set "RC=!ERRORLEVEL!"
  popd
  exit /b !RC!
)

if not exist ".\out" mkdir ".\out"
if not exist ".\out\Debug" mkdir ".\out\Debug"
if not exist ".\out\Release" mkdir ".\out\Release"

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build\config\win\write_msvc_env.ps1" -OutFile "%~dp0out\environment.x64.x64" -Arch x64
if errorlevel 1 (
  echo ERROR: failed to write out\environment.x64.x64
  popd
  exit /b 1
)
copy /Y "%~dp0out\environment.x64.x64" "%~dp0out\Debug\environment.x64.x64" >nul
copy /Y "%~dp0out\environment.x64.x64" "%~dp0out\Release\environment.x64.x64" >nul

REM Optional first arg: debug|release, then m / all / a raw ninja target.
REM Default and m build product demos (ui/views/examples + app/ui_designer). Both configs unless filtered.
set "BUILD_DEBUG=1"
set "BUILD_RELEASE=1"
set "TARGET_ARG=%~1"
if /I "%~1"=="debug" (
  set "BUILD_RELEASE=0"
  set "TARGET_ARG=%~2"
) else if /I "%~1"=="release" (
  set "BUILD_DEBUG=0"
  set "TARGET_ARG=%~2"
)

set "NINJA_TARGET=default"
if /I "!TARGET_ARG!"=="m" (
  set "NINJA_TARGET=default"
) else if /I "!TARGET_ARG!"=="all" (
  set "NINJA_TARGET=all"
) else if not "!TARGET_ARG!"=="" (
  set "NINJA_TARGET=!TARGET_ARG!"
)

set "ERR=0"
set "LAST_LOG="

if "!BUILD_DEBUG!"=="1" (
  call :build_config Debug true
  set "CFG_RC=!ERRORLEVEL!"
  if !CFG_RC! NEQ 0 (
    set "ERR=!CFG_RC!"
    goto :finish
  )
)

if "!BUILD_RELEASE!"=="1" (
  call :build_config Release false
  set "CFG_RC=!ERRORLEVEL!"
  if !CFG_RC! NEQ 0 (
    set "ERR=!CFG_RC!"
    goto :finish
  )
)

:finish
echo Exit code: !ERR!
if defined LAST_LOG (
  echo Log: !LAST_LOG!
) else (
  echo Log: %~dp0out\Debug\build.log / %~dp0out\Release\build.log
)
popd
exit /b !ERR!

REM ---------------------------------------------------------------------------
REM :build_config <OutDirName> <is_debug true|false>
REM ---------------------------------------------------------------------------
:build_config
set "OUT_NAME=%~1"
set "IS_DEBUG=%~2"
set "OUT_DIR=out\%OUT_NAME%"
REM Debug defaults to component build (DLLs) for faster incremental link.
REM Release stays static (is_component_build omitted → false).
if /I "%IS_DEBUG%"=="true" (
  set "GN_ARGS=is_debug=%IS_DEBUG% is_component_build=true is_build_third_party=false run_vs_env_script=false vs_version=180 msvc_installed=true"
) else (
  set "GN_ARGS=is_debug=%IS_DEBUG% is_component_build=false is_build_third_party=false run_vs_env_script=false vs_version=180 msvc_installed=true"
)

echo === gn gen %OUT_DIR% ^(is_debug=%IS_DEBUG%^) ===
"%GN%" gen "%OUT_DIR%" --root=./ --ide=vs2019 --args="!GN_ARGS!"
if errorlevel 1 exit /b 1

echo === ninja -C %OUT_DIR% !NINJA_TARGET! ===
"%NINJA%" -j 16 -C "./%OUT_DIR%" !NINJA_TARGET! > "./%OUT_DIR%/build.log" 2>&1
set "CFG_ERR=!ERRORLEVEL!"
set "LAST_LOG=%~dp0%OUT_DIR%\build.log"
if !CFG_ERR! NEQ 0 (
  echo ERROR: ninja failed for %OUT_DIR% ^(exit !CFG_ERR!^). See !LAST_LOG!
  exit /b !CFG_ERR!
)
exit /b 0
