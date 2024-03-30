SET Path=%Path%;D:\Dev\depot_tools
SET BUILDTOOLS_PATH=D:\Dev\buildtools
SET GN_PATH=%BUILDTOOLS_PATH%\win\
%GN_PATH%/gn.exe gen out --root=./ --ide=vs2022 --args="is_debug=true"
ninja -j 16 -C ./out > ./out/build.log