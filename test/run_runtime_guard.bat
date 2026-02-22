@echo off
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

python "%SCRIPT_DIR%runtime_guard.py" %*

exit /b %ERRORLEVEL%
