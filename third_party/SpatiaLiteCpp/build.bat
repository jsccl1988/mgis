@echo off

setlocal

@rem ----------------------------------------------------------------------
@rem FILE: build.bat
@rem DESCRIPTION: Script to build SpatiaLiteC++ on a Windows system.
@rem REQUIREMENTS: SpatiaLite, CMake, OpenCppCoverage, Doxygen in PATH
@rem ----------------------------------------------------------------------

@rem Default argument values
set SELF=%0
set BUILDCOV=OFF
set BUILDDIST=OFF
set BUILDDOC=OFF
set BUILDDYN=OFF
set BUILDEXS=OFF
set BUILDTEST=OFF
set BUILDMODE=Release
set BUILDDIR=build\windows
set CMAKEARGS=
set GENERATOR="Visual Studio 14 2015 Win64"

@rem Define directories
set SRCDIR=%~dp0
set DOCSDIR=%SRCDIR%\docs\api
set RESDIR=%SRCDIR%\docs\resources
set DOXYCSS=customdoxygen.css

@rem Parse arguments
set USERDIR=
if (%1)==() goto BUILD
:ARGUMENTS
    if /I %1 == -h goto USAGE
    if /I %1 == -a set BUILDDOC=ON
    if /I %1 == -b set BUILDDIST=ON
    if /I %1 == -c set CMAKEARGS=%2 %CMAKEARGS% & shift
    if /I %1 == -d set BUILDDYN=ON
    if /I %1 == -e set BUILDEXS=ON
    if /I %1 == -g set GENERATOR=%2& shift
    if /I %1 == -m set BUILDMODE=%2& shift
    if /I %1 == -o set USERDIR=%2& shift
    if /I %1 == -t set BUILDTEST=ON
    if /I %1 == -x set BUILDCOV=ON
    shift
if not (%1)==() goto ARGUMENTS
goto BUILD

@rem Display usage
:USAGE
echo Usage: %0 [-h] [-a] [-b] [-c ^<string^>] [-d] [-e] [-g ^<cmake generator^>]
echo                  [-m ^<Release^|Debug^>] [-o ^<directory^>] [-t] [-x]
echo     -h = Display this usage
echo     -a = Build API documentation
echo     -b = Build release distributions
echo     -c = Additional CMake arguments. Get concatenated and passed to CMake.
echo     -d = Build dynamic library otherwise static
echo     -e = Build examples project
echo     -g = Build generator (Default: "%GENERATOR%")
echo     -m = Build mode (Default: %BUILDMODE%)
echo     -o = Build directory (Default: %BUILDDIR%)
echo     -t = Build test project (will build examples in Debug mode)
echo     -x = Build test coverage (will build tests)
goto EOF

@rem Build library

:BUILD

@rem Build documentation

if %BUILDDOC% == ON (
    if exist %DOCSDIR% rmdir /q /s %DOCSDIR%
    mkdir %DOCSDIR%
    @rem xcopy %RESDIR% %DOCSDIR% /q /s > nul
    doxygen.exe Doxyfile
    @rem xcopy %RESDIR%\%DOXYCSS% %DOCSDIR%\%DOXYCSS% /q /s /y > nul
    goto EOF
)

if %BUILDDIST% == ON (
    call %SELF% -o "build\windows-static" -m Debug
    call %SELF% -o "build\windows-static" -m Release
    call %SELF% -o "build\windows-shared" -m Debug -d
    call %SELF% -o "build\windows-shared" -m Release -d
    goto EOF
)

if %BUILDCOV% == ON (
    set BUILDTEST=ON
)

if %BUILDTEST% == ON (
    set BUILDEXS=ON
    set BUILDMODE=Debug
)

@rem Add build mode as subfolder unless user gave explicit directory
if not "%USERDIR%" == "" (
    set BUILDDIR=%USERDIR%
)
set BUILDDIR=%BUILDDIR%\%BUILDMODE%

mkdir %BUILDDIR%
pushd %BUILDDIR%
cmake -DSPATIALITECPP_BUILD_DYNAMIC=%BUILDDYN% ^
      -DSPATIALITECPP_BUILD_EXAMPLES=%BUILDEXS% ^
      -DSPATIALITECPP_BUILD_TEST=%BUILDTEST% ^
      -DBUILD_TYPE=%BUILDMODE% ^
      -G%GENERATOR% ^
      %CMAKEARGS% ^
      %SRCDIR% || goto STOP

set TARGET=PACKAGE
if %BUILDTEST% == ON (
    set TARGET=INSTALL
)
cmake --build . ^
      --config %BUILDMODE% ^
      --target %TARGET% || goto STOP

if %BUILDCOV% == ON (
    if exist coverage rmdir /q /s coverage
    OpenCppCoverage.exe --export_type html:coverage ^
        --modules "SpatiaLiteCpp" ^
        --sources "SpatiaLiteCpp" ^
        --excluded_sources "build\\" ^
        --excluded_sources "ext\\" ^
        --excluded_sources "test\\" ^
        -- .\test\SpatiaLiteCpp\%BUILDMODE%\SpatiaLiteCppTest.exe || goto STOP
)

:STOP

popd

endlocal

:EOF

@echo on
