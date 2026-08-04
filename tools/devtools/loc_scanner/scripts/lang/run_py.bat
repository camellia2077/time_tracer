@echo off
setlocal
python "%~dp0..\src\loc_scanner\run.py" --workspace-root "%~dp0..\..\..\.." --config "tools/devtools/loc_scanner/config/scan_lines.toml" --lang py %*
set "CODE=%ERRORLEVEL%"
endlocal & exit /b %CODE%
