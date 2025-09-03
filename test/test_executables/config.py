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
SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/time_master/Time_Master_cpp/time_master/build/bin")
SOURCE_DATA_PARENT_DIR = Path("C:/Computer/my_github/github_cpp/time_master/my_test")
# 预处理后的json路径
PROCESSED_JSON_PATH = Path(r"C:\Computer\my_github\github_cpp\time_master\my_test\output\Processed_Date")
db = Path(r"C:\Computer\my_github\github_cpp\time_master\my_test\output\time_data.db")
SOURCE_DATA_FOLDER_NAME = "Date"
SOURCE_DATA_PATH = SOURCE_DATA_PARENT_DIR / SOURCE_DATA_FOLDER_NAME
TARGET_EXECUTABLES_DIR = Path("./") # The current directory where the script is run

# --- Executable and Database Names ---
EXECUTABLE_CLI_NAME = "time_tracker_cli.exe"
EXECUTABLE_APP_NAME = "time_tracker_app.exe"
GENERATED_DB_FILE_NAME = "time_data.db"
PROCESSED_DATA_DIR_NAME = f"Processed_{SOURCE_DATA_FOLDER_NAME}"
OUTPUT_DIR_NAME = "output" # <--- 新增






# ==============================================================================
#                      TEST PARAMETERS CONFIGURATION
# ==============================================================================

# --- 新增: 通用测试格式配置 ---
# "md", "tex", "typ"
# 开发者备注，rst格式暂时还没有实现
TEST_FORMATS = ["md", "tex", "typ"]

# --- Parameters for Query Tests ---
DAILY_QUERY_DATES = ["20250501", "20250601"]
MONTHLY_QUERY_MONTHS = ["202505", "202506"]
PERIOD_QUERY_DAYS = [7, 10, 15]

# --- Parameters for Export Tests ---
#  总开关: True -> 批量导出, False -> 按指定列表导出
EXPORT_MODE_IS_BULK = False
PERIOD_EXPORT_DAYS = [7, 10, 15] 

# 仅在 EXPORT_MODE_IS_BULK = False 时生效
SPECIFIC_EXPORT_DATES = ["20250501", "20250601"]
SPECIFIC_EXPORT_MONTHS = ["202505", "202506"]
# ==============================================================================


# --- Artifacts to be cleaned up before each test run ---
DIRECTORIES_TO_CLEAN = [
    OUTPUT_DIR_NAME,
    "py_output",
    "config" 
]