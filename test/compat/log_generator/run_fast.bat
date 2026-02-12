@echo off
cd /d %~dp0

call ..\..\run_log_generator.bat --build-dir build_fast %*
exit /b %ERRORLEVEL%
