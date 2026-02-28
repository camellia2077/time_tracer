# src/tt_testing/main.py
from pathlib import Path

from .conf.definitions import Colors
from .conf.loader import load_config
from .core.engine import TestEngine


def print_header(paths, no_color: bool):
    import datetime

    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    color_cyan = "" if no_color else Colors.CYAN
    color_green = "" if no_color else Colors.GREEN
    color_reset = "" if no_color else Colors.RESET
    print(f"{color_cyan}" + "=" * 80 + f"{color_reset}")
    print(f" Running Python test script: {Path(__file__).name} (Started at: {now})")
    print(f"{color_cyan}" + "=" * 80 + f"{color_reset}")

    print(f" {color_green}[Configuration Summary]{color_reset}")
    print(f"  - Source Binaries : {paths.SOURCE_EXECUTABLES_DIR}")
    print(f"  - Source Data     : {paths.SOURCE_DATA_PATH}")
    print(f"  - Test Environment: {paths.TARGET_EXECUTABLES_DIR}")
    print(f"  - Database File   : {paths.DB_DIR}")
    print(f"  - Export Output   : {paths.EXPORT_OUTPUT_DIR}")
    print(f"  - Python Logs     : {paths.PY_OUTPUT_DIR}")
    print(f"{color_cyan}" + "-" * 80 + f"{color_reset}\n")


def _build_error_result(error_message: str, log_dir: str) -> dict:
    bootstrap_case = {
        "module": "runner",
        "name": "bootstrap",
        "status": "FAIL",
        "log_file": "",
        "messages": [error_message],
        "return_code": 1,
        "command": [],
        "stderr_tail": [error_message],
        "stdout_tail": [],
    }
    return {
        "success": False,
        "exit_code": 1,
        "total_tests": 0,
        "total_failed": 1,
        "duration_seconds": 0.0,
        "log_dir": log_dir,
        "modules": [],
        "failed_cases": [bootstrap_case],
        "_case_records": [bootstrap_case],
        "error_message": error_message,
    }


def main(
    config_path: Path | None = None,
    build_dir_name: str | None = None,
    bin_dir: str | None = None,
    options: dict | None = None,
    return_result: bool = False,
):
    run_options = options or {}
    no_color = bool(run_options.get("no_color", False))
    log_dir = ""
    try:
        config = load_config(config_path, build_dir_name=build_dir_name, bin_dir=bin_dir)
        if config.paths.PY_OUTPUT_DIR:
            log_dir = str(config.paths.PY_OUTPUT_DIR)
    except Exception as error:
        error_message = f"Config Error: {error}"
        color_red = "" if no_color else Colors.RED
        color_reset = "" if no_color else Colors.RESET
        print(f"{color_red}{error_message}{color_reset}")
        result = _build_error_result(error_message, log_dir)
        return result if return_result else result["exit_code"]

    print_header(config.paths, no_color)
    try:
        engine = TestEngine(config, options=run_options)
        success = engine.run()
        result = engine.get_result()
        result["_case_records"] = engine.get_case_records()
        result["success"] = success
        result["exit_code"] = 0 if success else 1
        return result if return_result else result["exit_code"]
    except Exception as error:
        error_message = f"Runtime Error: {error}"
        color_red = "" if no_color else Colors.RED
        color_reset = "" if no_color else Colors.RESET
        print(f"{color_red}{error_message}{color_reset}")
        result = _build_error_result(error_message, log_dir)
        return result if return_result else result["exit_code"]
