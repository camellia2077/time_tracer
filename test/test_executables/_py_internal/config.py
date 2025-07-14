# config.py
from pathlib import Path
from datetime import datetime

# --- Global Configuration Parameters ---

# Executable file names
EXECUTABLE_CLI_NAME = "time_tracker_cli.exe"
EXECUTABLE_APP_NAME = "time_tracker_app.exe"

# Directory paths
# IMPORTANT: Update these paths to match your system
SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/Time_Master_cpp/time_master/build")
SOURCE_DATA_PATH = Path("C:/Computer/my_github/github_cpp/Time_Master_cpp/test/Date_test")
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
PERIOD_QUERY_DAYS = "7"
PERIOD_EXPORT_DAYS = "7,30,90"