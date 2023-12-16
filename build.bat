SET Path=%Path%;D:\Dev\depot_tools
SET BUILDTOOLS_PATH=D:\Dev\buildtools
set GN_PATH=%BUILDTOOLS_PATH%\win\
%GN_PATH%/gn.exe --root=./ args out
@REM %GN_PATH%/gn.exe --root=./ args out --ide=vs
ninja -j 16 -C ./out > ./out/build.log