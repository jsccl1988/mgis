@REM Copyright (c) 2014 S�bastien Rombauts (sebastien.rombauts@gmail.com)
@REM 
@REM Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
@REM or copy at http://opensource.org/licenses/MIT)

mkdir build
cd build
@REM generate solution for Visual Studio, and build it
cmake ..
cmake --build .

@REM launch tests
ctest --output-on-failure
cd ..
