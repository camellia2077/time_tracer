# config.py
import sys
from pathlib import Path

# 从 Python 3.11+ 开始，tomllib 是内置模块，不再需要单独安装 tomli。
# 如果使用旧版本，则需要安装 tomli。
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError:
        raise ImportError("For Python versions < 3.11, please install 'tomli' using 'pip install tomli'")

# --- ANSI Color Codes ---
class Colors:
    """A class to hold ANSI color codes for colored console output."""
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    RESET = '\033[0m'

# --- Configuration Classes (to be populated from TOML) ---
class Paths:
    pass

class CLINames:
    pass

class TestParams:
    pass

class Cleanup:
    pass

# --- TOML Loader Function ---
def _load_config():
    """Load configuration from config.toml and populate the classes."""
    try:
        with open("config.toml", "rb") as f:
            toml_data = tomllib.load(f)

        # Populate Paths class
        paths_data = toml_data.get("paths", {})
        Paths.SOURCE_EXECUTABLES_DIR = Path(paths_data.get("source_executables_dir"))
        Paths.OUTPUT_ROOT_DIR = Path(paths_data.get("output_root_dir"))
        Paths.SOURCE_DATA_FOLDER_NAME = paths_data.get("source_data_folder_name")
        Paths.TARGET_EXECUTABLES_DIR = Path(paths_data.get("target_executables_dir"))
        Paths.OUTPUT_DIR_NAME = paths_data.get("output_dir_name")
        Paths.SOURCE_DATA_PATH = Paths.OUTPUT_ROOT_DIR / Paths.SOURCE_DATA_FOLDER_NAME
        Paths.PROCESSED_DATA_DIR_NAME = f"Processed_{Paths.SOURCE_DATA_FOLDER_NAME}"
        Paths.PROCESSED_JSON_PATH = Paths.OUTPUT_ROOT_DIR / Paths.OUTPUT_DIR_NAME / Paths.PROCESSED_DATA_DIR_NAME
        
        # Populate CLINames class
        cli_names_data = toml_data.get("cli_names", {})
        CLINames.EXECUTABLE_CLI_NAME = cli_names_data.get("executable_cli_name")
        CLINames.EXECUTABLE_APP_NAME = cli_names_data.get("executable_app_name")
        CLINames.GENERATED_DB_FILE_NAME = cli_names_data.get("generated_db_file_name")
        
        # Populate TestParams class
        test_params_data = toml_data.get("test_params", {})
        TestParams.TEST_FORMATS = test_params_data.get("test_formats", [])
        TestParams.DAILY_QUERY_DATES = test_params_data.get("daily_query_dates", [])
        TestParams.MONTHLY_QUERY_MONTHS = test_params_data.get("monthly_query_months", [])
        TestParams.PERIOD_QUERY_DAYS = test_params_data.get("period_query_days", [])
        TestParams.EXPORT_MODE_IS_BULK = test_params_data.get("export_mode_is_bulk", False)
        TestParams.SPECIFIC_EXPORT_DATES = test_params_data.get("specific_export_dates", [])
        TestParams.SPECIFIC_EXPORT_MONTHS = test_params_data.get("specific_export_months", [])
        TestParams.PERIOD_EXPORT_DAYS = test_params_data.get("period_export_days", [])

        # Populate Cleanup class
        cleanup_data = toml_data.get("cleanup", {})
        Cleanup.FILES_TO_COPY = cleanup_data.get("files_to_copy", [])
        Cleanup.FILES_TO_CLEAN = cleanup_data.get("files_to_clean", [])
        Cleanup.DIRECTORIES_TO_CLEAN = cleanup_data.get("directories_to_clean", [])
        
    except FileNotFoundError:
        raise FileNotFoundError("config.toml not found. Please create one.")
    except Exception as e:
        raise RuntimeError(f"Error loading config.toml: {e}")

_load_config()