@echo off
:: Batch script to clean up the old output directory, then run the Python script.

:: Switch to the directory where this batch file is located.
cd /d %~dp0

echo Checking for previous output directory...

:: Check if the 'output' directory exists.
IF EXIST "output" (
    echo Found "output" directory, deleting it...
    :: Quietly (/q) and recursively (/s) remove the directory.
    rmdir /s /q "output"
)

echo.
echo -----------------------------------
echo Executing Python test script...
echo -----------------------------------
echo.

:: Execute the Python script.
py test_exe_main.py

echo.
echo -----------------------------------
echo Script execution finished.
echo -----------------------------------
