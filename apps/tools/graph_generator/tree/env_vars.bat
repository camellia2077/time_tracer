@echo off

set "SCRIPT_PATH=C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\apps\graph_generator\tree\run_tree.py"
set "CONFIG_DIR=C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\apps\graph_generator\tree\configs"
set "DB_PATH=C:\Computer\my_github\github_cpp\time_tracer\my_test\time_tracer_test\output\db\time_data.sqlite3"
set "OUT_DIR=C:\Computer\my_github\github_cpp\time_tracer\my_test\graph_generator\tree\output"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
