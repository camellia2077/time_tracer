@echo off
setlocal
cd /d "%~dp0"
python "run.py" --lang py --dir-over-files %*
set "CODE=%ERRORLEVEL%"
endlocal & exit /b %CODE%
