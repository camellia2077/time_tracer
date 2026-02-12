@echo off
cd /d %~dp0

call ..\..\run_time_tracer.bat --skip-configure --skip-build --agent --build-dir build_agent %*
exit /b %ERRORLEVEL%
