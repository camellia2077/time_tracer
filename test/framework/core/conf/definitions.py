# test/conf/definitions.py
from dataclasses import dataclass, field
from pathlib import Path


@dataclass(frozen=True)
class Colors:
    CYAN: str = "\033[96m"
    GREEN: str = "\033[92m"
    RED: str = "\033[91m"
    YELLOW: str = "\033[93m"
    RESET: str = "\033[0m"


class AppExitCode:
    SUCCESS = 0
    GENERIC_ERROR = 1
    COMMAND_NOT_FOUND = 2
    INVALID_ARGUMENTS = 3
    DATABASE_ERROR = 4
    IO_ERROR = 5
    LOGIC_ERROR = 6
    CONFIG_ERROR = 7
    MEMORY_ERROR = 8
    UNKNOWN_ERROR = 9
    DLL_COMPATIBILITY_ERROR = 10

    _MAPPING = {
        SUCCESS: "Success",
        GENERIC_ERROR: "Generic Error (Check logs)",
        COMMAND_NOT_FOUND: "Unknown Command",
        INVALID_ARGUMENTS: "Invalid Arguments",
        DATABASE_ERROR: "Database Access Error",
        IO_ERROR: "File I/O Error",
        LOGIC_ERROR: "Internal Logic Error",
        CONFIG_ERROR: "Configuration Error",
        MEMORY_ERROR: "Memory Allocation Error",
        UNKNOWN_ERROR: "Unknown/Unexpected Error",
        DLL_COMPATIBILITY_ERROR: "DLL Compatibility Error",
    }

    @classmethod
    def to_string(cls, code: int) -> str:
        return cls._MAPPING.get(code, f"Undefined Error ({code})")


@dataclass
class ExecutionResult:
    command: list
    return_code: int
    stdout: str
    stderr: str
    duration: float
    error: str | None = None


@dataclass
class SingleTestResult:
    name: str
    status: str  # "PASS" or "FAIL"
    execution_result: ExecutionResult | None = None
    messages: list[str] = field(default_factory=list)
    log_file: str | None = None


@dataclass
class TestReport:
    module_name: str
    results: list[SingleTestResult] = field(default_factory=list)
    duration: float = 0.0

    @property
    def passed_count(self) -> int:
        return sum(1 for item in self.results if item.status == "PASS")

    @property
    def failed_count(self) -> int:
        return sum(1 for item in self.results if item.status == "FAIL")


@dataclass
class TestContext:
    exe_path: Path
    source_data_path: Path
    output_dir: Path
    db_path: Path
    export_output_dir: Path | None
    processed_json_dir: Path | None
    py_output_base_dir: Path
    processed_dir_name: str
    processed_json_path: Path | None = None


@dataclass
class CommandSpec:
    name: str
    args: list[str]
    stage: str = "commands"
    expect_exit: int = 0
    raw_command: bool = False
    add_output_dir: bool = False
    stdin_input: str | None = None
    expect_files: list[str] = field(default_factory=list)
    # Each item format: "<absolute_or_formatted_path>::<required_substring>"
    expect_file_contains: list[str] = field(default_factory=list)
    expect_stdout_contains: list[str] = field(default_factory=list)
    expect_stderr_contains: list[str] = field(default_factory=list)


@dataclass
class Paths:
    PROJECT_APPS_ROOT: Path | None = None
    DEFAULT_BUILD_DIR: str | None = None
    SOURCE_EXECUTABLES_DIR: Path | None = None
    SOURCE_DATA_PATH: Path | None = None
    TEST_DATA_ROOT: Path | None = None
    SOURCE_DATA_FOLDER_NAME: str | None = None
    TARGET_EXECUTABLES_DIR: Path | None = None
    DB_DIR: Path | None = None
    EXPORT_OUTPUT_DIR: Path | None = None
    PY_OUTPUT_DIR: Path | None = None
    OUTPUT_DIR_NAME: Path | None = None
    PROCESSED_DATA_DIR_NAME: str | None = None
    PROCESSED_JSON_PATH: Path | None = None
    PROCESSED_JSON_DIR: Path | None = None


@dataclass
class CLINames:
    EXECUTABLE_CLI_NAME: str | None = None
    EXECUTABLE_APP_NAME: str | None = None
    GENERATED_DB_FILE_NAME: str | None = None


@dataclass
class TestParams:
    TEST_FORMATS: list[str] = field(default_factory=list)
    DAILY_QUERY_DATES: list[str] = field(default_factory=list)
    MONTHLY_QUERY_MONTHS: list[str] = field(default_factory=list)
    WEEKLY_QUERY_WEEKS: list[str] = field(default_factory=list)
    YEARLY_QUERY_YEARS: list[str] = field(default_factory=list)
    RECENT_QUERY_DAYS: list[int] = field(default_factory=list)
    EXPORT_MODE_IS_BULK: bool = False
    SPECIFIC_EXPORT_DATES: list[str] = field(default_factory=list)
    SPECIFIC_EXPORT_MONTHS: list[str] = field(default_factory=list)
    SPECIFIC_EXPORT_WEEKS: list[str] = field(default_factory=list)
    SPECIFIC_EXPORT_YEARS: list[str] = field(default_factory=list)
    RECENT_EXPORT_DAYS: list[int] = field(default_factory=list)


@dataclass
class Cleanup:
    FILES_TO_COPY: list[str] = field(default_factory=list)
    FOLDERS_TO_COPY: list[str] = field(default_factory=list)
    DIRECTORIES_TO_CLEAN: list[str] = field(default_factory=list)


@dataclass
class LogRoutingRule:
    stage: str
    subdir: str
    command_prefix: list[str] = field(default_factory=list)
    legacy_name_contains: list[str] = field(default_factory=list)


@dataclass
class LogRoutingConfig:
    rules: list[LogRoutingRule] = field(default_factory=list)


@dataclass
class RunControl:
    ENABLE_ENVIRONMENT_CLEAN: bool = False
    ENABLE_ENVIRONMENT_PREPARE: bool = False
    ENABLE_TEST_EXECUTION: bool = False
    STOP_ON_FAILURE: bool = True


@dataclass
class PipelineConfig:
    MODE: str = "ingest"  # "ingest" or "staged" or "none"


@dataclass
class GlobalConfig:
    paths: Paths
    cli_names: CLINames
    test_params: TestParams
    cleanup: Cleanup
    log_routing: LogRoutingConfig
    run_control: RunControl
    pipeline: PipelineConfig
    commands: list[CommandSpec] = field(default_factory=list)
