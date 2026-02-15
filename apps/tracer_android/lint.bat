@echo off
set "SCRIPT_DIR=%~dp0"
echo [1/3] Running Android Lint...
call "%SCRIPT_DIR%gradlew.bat" -p "%SCRIPT_DIR%." app:lintDebug

echo [2/3] Running Detekt Static Analysis...
call "%SCRIPT_DIR%gradlew.bat" -p "%SCRIPT_DIR%." detekt

echo [3/3] Aggregating results to current directory...

rem Copy Android Lint results
if exist "%SCRIPT_DIR%app\build\reports\lint-results-debug.txt" (
    copy "%SCRIPT_DIR%app\build\reports\lint-results-debug.txt" .\lint-results.txt /Y > nul
    echo SUCCESS: Android Lint results saved to .\lint-results.txt
) else (
    echo WARNING: Android Lint report not found. (Expected: %SCRIPT_DIR%app\build\reports\lint-results-debug.txt)
)

rem Copy Detekt results
if exist "%SCRIPT_DIR%build\reports\detekt\detekt.txt" (
    copy "%SCRIPT_DIR%build\reports\detekt\detekt.txt" .\detekt-results.txt /Y > nul
    echo SUCCESS: Detekt results saved to .\detekt-results.txt
) else if exist "%SCRIPT_DIR%app\build\reports\detekt\detekt.txt" (
     copy "%SCRIPT_DIR%app\build\reports\detekt\detekt.txt" .\detekt-results.txt /Y > nul
     echo SUCCESS: Detekt results saved to .\detekt-results.txt
) else (
    echo WARNING: Detekt report not found.
)

echo.
echo Analysis complete. Check .\lint-results.txt and .\detekt-results.txt
