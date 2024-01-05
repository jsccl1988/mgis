
echo off
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set GYP_GENERATORS=msvs
set GYP_MSVS_VERSION=2022
set path=C:\Python27;%~dp0third_party\python26;%PATH%
echo %PATH%
python build\gyp_chromium.py
