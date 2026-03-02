@echo off
setlocal
cd /d "%~dp0"
python "run.py" --lang cpp %*
set "CODE=%ERRORLEVEL%"
endlocal & exit /b %CODE%
