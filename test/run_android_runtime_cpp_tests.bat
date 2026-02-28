@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%.."
set "BUILD_DIR=build_fast"

cd /d "%REPO_ROOT%"
if errorlevel 1 (
  echo Error: failed to enter repo root: %REPO_ROOT%>&2
  exit /b 1
)

echo [1/1] Verify time_tracer (build + tests, %BUILD_DIR%)
python scripts/run.py verify --app tracer_core --build-dir %BUILD_DIR% --concise
exit /b %ERRORLEVEL%
