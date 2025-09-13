# _py_internal/config_loader.py
import sys
from pathlib import Path

# 从 Python 3.11+ 开始，tomllib 是内置模块
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError:
        raise ImportError("For Python versions < 3.11, please install 'tomli' using 'pip install tomli'")

def _load_paths(toml_data, paths_class):
    """从 TOML 数据中加载路径配置并填充 Paths 类。"""
    paths_data = toml_data.get("paths", {})
    paths_class.SOURCE_EXECUTABLES_DIR = Path(paths_data.get("source_executables_dir"))
    paths_class.OUTPUT_ROOT_DIR = Path(paths_data.get("output_root_dir"))
    paths_class.SOURCE_DATA_FOLDER_NAME = paths_data.get("source_data_folder_name")
    paths_class.TARGET_EXECUTABLES_DIR = Path(paths_data.get("target_executables_dir"))
    paths_class.OUTPUT_DIR_NAME = paths_data.get("output_dir_name")
    
    # 派生路径
    paths_class.SOURCE_DATA_PATH = paths_class.OUTPUT_ROOT_DIR / paths_class.SOURCE_DATA_FOLDER_NAME
    paths_class.PROCESSED_DATA_DIR_NAME = f"Processed_{paths_class.SOURCE_DATA_FOLDER_NAME}"
    paths_class.PROCESSED_JSON_PATH = paths_class.OUTPUT_ROOT_DIR / paths_class.OUTPUT_DIR_NAME / paths_class.PROCESSED_DATA_DIR_NAME
    
def _load_cli_names(toml_data, cli_names_class):
    """从 TOML 数据中加载可执行文件和数据库名称。"""
    cli_names_data = toml_data.get("cli_names", {})
    cli_names_class.EXECUTABLE_CLI_NAME = cli_names_data.get("executable_cli_name")
    cli_names_class.EXECUTABLE_APP_NAME = cli_names_data.get("executable_app_name")
    cli_names_class.GENERATED_DB_FILE_NAME = cli_names_data.get("generated_db_file_name")

def _load_test_params(toml_data, test_params_class):
    """从 TOML 数据中加载测试参数。"""
    test_params_data = toml_data.get("test_params", {})
    test_params_class.TEST_FORMATS = test_params_data.get("test_formats", [])
    test_params_class.DAILY_QUERY_DATES = test_params_data.get("daily_query_dates", [])
    test_params_class.MONTHLY_QUERY_MONTHS = test_params_data.get("monthly_query_months", [])
    test_params_class.PERIOD_QUERY_DAYS = test_params_data.get("period_query_days", [])
    test_params_class.EXPORT_MODE_IS_BULK = test_params_data.get("export_mode_is_bulk", False)
    test_params_class.SPECIFIC_EXPORT_DATES = test_params_data.get("specific_export_dates", [])
    test_params_class.SPECIFIC_EXPORT_MONTHS = test_params_data.get("specific_export_months", [])
    test_params_class.PERIOD_EXPORT_DAYS = test_params_data.get("period_export_days", [])

def _load_cleanup_params(toml_data, cleanup_class):
    """从 TOML 数据中加载清理参数。"""
    cleanup_data = toml_data.get("cleanup", {})
    cleanup_class.FILES_TO_COPY = cleanup_data.get("files_to_copy", [])
    # ======================= 核心修改 =======================
    # cleanup_class.FILES_TO_CLEAN = cleanup_data.get("files_to_clean", []) # <--- 已移除
    # =========================================================
    cleanup_class.DIRECTORIES_TO_CLEAN = cleanup_data.get("directories_to_clean", [])


def load_config(paths_class, cli_names_class, test_params_class, cleanup_class):
    """
    主加载函数，协调所有配置的加载。
    """
    try:
        with open("config.toml", "rb") as f:
            toml_data = tomllib.load(f)

        _load_paths(toml_data, paths_class)
        _load_cli_names(toml_data, cli_names_class)
        _load_test_params(toml_data, test_params_class)
        _load_cleanup_params(toml_data, cleanup_class)
        
    except FileNotFoundError:
        raise FileNotFoundError("config.toml not found. Please create one.")
    except Exception as e:
        raise RuntimeError(f"Error loading config.toml: {e}")