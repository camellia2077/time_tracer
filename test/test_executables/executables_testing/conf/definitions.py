# test/conf/definitions.py
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional

# --- ANSI Color Codes ---
@dataclass(frozen=True)
class Colors:
    """控制台输出颜色代码 (使用 frozen=True 确保常量不被修改)"""
    CYAN: str = '\033[96m'
    GREEN: str = '\033[92m'
    RED: str = '\033[91m'
    YELLOW: str = '\033[93m'
    RESET: str = '\033[0m'

# --- Configuration Classes ---
@dataclass
class TestContext:
    """测试上下文：聚合所有 Tester 需要的共享资源"""
    exe_path: Path
    source_data_path: Path
    output_dir: Path
    db_path: Path
    py_output_base_dir: Path
    processed_dir_name: str
    processed_json_path: Optional[Path] = None
    
@dataclass
class Paths:
    """路径配置"""
    SOURCE_EXECUTABLES_DIR: Optional[Path] = None
    SOURCE_DATA_PATH: Optional[Path] = None
    TEST_DATA_ROOT: Optional[Path] = None
    SOURCE_DATA_FOLDER_NAME: Optional[str] = None
    TARGET_EXECUTABLES_DIR: Optional[Path] = None
    DB_DIR: Optional[Path] = None
    EXPORT_OUTPUT_DIR: Optional[Path] = None
    PY_OUTPUT_DIR: Optional[Path] = None
    OUTPUT_DIR_NAME: Optional[str] = None
    PROCESSED_DATA_DIR_NAME: Optional[str] = None
    PROCESSED_JSON_PATH: Optional[Path] = None

@dataclass
class CLINames:
    """可执行文件及数据库名称配置"""
    EXECUTABLE_CLI_NAME: Optional[str] = None
    EXECUTABLE_APP_NAME: Optional[str] = None
    GENERATED_DB_FILE_NAME: Optional[str] = None

@dataclass
class TestParams:
    """测试参数配置"""
    TEST_FORMATS: List[str] = field(default_factory=list)
    DAILY_QUERY_DATES: List[str] = field(default_factory=list)
    MONTHLY_QUERY_MONTHS: List[str] = field(default_factory=list)
    PERIOD_QUERY_DAYS: List[int] = field(default_factory=list)
    EXPORT_MODE_IS_BULK: bool = False
    SPECIFIC_EXPORT_DATES: List[str] = field(default_factory=list)
    SPECIFIC_EXPORT_MONTHS: List[str] = field(default_factory=list)
    PERIOD_EXPORT_DAYS: List[int] = field(default_factory=list)

@dataclass
class Cleanup:
    FILES_TO_COPY: List[str] = field(default_factory=list)
    # [新增] 明确指定要复制的文件夹，而不是硬编码
    FOLDERS_TO_COPY: List[str] = field(default_factory=list) 
    DIRECTORIES_TO_CLEAN: List[str] = field(default_factory=list)

@dataclass
class RunControl:
    """控制环境清理、准备和测试执行开关"""
    ENABLE_ENVIRONMENT_CLEAN: bool = False
    ENABLE_ENVIRONMENT_PREPARE: bool = False
    ENABLE_TEST_EXECUTION: bool = False


@dataclass
class GlobalConfig:
    """聚合所有静态配置的统一对象"""
    paths: Paths
    cli_names: CLINames
    test_params: TestParams
    cleanup: Cleanup
    run_control: RunControl
