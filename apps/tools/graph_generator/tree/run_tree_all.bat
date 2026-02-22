@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%"

call "%~dp0env_vars.bat"

call run_tree_png.bat %*
call run_tree_svg.bat %*
call run_tree_html.bat %*

popd
endlocal
