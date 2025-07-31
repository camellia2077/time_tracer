# main.py
import sys
import shutil
import os
from pathlib import Path
from datetime import datetime

# --- ANSI Color Codes ---
class Colors:
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    RESET = '\033[0m'

# --- Configuration Parameters ---
# MODIFICATION: Separated the parent directory and the specific data folder name.
# To test a new folder, you only need to change SOURCE_DATA_FOLDER_NAME.
SOURCE_DATA_PARENT_DIR = Path("C:/Computer/my_github/github_cpp/New_time_master/my_test")
SOURCE_DATA_FOLDER_NAME = "Date_mini" 

# The full path to the source data is now constructed automatically.
SOURCE_DATA_PATH = SOURCE_DATA_PARENT_DIR / SOURCE_DATA_FOLDER_NAME

# MODIFICATION: The processed directory name is now generated dynamically.
# It will be "Processed_" + whatever SOURCE_DATA_FOLDER_NAME is.
PROCESSED_DATA_DIR_NAME = f"Processed_{SOURCE_DATA_FOLDER_NAME}"

# Executable configuration remains the same.
EXECUTABLE_CLI_NAME = "time_tracker_cli.exe"
EXECUTABLE_APP_NAME = "time_tracker_app.exe"
SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/New_time_master/Time_Master_cpp/time_master/build")
TARGET_EXECUTABLES_DIR = Path("./") 

# Constants for the program's operation
EXECUTABLE_TO_RUN = "time_tracker_cli.exe"
GENERATED_DB_FILE_NAME = "time_data.db"

# Query and Export parameters
DAILY_QUERY_DATE = datetime.now().strftime("%Y%m%d")
MONTHLY_QUERY_MONTH = datetime.now().strftime("%Y%m")
PERIOD_QUERY_DAYS = "7,10,15"
PERIOD_EXPORT_DAYS = "7,10,15"

# --- Python Internal Modules ---
from _py_internal.base_module import TestCounter
from _py_internal.module_preprocessing import PreprocessingTester
from _py_internal.module_database import DatabaseImportTester
from _py_internal.module_query import QueryTester
from _py_internal.module_export import ExportTester

def setup_environment():
    """验证路径、复制可执行文件并清理环境。"""
    print(f"{Colors.CYAN}--- 1. Preparing Executable ---{Colors.RESET}")
    
    if not SOURCE_EXECUTABLES_DIR.exists():
        print(f"  {Colors.RED}错误: 源目录不存在: {SOURCE_EXECUTABLES_DIR}{Colors.RESET}")
        sys.exit(1)

    executables_to_copy = [EXECUTABLE_CLI_NAME, EXECUTABLE_APP_NAME]
    for exe_name in executables_to_copy:
        source_path = SOURCE_EXECUTABLES_DIR / exe_name
        target_path = TARGET_EXECUTABLES_DIR / exe_name

        if not source_path.exists():
            print(f"  {Colors.RED}警告: 在源目录中未找到可执行文件: {exe_name}{Colors.RESET}")
            continue

        try:
            shutil.copy(source_path, target_path)
            print(f"  {Colors.GREEN}已成功复制: {exe_name}{Colors.RESET}")
        except Exception as e:
            print(f"  {Colors.RED}复制文件时出错 {exe_name}: {e}{Colors.RESET}")
            sys.exit(1)

    print("  可执行文件已准备就绪。")
    
    print(f"{Colors.CYAN}--- 2. Cleaning Artifacts & Setting up Directories ---{Colors.RESET}")
    db_file = Path.cwd() / GENERATED_DB_FILE_NAME
    
    # MODIFICATION: The script now uses the dynamically generated name for cleanup.
    processed_dir = Path.cwd() / PROCESSED_DATA_DIR_NAME 
    
    output_dir = Path.cwd() / "output"
    export_dir = Path.cwd() / "Export"

    if processed_dir.exists(): shutil.rmtree(processed_dir)
    if export_dir.exists(): shutil.rmtree(export_dir)
    if output_dir.exists(): shutil.rmtree(output_dir)
    if db_file.exists(): db_file.unlink()
    
    output_dir.mkdir(parents=True, exist_ok=True)
    print(f"  {Colors.GREEN}清理完成，已创建 'output' 日志目录。{Colors.RESET}")


def main():
    """Main function to run all test modules."""
    os.system('') 
    
    print("\n" + "="*50)
    print(f" Running Python test script: {Path(__file__).name}")
    print(f" Current directory: {Path.cwd()}")
    print(f" Input data folder: {SOURCE_DATA_FOLDER_NAME}")
    print(f" Expecting processed folder: {PROCESSED_DATA_DIR_NAME}")
    print("="*50 + "\n")
    
    setup_environment()
    
    print("\n========== Starting Test Sequence ==========")
    
    shared_counter = TestCounter()
    
    # MODIFICATION: Pass the new dynamic PROCESSED_DATA_DIR_NAME to the test modules.
    common_args = {
        "executable_to_run": EXECUTABLE_TO_RUN,
        "source_data_path": SOURCE_DATA_PATH,
        "converted_text_dir_name": PROCESSED_DATA_DIR_NAME
    }

    modules = [
        PreprocessingTester(shared_counter, 1, **common_args),
        DatabaseImportTester(shared_counter, 2, **common_args),
        QueryTester(shared_counter, 3, 
                    generated_db_file_name=GENERATED_DB_FILE_NAME, 
                    daily_query_date=DAILY_QUERY_DATE, 
                    monthly_query_month=MONTHLY_QUERY_MONTH, 
                    period_query_days=PERIOD_QUERY_DAYS,
                    **common_args),
        ExportTester(shared_counter, 4, 
                     generated_db_file_name=GENERATED_DB_FILE_NAME, 
                     period_export_days=PERIOD_EXPORT_DAYS,
                     **common_args)
    ]
    
    all_tests_passed = True
    for i, module in enumerate(modules, 1):
        module.reports_dir.mkdir(parents=True, exist_ok=True)
        print(f"{Colors.CYAN}--- {i}. Running {module.module_name} Tasks ---{Colors.RESET}")
        
        if not module.run_tests():
            all_tests_passed = False
            print(f"\n{Colors.RED}错误: 测试序列因 '{module.module_name}' 模块执行失败而中断。{Colors.RESET}")
            break

    if all_tests_passed:
        final_message = f"""
{Colors.GREEN}✅ All test steps completed successfully!{Colors.RESET}
   Check the 'output' directory for detailed logs.
"""
        print(final_message)


if __name__ == "__main__":
    main()