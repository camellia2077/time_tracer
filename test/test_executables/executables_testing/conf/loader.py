# test/conf/loader.py
import sys
from pathlib import Path
from .definitions import Paths, CLINames, TestParams, Cleanup, RunControl, GlobalConfig

# Python 3.11+ 内置 tomllib
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError:
        raise ImportError("For Python versions < 3.11, please install 'tomli' using 'pip install tomli'")

def _load_paths(toml_data) -> Paths:
    paths_data = toml_data.get("paths", {})
    paths_inst = Paths()
    
    # 源执行文件目录
    source_exe_dir = paths_data.get("source_executables_dir")
    paths_inst.SOURCE_EXECUTABLES_DIR = Path(source_exe_dir) if source_exe_dir else None
    
    # 源数据路径（强制要求）
    test_data_path_str = paths_data.get("test_data_path")
    if not test_data_path_str:
        raise ValueError("Config error: 'test_data_path' is missing in [paths] section.")
    paths_inst.SOURCE_DATA_PATH = Path(test_data_path_str)
    
    # 自动推导逻辑
    paths_inst.TEST_DATA_ROOT = paths_inst.SOURCE_DATA_PATH.parent
    paths_inst.SOURCE_DATA_FOLDER_NAME = paths_inst.SOURCE_DATA_PATH.name
    
    # [修改] 不再强制要求 target_executables_dir 存在，默认为 None
    # 等待 EnvironmentManager 在运行时动态注入临时路径
    target_exe_dir = paths_data.get("target_executables_dir")
    paths_inst.TARGET_EXECUTABLES_DIR = Path(target_exe_dir) if target_exe_dir else None
    
    # 其它路径配置
    db_dir = paths_data.get("db_dir")
    paths_inst.DB_DIR = Path(db_dir) if db_dir else None
    
    export_dir = paths_data.get("export_output_dir")
    paths_inst.EXPORT_OUTPUT_DIR = Path(export_dir) if export_dir else None
    
    py_output_val = paths_data.get("py_output_dir")
    paths_inst.PY_OUTPUT_DIR = Path(py_output_val) if py_output_val else Path.cwd() / "py_output"
    
    paths_inst.OUTPUT_DIR_NAME = paths_data.get("output_dir_name")

    # 派生目录名称与路径
    paths_inst.PROCESSED_DATA_DIR_NAME = f"Processed_{paths_inst.SOURCE_DATA_FOLDER_NAME}"
    if paths_inst.OUTPUT_DIR_NAME:
        paths_inst.PROCESSED_JSON_PATH = paths_inst.TEST_DATA_ROOT / paths_inst.OUTPUT_DIR_NAME / paths_inst.PROCESSED_DATA_DIR_NAME
    
    return paths_inst
    
def _load_cli_names(toml_data) -> CLINames:
    cli_names_data = toml_data.get("cli_names", {})
    cli_inst = CLINames()
    cli_inst.EXECUTABLE_CLI_NAME = cli_names_data.get("executable_cli_name")
    cli_inst.EXECUTABLE_APP_NAME = cli_names_data.get("executable_app_name")
    cli_inst.GENERATED_DB_FILE_NAME = cli_names_data.get("generated_db_file_name")
    return cli_inst

def _load_test_params(toml_data) -> TestParams:
    test_params_data = toml_data.get("test_params", {})
    params_inst = TestParams()
    params_inst.TEST_FORMATS = test_params_data.get("test_formats", [])
    params_inst.DAILY_QUERY_DATES = test_params_data.get("daily_query_dates", [])
    params_inst.MONTHLY_QUERY_MONTHS = test_params_data.get("monthly_query_months", [])
    params_inst.PERIOD_QUERY_DAYS = test_params_data.get("period_query_days", [])
    
    params_inst.EXPORT_MODE_IS_BULK = bool(test_params_data.get("export_mode_is_bulk", False))
    params_inst.SPECIFIC_EXPORT_DATES = test_params_data.get("specific_export_dates", [])
    params_inst.SPECIFIC_EXPORT_MONTHS = test_params_data.get("specific_export_months", [])
    params_inst.PERIOD_EXPORT_DAYS = test_params_data.get("period_export_days", [])
    return params_inst

def _load_cleanup_params(toml_data) -> Cleanup:
    cleanup_data = toml_data.get("cleanup", {})
    cleanup_inst = Cleanup()
    cleanup_inst.FILES_TO_COPY = cleanup_data.get("files_to_copy", [])
    # [新增] 读取配置，给予默认值以兼容旧代码
    cleanup_inst.FOLDERS_TO_COPY = cleanup_data.get("folders_to_copy", ["config", "plugins"])
    cleanup_inst.DIRECTORIES_TO_CLEAN = cleanup_data.get("directories_to_clean", [])
    return cleanup_inst

def _load_run_control(toml_data) -> RunControl:
    run_control_data = toml_data.get("run_control", {})
    run_inst = RunControl()
    run_inst.ENABLE_ENVIRONMENT_CLEAN = bool(run_control_data.get("enable_environment_clean", True))
    run_inst.ENABLE_ENVIRONMENT_PREPARE = bool(run_control_data.get("enable_environment_prepare", True))
    run_inst.ENABLE_TEST_EXECUTION = bool(run_control_data.get("enable_test_execution", True))
    return run_inst

def load_config() -> GlobalConfig:
    """加载 config.toml 并返回统一的 GlobalConfig 对象。"""
    try:
        with open("config.toml", "rb") as f:
            toml_data = tomllib.load(f)

        # [修改] 不再返回元组，而是返回 GlobalConfig 实例
        return GlobalConfig(
            paths=_load_paths(toml_data),
            cli_names=_load_cli_names(toml_data),
            test_params=_load_test_params(toml_data),
            cleanup=_load_cleanup_params(toml_data),
            run_control=_load_run_control(toml_data)
        )
        
    except FileNotFoundError:
        raise FileNotFoundError("config.toml not found. Please create one in the root directory.")
    except Exception as e:
        raise RuntimeError(f"Error loading config.toml: {e}")

