# main.py
import sys
import shutil
from pathlib import Path
import config
from _py_internal import config
from _py_internal.base_module import TestCounter
from _py_internal.module_preprocessing import PreprocessingTester
from _py_internal.module_database import DatabaseImportTester
from _py_internal.module_query import QueryTester
from _py_internal.module_export import ExportTester

def setup_environment():
    """Validates paths and cleans the environment before tests."""
    print("--- 1. Setting Up Test Environment ---")
    
    # Validate required source paths
    if not config.SOURCE_EXECUTABLES_DIR.exists():
        print(f"Error: Source executables directory not found: {config.SOURCE_EXECUTABLES_DIR}")
        sys.exit(1)
    if not config.SOURCE_DATA_PATH.exists():
        print(f"Error: Source data path not found: {config.SOURCE_DATA_PATH}")
        sys.exit(1)
    print("✔️ Source paths validated.")

    # Clean up artifacts from previous runs
    db_file = Path.cwd() / config.GENERATED_DB_FILE_NAME
    processed_dir = Path.cwd() / config.PROCESSED_DATA_DIR_NAME
    if db_file.exists():
        print(f"Deleting old database file: {db_file.name}")
        db_file.unlink()
    if processed_dir.exists():
        print(f"Deleting old processed data directory: {processed_dir.name}")
        shutil.rmtree(processed_dir)
    
    # Copy fresh executables
    print("Copying executables...")
    executables = [config.EXECUTABLE_CLI_NAME, config.EXECUTABLE_APP_NAME]
    for exe_name in executables:
        source_file = config.SOURCE_EXECUTABLES_DIR / exe_name
        target_file = config.TARGET_EXECUTABLES_DIR / exe_name
        if target_file.exists():
            target_file.unlink()
        shutil.copy2(source_file, target_file)
    print("✔️ Environment is clean and ready.")

def main():
    """Main function to run all test modules."""
    setup_environment()
    
    shared_counter = TestCounter()
    
    # Instantiate all test modules
    modules = [
        PreprocessingTester(shared_counter, 1),
        DatabaseImportTester(shared_counter, 2),
        QueryTester(shared_counter, 3),
        ExportTester(shared_counter, 4)
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