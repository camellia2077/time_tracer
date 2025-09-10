# config.py
from pathlib import Path

# --- ANSI Color Codes ---
class Colors:
    """A class to hold ANSI color codes for colored console output."""
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    RESET = '\033[0m'

# ======================= 优化后的配置结构 =======================
class Paths:
    """集中管理所有路径和目录配置。"""
    SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/time_master/Time_Master_cpp/time_master/build/bin")
    OUTPUT_ROOT_DIR = Path("C:/Computer/my_github/github_cpp/time_master/my_test")
    SOURCE_DATA_FOLDER_NAME = "Date"
    SOURCE_DATA_PATH = OUTPUT_ROOT_DIR / SOURCE_DATA_FOLDER_NAME
    TARGET_EXECUTABLES_DIR = Path("./")
    OUTPUT_DIR_NAME = "output"
    PROCESSED_DATA_DIR_NAME = f"Processed_{SOURCE_DATA_FOLDER_NAME}"
    PROCESSED_JSON_PATH = OUTPUT_ROOT_DIR / OUTPUT_DIR_NAME / PROCESSED_DATA_DIR_NAME
    
class CLINames:
    """集中管理可执行文件和数据库名称。"""
    EXECUTABLE_CLI_NAME = "time_tracker_cli.exe"
    EXECUTABLE_APP_NAME = "time_tracker_app.exe"
    GENERATED_DB_FILE_NAME = "time_data.db"
    
class TestParams:
    """集中管理所有测试参数。"""
    TEST_FORMATS = ["md", "tex", "typ"]
    DAILY_QUERY_DATES = ["20250501", "20250601"]
    MONTHLY_QUERY_MONTHS = ["202505", "202506"]
    PERIOD_QUERY_DAYS = [7, 10, 15]
    EXPORT_MODE_IS_BULK = False
    SPECIFIC_EXPORT_DATES = ["20250501", "20250601"]
    SPECIFIC_EXPORT_MONTHS = ["202505", "202506"]
    PERIOD_EXPORT_DAYS = [7, 10, 15]
# =========================================================

# --- Artifacts to be cleaned up before each test run ---
FILES_TO_COPY = [
    CLINames.EXECUTABLE_CLI_NAME,
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libsqlite3-0.dll",
    "libwinpthread-1.dll"
]

# [核心修改] 移除 errors.log，因为在清理 output 目录时，它会被一并删除
FILES_TO_CLEAN = [
    CLINames.GENERATED_DB_FILE_NAME,
]

DIRECTORIES_TO_CLEAN = [
    Paths.OUTPUT_DIR_NAME,
    "py_output",
    "config" 
]