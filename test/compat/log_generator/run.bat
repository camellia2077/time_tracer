@echo off
cd /d %~dp0

call ..\..\run_log_generator.bat %*
exit /b %ERRORLEVEL%
