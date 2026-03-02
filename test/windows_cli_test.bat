@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"

set "BUILD_MODE="
set "EXTRA_ARGS="

:parse_args
if "%~1"=="" goto args_done

if /I "%~1"=="-d" (
    if "%~2"=="" (
        echo [ERROR] Missing value after -d. Use build or fast.
        exit /b 2
    )
    set "BUILD_MODE=%~2"
    shift /1
    shift /1
    goto parse_args
)

set "EXTRA_ARGS=!EXTRA_ARGS! "%~1""
shift /1
goto parse_args

:args_done
if "%BUILD_MODE%"=="" (
    echo Usage:
    echo   windows_cli_test.bat -d ^<build^|fast^> [suite_extra_args...]
    echo.
    echo Examples:
    echo   windows_cli_test.bat -d build
    echo   windows_cli_test.bat -d fast --concise
    exit /b 2
)

if /I "%BUILD_MODE%"=="build" (
    set "RUST_BIN_REL=apps/tracer_cli/windows/rust_cli/build/bin"
) else if /I "%BUILD_MODE%"=="fast" (
    set "RUST_BIN_REL=apps/tracer_cli/windows/rust_cli/build_fast/bin"
) else (
    echo [ERROR] Unsupported -d value: "%BUILD_MODE%"
    echo [HINT]  Use: -d build  or  -d fast
    exit /b 2
)

pushd "%REPO_ROOT%" >nul

where python >nul 2>nul
if %errorlevel%==0 (
    set "PY_CMD=python"
) else (
    where py >nul 2>nul
    if %errorlevel%==0 (
        set "PY_CMD=py -3"
    ) else (
        echo [ERROR] Python runtime not found. Please install python or py launcher.
        popd >nul
        exit /b 1
    )
)

for %%I in ("%RUST_BIN_REL%") do set "RUST_BIN_DIR=%%~fI"

if not exist "%RUST_BIN_DIR%\time_tracer_cli.exe" (
    echo [ERROR] Cannot locate Rust CLI under:
    echo         "%RUST_BIN_DIR%"
    popd >nul
    exit /b 1
)

echo [INFO] Repo Root : "%REPO_ROOT%"
echo [INFO] Rust Bin  : "%RUST_BIN_DIR%"
echo.
echo [STEP 1/1] Running artifact_windows_cli suite...

call %PY_CMD% "test/run.py" "suite" "--suite" "artifact_windows_cli" "--bin-dir" "%RUST_BIN_DIR%" "--no-format-on-success" !EXTRA_ARGS!

if not %errorlevel%==0 (
    echo [FAIL] suite failed with exit code %errorlevel%.
    popd >nul
    exit /b %errorlevel%
)

echo.
echo [PASS] suite tests completed.
popd >nul
exit /b 0
