from pathlib import Path

from .definitions import (
    CLINames,
    Cleanup,
    LogRoutingConfig,
    LogRoutingRule,
    Paths,
    PipelineConfig,
    RunControl,
    TestParams,
)


def _normalize_list(value):
    if value is None:
        return []
    if isinstance(value, list):
        return value
    return [value]


def _load_paths(toml_data) -> Paths:
    paths_data = toml_data.get("paths", {})
    paths_inst = Paths()
    default_build_dir = paths_data.get("default_build_dir")
    if isinstance(default_build_dir, str):
        default_build_dir = default_build_dir.strip()
    paths_inst.DEFAULT_BUILD_DIR = default_build_dir or None

    project_apps_root = paths_data.get("project_apps_root")
    paths_inst.PROJECT_APPS_ROOT = (
        Path(project_apps_root) if project_apps_root else None
    )

    source_exe_dir = paths_data.get("source_executables_dir")
    paths_inst.SOURCE_EXECUTABLES_DIR = Path(source_exe_dir) if source_exe_dir else None

    bin_dir = toml_data.get("_bin_dir")
    build_dir_name = toml_data.get("_build_dir_name")
    if not build_dir_name:
        build_dir_name = paths_inst.DEFAULT_BUILD_DIR
    if bin_dir:
        paths_inst.SOURCE_EXECUTABLES_DIR = Path(bin_dir)
        print(
            f"  - Binary directory override active: {paths_inst.SOURCE_EXECUTABLES_DIR}"
        )
    else:
        build_dir_name = toml_data.get("_build_dir_name")
        if build_dir_name and paths_inst.PROJECT_APPS_ROOT:
            paths_inst.SOURCE_EXECUTABLES_DIR = (
                paths_inst.PROJECT_APPS_ROOT / build_dir_name / "bin"
            )
            print(f"  - Build Folder override active: Using {build_dir_name}")
    if build_dir_name and not paths_inst.PROJECT_APPS_ROOT:
        raise ValueError(
            "Config error: [paths].project_apps_root is required when using --build-dir."
        )
    if not bin_dir and not build_dir_name:
        raise ValueError(
            "Config error: executable directory is not configured. "
            "Pass --build-dir/--bin-dir, or set [paths].default_build_dir."
        )

    test_data_path_str = paths_data.get("test_data_path")
    if not test_data_path_str:
        raise ValueError(
            "Config error: 'test_data_path' is missing in [paths] section."
        )
    paths_inst.SOURCE_DATA_PATH = Path(test_data_path_str)

    paths_inst.TEST_DATA_ROOT = paths_inst.SOURCE_DATA_PATH.parent
    paths_inst.SOURCE_DATA_FOLDER_NAME = paths_inst.SOURCE_DATA_PATH.name

    target_exe_dir = paths_data.get("target_executables_dir")
    paths_inst.TARGET_EXECUTABLES_DIR = Path(target_exe_dir) if target_exe_dir else None

    db_dir = paths_data.get("db_dir")
    paths_inst.DB_DIR = Path(db_dir) if db_dir else None

    export_dir = paths_data.get("export_output_dir")
    paths_inst.EXPORT_OUTPUT_DIR = Path(export_dir) if export_dir else None

    py_output_val = paths_data.get("py_output_dir")
    paths_inst.PY_OUTPUT_DIR = (
        Path(py_output_val) if py_output_val else Path.cwd() / "py_output"
    )

    output_dir_name = paths_data.get("output_dir_name")
    paths_inst.OUTPUT_DIR_NAME = Path(output_dir_name) if output_dir_name else None

    paths_inst.PROCESSED_DATA_DIR_NAME = (
        f"Processed_{paths_inst.SOURCE_DATA_FOLDER_NAME}"
    )
    processed_json_dir = paths_data.get("processed_json_dir")
    paths_inst.PROCESSED_JSON_DIR = (
        Path(processed_json_dir) if processed_json_dir else None
    )

    if paths_inst.PROCESSED_JSON_DIR:
        paths_inst.PROCESSED_JSON_PATH = paths_inst.PROCESSED_JSON_DIR
    elif paths_inst.OUTPUT_DIR_NAME:
        paths_inst.PROCESSED_JSON_PATH = (
            paths_inst.OUTPUT_DIR_NAME / paths_inst.PROCESSED_DATA_DIR_NAME
        )

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
    params_inst.WEEKLY_QUERY_WEEKS = test_params_data.get("weekly_query_weeks", [])
    params_inst.YEARLY_QUERY_YEARS = test_params_data.get("yearly_query_years", [])
    params_inst.RECENT_QUERY_DAYS = test_params_data.get("recent_query_days", [])

    params_inst.EXPORT_MODE_IS_BULK = bool(
        test_params_data.get("export_mode_is_bulk", False)
    )
    params_inst.SPECIFIC_EXPORT_DATES = test_params_data.get(
        "specific_export_dates", []
    )
    params_inst.SPECIFIC_EXPORT_MONTHS = test_params_data.get(
        "specific_export_months", []
    )
    params_inst.SPECIFIC_EXPORT_WEEKS = test_params_data.get(
        "specific_export_weeks", []
    )
    params_inst.SPECIFIC_EXPORT_YEARS = test_params_data.get(
        "specific_export_years", []
    )
    params_inst.RECENT_EXPORT_DAYS = test_params_data.get("recent_export_days", [])
    return params_inst


def _load_cleanup_params(toml_data) -> Cleanup:
    cleanup_data = toml_data.get("cleanup", {})
    cleanup_inst = Cleanup()
    cleanup_inst.FILES_TO_COPY = cleanup_data.get("files_to_copy", [])
    cleanup_inst.FOLDERS_TO_COPY = cleanup_data.get(
        "folders_to_copy", ["config", "plugins"]
    )
    cleanup_inst.DIRECTORIES_TO_CLEAN = cleanup_data.get("directories_to_clean", [])
    return cleanup_inst


def _load_log_routing(toml_data) -> LogRoutingConfig:
    log_routing_data = toml_data.get("log_routing", {})
    if not isinstance(log_routing_data, dict):
        return LogRoutingConfig()

    raw_rules = log_routing_data.get("rules", [])
    if not isinstance(raw_rules, list):
        return LogRoutingConfig()

    rules: list[LogRoutingRule] = []
    for item in raw_rules:
        if not isinstance(item, dict):
            continue

        stage = str(item.get("stage", "")).strip()
        subdir = str(item.get("subdir", "")).strip().strip("/\\")
        if not stage or not subdir:
            continue

        command_prefix = [
            str(token).strip().lower()
            for token in _normalize_list(item.get("command_prefix"))
            if str(token).strip()
        ]
        legacy_name_contains = [
            str(token).strip().lower()
            for token in _normalize_list(item.get("legacy_name_contains"))
            if str(token).strip()
        ]

        rules.append(
            LogRoutingRule(
                stage=stage,
                subdir=subdir,
                command_prefix=command_prefix,
                legacy_name_contains=legacy_name_contains,
            )
        )

    return LogRoutingConfig(rules=rules)


def _load_run_control(toml_data) -> RunControl:
    run_control_data = toml_data.get("run_control", {})
    run_inst = RunControl()
    run_inst.ENABLE_ENVIRONMENT_CLEAN = bool(
        run_control_data.get("enable_environment_clean", True)
    )
    run_inst.ENABLE_ENVIRONMENT_PREPARE = bool(
        run_control_data.get("enable_environment_prepare", True)
    )
    run_inst.ENABLE_TEST_EXECUTION = bool(
        run_control_data.get("enable_test_execution", True)
    )
    run_inst.STOP_ON_FAILURE = bool(run_control_data.get("stop_on_failure", True))
    return run_inst


def _load_pipeline(toml_data) -> PipelineConfig:
    pipeline_data = toml_data.get("pipeline", {})
    mode = str(pipeline_data.get("mode", "ingest")).strip().lower()
    if mode not in {"ingest", "staged", "none"}:
        raise ValueError(
            "Config error: [pipeline].mode must be 'ingest', 'staged', or 'none'."
        )

    return PipelineConfig(MODE=mode)
