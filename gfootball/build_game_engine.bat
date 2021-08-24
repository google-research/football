@echo off
rem This batch file compiles gfootball engine on Windows

if "%VCPKG_ROOT%"=="" (
    echo VCPKG_ROOT environment variable is missing
    echo Execute: SET "VCPKG_ROOT=PATH_TO_VCPKG" e.g., SET "VCPKG_ROOT=C:\src\vcpkg\"
    exit /b 1
)
if "%GENERATOR_PLATFORM%"=="" (
    echo GENERATOR_PLATFORM environment variable is missing
    echo Execute: SET GENERATOR_PLATFORM={x64 or Win32}, depending on the Python Interpreter
    exit /b 1
)
if "%BUILD_CONFIGURATION%"=="" set BUILD_CONFIGURATION=Release

pushd third_party\gfootball_engine

if not exist build_win mkdir build_win
if exist build_win\CMakeCache.txt del build_win\CMakeCache.txt
if exist build_win\%BUILD_CONFIGURATION% rmdir /s /q build_win\%BUILD_CONFIGURATION%

pushd build_win
cmake .. -A%GENERATOR_PLATFORM%
if errorlevel 1 exit /B 1

cmake --build . --parallel --config %BUILD_CONFIGURATION%
if errorlevel 1 exit /B 1

popd && popd