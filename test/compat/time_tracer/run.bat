@echo off
cd /d %~dp0

call ..\..\run_time_tracer.bat --skip-configure --skip-build %*

exit /b %ERRORLEVEL%
