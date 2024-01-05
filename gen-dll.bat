echo off
set GYP_DEFINES=component=shared_library
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set GYP_GENERATORS=msvs
set GYP_MSVS_VERSION=2022
set path=%~dp0third_party\python26;%PATH%
python build\gyp_chromium.py
