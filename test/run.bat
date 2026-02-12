@echo off
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

setlocal EnableExtensions EnableDelayedExpansion
set "TARGET=time_tracer"
set "EXPECT_SUITE_VALUE=0"

for %%A in (%*) do (
    if /I "%%~A"=="--suite=log_generator" set "TARGET=log_generator"
    if /I "%%~A"=="--suite=lg" set "TARGET=log_generator"
    if /I "%%~A"=="--suite=time_tracer" set "TARGET=time_tracer"
    if /I "%%~A"=="--suite=tt" set "TARGET=time_tracer"
    if "!EXPECT_SUITE_VALUE!"=="1" (
        if /I "%%~A"=="log_generator" set "TARGET=log_generator"
        if /I "%%~A"=="lg" set "TARGET=log_generator"
        if /I "%%~A"=="time_tracer" set "TARGET=time_tracer"
        if /I "%%~A"=="tt" set "TARGET=time_tracer"
        set "EXPECT_SUITE_VALUE=0"
    ) else (
        if /I "%%~A"=="--suite" set "EXPECT_SUITE_VALUE=1"
    )
)

if /I "%TARGET%"=="log_generator" (
    call "%SCRIPT_DIR%run_log_generator.bat" %*
) else (
    call "%SCRIPT_DIR%run_time_tracer.bat" %*
)

exit /b %ERRORLEVEL%
