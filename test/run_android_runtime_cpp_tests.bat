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

echo [1/3] Configure time_tracer (%BUILD_DIR%)
python scripts/run.py configure --app time_tracer --build-dir %BUILD_DIR%
if errorlevel 1 exit /b %ERRORLEVEL%

echo [2/3] Build time_tracer (%BUILD_DIR%)
python scripts/run.py build --app time_tracer --build-dir %BUILD_DIR%
if errorlevel 1 exit /b %ERRORLEVEL%

echo [3/3] Run time_tracer tests (agent mode)
python test/run.py --suite time_tracer --agent --build-dir %BUILD_DIR% --concise
exit /b %ERRORLEVEL%
