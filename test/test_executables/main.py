# main.py
import sys
import shutil
from pathlib import Path
from datetime import datetime

# --- Configuration Parameters (Merged from config.py) ---

# Executable file names
EXECUTABLE_CLI_NAME = "time_tracker_cli.exe"
EXECUTABLE_APP_NAME = "time_tracker_app.exe"

# Directory paths
# IMPORTANT: Update these paths to match your system
SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/New_time_master/Time_Master_cpp/time_master/build")
SOURCE_DATA_PATH = Path("C:/Computer/my_github/github_cpp/New_time_master//my_test/Date_test")
TARGET_EXECUTABLES_DIR = Path("./") # Current directory

# File and directory names for processing
EXECUTABLE_TO_RUN = "time_tracker_cli.exe"
PROCESSED_DATA_DIR_NAME = "Processed_Date_test"
GENERATED_DB_FILE_NAME = "time_data.db"
CONVERTED_TEXT_DIR_NAME = "Processed_Date_test"

# --- Query and Export Module Parameters ---

# Dates are generated automatically from the current system time
DAILY_QUERY_DATE = datetime.now().strftime("%Y%m%d")
MONTHLY_QUERY_MONTH = datetime.now().strftime("%Y%m")

# Default query periods
PERIOD_QUERY_DAYS = "7,30,90"
PERIOD_EXPORT_DAYS = "7,30,90"

# --- Python Internal Modules ---
from _py_internal.base_module import TestCounter
from _py_internal.module_preprocessing import PreprocessingTester
from _py_internal.module_database import DatabaseImportTester
from _py_internal.module_query import QueryTester
from _py_internal.module_export import ExportTester

def setup_environment():
    """Validates paths and cleans the environment before tests."""
    print("--- 1. Setting Up Test Environment ---")
    
    # Validate required source paths
    if not SOURCE_EXECUTABLES_DIR.exists():
        print(f"Error: Source executables directory not found: {SOURCE_EXECUTABLES_DIR}")
        sys.exit(1)
    if not SOURCE_DATA_PATH.exists():
        print(f"Error: Source data path not found: {SOURCE_DATA_PATH}")
        sys.exit(1)
    print("✔️ Source paths validated.")

    # Clean up artifacts from previous runs
    db_file = Path.cwd() / GENERATED_DB_FILE_NAME
    processed_dir = Path.cwd() / PROCESSED_DATA_DIR_NAME
    if db_file.exists():
        print(f"Deleting old database file: {db_file.name}")
        db_file.unlink()
    if processed_dir.exists():
        print(f"Deleting old processed data directory: {processed_dir.name}")
        shutil.rmtree(processed_dir)
    
    # Copy fresh executables
    print("Copying executables...")
    executables = [EXECUTABLE_CLI_NAME, EXECUTABLE_APP_NAME]
    for exe_name in executables:
        source_file = SOURCE_EXECUTABLES_DIR / exe_name
        target_file = TARGET_EXECUTABLES_DIR / exe_name
        if target_file.exists():
            target_file.unlink()
        shutil.copy2(source_file, target_file)
    print("✔️ Environment is clean and ready.")

def main():
    """Main function to run all test modules."""
    setup_environment()
    
    shared_counter = TestCounter()
    
    # [FIXED] Pass the required configuration arguments when creating each test module.
    common_args = {
        "executable_to_run": EXECUTABLE_TO_RUN,
        "source_data_path": SOURCE_DATA_PATH,
        "converted_text_dir_name": CONVERTED_TEXT_DIR_NAME
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
    
    # Run each module
    for module in modules:
        module.reports_dir.mkdir(parents=True, exist_ok=True)
        module.run_tests()

    # --- Final Summary ---
    output_dir = Path.cwd() / "output"
    final_message = f"""
================================================================================
--- All Tests Completed ---
Logs for each module are saved in the 'output' directory: {output_dir.resolve()}
================================================================================
"""
    print(final_message)

if __name__ == "__main__":
    main()