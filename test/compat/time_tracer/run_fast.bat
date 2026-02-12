@echo off
cd /d %~dp0

call ..\..\run_time_tracer.bat --skip-configure --skip-build --build-dir build_fast %*
exit /b %ERRORLEVEL%
