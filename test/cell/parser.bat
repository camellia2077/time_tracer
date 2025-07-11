@echo off

cd /d "%~dp0"


echo Compiling parser.cpp...
g++ parser.cpp -o classify_text -std=c++23

echo.
echo Compilation command executed.
