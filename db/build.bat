@echo off
REM This batch script uses modern CMake syntax to configure and build the project.
REM It is designed to be run from any directory by first changing to its own location.

REM Change the current directory to the directory where this script resides.
REM pushd saves the current location so we can return to it later.
pushd "%~dp0"

echo Configuring the project...
REM Now that we are in the correct directory, we can use simple relative paths.
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release

echo Building the project...
cmake --build build

REM Return to the original directory from where the script was called.
popd

echo.
echo Build finished. The executable is in the 'build' folder.
pause
