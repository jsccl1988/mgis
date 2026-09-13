@echo off
REM Copyright (c) 2026 The Mogu Authors.
REM All rights reserved.
setlocal EnableExtensions

REM CMD cannot use a UNC path as the working directory; pushd maps it to a drive letter.
pushd "%~dp0" || (
  echo ERROR: failed to enter "%~dp0"
  exit /b 1
)

if exist "D:\Dev\depot_tools" set "PATH=%PATH%;D:\Dev\depot_tools"

if not defined BUILDTOOLS_PATH (
  if exist "%~dp0build\bin\gn.exe" (
    set "GN_PATH=%~dp0build\bin\"
  ) else if exist "D:\Dev\buildtools\win\gn.exe" (
    set "BUILDTOOLS_PATH=D:\Dev\buildtools"
    set "GN_PATH=%BUILDTOOLS_PATH%\win\"
  ) else (
    echo gn.exe not found; fetching into build\bin ...
    python "%~dp0build\fetch_binaries.py"
    if errorlevel 1 (
      echo ERROR: failed to fetch build binaries. Install Python, or set BUILDTOOLS_PATH.
      popd
      exit /b 1
    )
    set "GN_PATH=%~dp0build\bin\"
  )
) else (
  set "GN_PATH=%BUILDTOOLS_PATH%\win\"
)

if exist "%~dp0build\bin" set "PATH=%PATH%;%~dp0build\bin"

REM VS ships CMake; use it when cmake.exe is not already on PATH.
where cmake >nul 2>&1
if errorlevel 1 (
  if exist "%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set "PATH=%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
  ) else if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
    set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
  )
)

if not exist "%GN_PATH%gn.exe" (
  echo ERROR: gn.exe not found at "%GN_PATH%gn.exe"
  popd
  exit /b 1
)

"%GN_PATH%gn.exe" gen out --root=./ --ide=vs2019 --args="is_debug=true is_build_third_party=false"
if errorlevel 1 (
  popd
  exit /b 1
)

where ninja >nul 2>&1
if errorlevel 1 (
  echo ERROR: ninja not found on PATH. Put it in build\bin or D:\Dev\depot_tools.
  popd
  exit /b 1
)

if not exist ".\out" mkdir ".\out"

REM mogu `build.sh build t`: fetch is optional (vendored trees); install to .install
if /I "%~1"=="t" (
  set "PY=py -3"
  py -3 -c "import sys" >nul 2>&1
  if errorlevel 1 set "PY=python"
  if /I "%~2"=="" (
    %PY% "%~dp0third_party\tools\batch.py" --manifest "%~dp0third_party\manifest.json" --install-prefix "%~dp0third_party\.install" --build-type Debug
  ) else (
    %PY% "%~dp0third_party\tools\batch.py" --manifest "%~dp0third_party\manifest.json" --install-prefix "%~dp0third_party\.install" --build-type Debug --package "%~2"
  )
  set "ERR=%ERRORLEVEL%"
  if %ERR%==0 (
    if not exist "%~dp0out" mkdir "%~dp0out"
    if not exist "%~dp0out\third_party" (
      mklink /J "%~dp0out\third_party" "%~dp0third_party\.install"
    )
  )
  popd
  exit /b %ERR%
)

REM Optional first arg: mogu-style aliases (m/te/a/b) or a raw ninja target.
set "NINJA_TARGET="
if not "%~1"=="" (
  if /I "%~1"=="m" (
    set "NINJA_TARGET=all"
  ) else if /I "%~1"=="te" (
    set "NINJA_TARGET=test_all"
  ) else if /I "%~1"=="a" (
    set "NINJA_TARGET=all_with_tests"
  ) else if /I "%~1"=="b" (
    set "NINJA_TARGET=benchmark_all"
  ) else (
    set "NINJA_TARGET=%~1"
  )
)

if defined NINJA_TARGET (
  ninja -j 16 -C ./out %NINJA_TARGET% > ./out/build.log
) else (
  ninja -j 16 -C ./out > ./out/build.log
)
set "ERR=%ERRORLEVEL%"
popd
exit /b %ERR%
