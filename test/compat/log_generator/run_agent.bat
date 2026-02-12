@echo off
cd /d %~dp0

call ..\..\run_log_generator.bat --agent --concise %*
exit /b %ERRORLEVEL%
