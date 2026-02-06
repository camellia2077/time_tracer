@echo off
rem Change to the directory where the script is located
cd /d %~dp0

rem Run the Python test suite using the fast-build directory
python run.py --build-dir build_fast %*

rem Propagate the exit code from Python to the shell
exit /b %ERRORLEVEL%
