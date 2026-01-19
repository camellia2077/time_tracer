# test/conf/definitions.py
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional

# --- ANSI Color Codes ---
@dataclass(frozen=True)
class Colors:
    CYAN: str = '\033[96m'
    GREEN: str = '\033[92m'
    RED: str = '\033[91m'
    YELLOW: str = '\033[93m'
    RESET: str = '\033[0m'

# --- Execution & Report Structures (New) ---
@dataclass
class ExecutionResult:
    """单个命令执行的底层结果 (moved from executor.py)"""
    command: list
    return_code: int
    stdout: str
    stderr: str
    duration: float
    error: Optional[str] = None

@dataclass
class SingleTestResult:
    """单个测试用例的业务结果"""
    name: str
    status: str  # "PASS" or "FAIL"
    execution_result: Optional[ExecutionResult] = None
    messages: List[str] = field(default_factory=list) # 用于携带额外信息(如文件大小)

@dataclass
class TestReport:
    """一个测试模块(BaseTester子类)的完整报告"""
    module_name: str
    results: List[SingleTestResult] = field(default_factory=list)
    
    @property
    def passed_count(self) -> int:
        return sum(1 for r in self.results if r.status == "PASS")
    
    @property
    def failed_count(self) -> int:
        return sum(1 for r in self.results if r.status == "FAIL")

# --- Configuration Classes ---
@dataclass
class TestContext:
    exe_path: Path
    source_data_path: Path
    output_dir: Path
    db_path: Path
    py_output_base_dir: Path
    processed_dir_name: str
    processed_json_path: Optional[Path] = None
    
@dataclass
class Paths:
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
    EXECUTABLE_CLI_NAME: Optional[str] = None
    EXECUTABLE_APP_NAME: Optional[str] = None
    GENERATED_DB_FILE_NAME: Optional[str] = None

@dataclass
class TestParams:
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
    FOLDERS_TO_COPY: List[str] = field(default_factory=list) 
    DIRECTORIES_TO_CLEAN: List[str] = field(default_factory=list)

@dataclass
class RunControl:
    ENABLE_ENVIRONMENT_CLEAN: bool = False
    ENABLE_ENVIRONMENT_PREPARE: bool = False
    ENABLE_TEST_EXECUTION: bool = False

@dataclass
class GlobalConfig:
    paths: Paths
    cli_names: CLINames
    test_params: TestParams
    cleanup: Cleanup
    run_control: RunControl