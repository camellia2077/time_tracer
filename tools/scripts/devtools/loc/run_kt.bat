@echo off
setlocal
cd /d "%~dp0"
python "run.py" --lang kt %*
set "CODE=%ERRORLEVEL%"
endlocal & exit /b %CODE%
