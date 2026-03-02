@echo off
setlocal
cd /d "%~dp0"
python "run.py" --lang rs %*
set "CODE=%ERRORLEVEL%"
endlocal & exit /b %CODE%
