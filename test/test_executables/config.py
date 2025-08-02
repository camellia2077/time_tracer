# config.py
from pathlib import Path

# --- ANSI Color Codes ---
class Colors:
    """A class to hold ANSI color codes for colored console output."""
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    RESET = '\033[0m'

# --- File and Directory Paths ---
# Note: Adjust SOURCE_EXECUTABLES_DIR if your build directory is different.
SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/New_time_master/Time_Master_cpp/time_master/build")
SOURCE_DATA_PARENT_DIR = Path("C:/Computer/my_github/github_cpp/New_time_master/my_test")
SOURCE_DATA_FOLDER_NAME = "Date"
SOURCE_DATA_PATH = SOURCE_DATA_PARENT_DIR / SOURCE_DATA_FOLDER_NAME
TARGET_EXECUTABLES_DIR = Path("./") # The current directory where the script is run

# --- Executable and Database Names ---
EXECUTABLE_CLI_NAME = "time_tracker_cli.exe"
EXECUTABLE_APP_NAME = "time_tracker_app.exe"
GENERATED_DB_FILE_NAME = "time_data.db"
PROCESSED_DATA_DIR_NAME = f"Processed_{SOURCE_DATA_FOLDER_NAME}"

# --- Test Parameters for Query and Export ---
DAILY_QUERY_DATE = "20250501"
MONTHLY_QUERY_MONTH = "202505"
PERIOD_QUERY_DAYS = "7,10,15"

# ==============================================================================
#                      EXPORT MODULE CONFIGURATION
# ==============================================================================
#  Master switch to control the export testing mode.
#  True:  Run "bulk export" tests (exports all daily and monthly reports).
#  False: Run "specific export" tests (exports for a single specified day/month).
# ==============================================================================
EXPORT_MODE_IS_BULK = False

# This setting is used for period reports regardless of the mode.
PERIOD_EXPORT_DAYS = "7,10,15" 

# These settings are only used when EXPORT_MODE_IS_BULK is False.
SPECIFIC_EXPORT_DATE = "20250501"
SPECIFIC_EXPORT_MONTH = "202505"
# ==============================================================================


# --- Artifacts to be cleaned up before each test run ---
DIRECTORIES_TO_CLEAN = [
    PROCESSED_DATA_DIR_NAME,
    "Processed_Date", # Legacy name, can be removed if no longer generated
    "output",
    "exported_files"
]