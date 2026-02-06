@echo off
rem Change to the directory where the script is located
cd /d %~dp0

rem Run the Python test suite
python run.py

rem Propagate the exit code from Python to the shell
exit /b %ERRORLEVEL%
