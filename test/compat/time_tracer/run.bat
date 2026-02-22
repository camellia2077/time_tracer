@echo off
cd /d %~dp0

call ..\..\run_time_tracer_cli.bat --skip-configure --skip-build %*

exit /b %ERRORLEVEL%
