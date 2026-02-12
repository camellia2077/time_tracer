@echo off
setlocal

call "%~dp0env_vars.bat"

python "%SCRIPT_PATH%" tree --config-dir "%CONFIG_DIR%" --database "%DB_PATH%" --output-directory "%OUT_DIR%" --output-formats svg --no-output-html --no-output-json %*
endlocal
