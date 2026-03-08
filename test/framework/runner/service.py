import os
import sys
from collections.abc import Sequence
from pathlib import Path

from core.main import main

from .args import parse_suite_args
from .formatting import (
    run_clang_format_after_success,
    should_run_format_on_success,
)
from .io import TeeStream, resolve_path, write_result_json
from .log_persistence import TEMP_OUTPUT_LOG_NAME, persist_output_logs
from .output_contract import validate_output_contract
from .result_payload import (
    extract_case_records,
    resolve_result_json_path,
)
from tools.toolchain.core.generated_paths import resolve_test_result_layout


def _cleanup_legacy_result_json_files(suite_output_root: Path) -> None:
    # Keep output root concise for agent consumption: only retain canonical result.json.
    for result_path in suite_output_root.glob("result*.json"):
        if result_path.name == "result.json":
            continue
        try:
            result_path.unlink()
        except OSError:
            pass


def _build_minimal_result_payload(result: dict) -> dict:
    raw_exit_code = result.get("exit_code", 1)
    exit_code = 1 if raw_exit_code is None else int(raw_exit_code)
    payload: dict[str, object] = {
        "success": bool(result.get("success", False)),
        "exit_code": exit_code,
        "total_tests": int(result.get("total_tests", 0) or 0),
        "total_failed": int(result.get("total_failed", 0) or 0),
        "duration_seconds": float(result.get("duration_seconds", 0.0) or 0.0),
        "log_dir": str(result.get("log_dir", "")),
        "output_contract_ok": bool(result.get("output_contract_ok", False)),
    }
    error_message = str(result.get("error_message", "") or "")
    if error_message:
        payload["error_message"] = error_message
    if not payload["success"]:
        payload["next_action"] = "Read logs/output.log for failure details."
    return payload


def run_suite(
    argv: Sequence[str],
    suite_root: Path,
    suite_name: str,
    description: str,
    format_app: str | None = None,
    test_root: Path | None = None,
) -> int:
    workspace_root = test_root if test_root else suite_root.parent
    repo_root = (test_root.parent if test_root else suite_root.parent.parent.parent).resolve()
    suite_output_layout = resolve_test_result_layout(repo_root, suite_name)
    suite_output_root = suite_output_layout.root
    logs_root = suite_output_layout.logs_dir
    logs_root.mkdir(parents=True, exist_ok=True)

    args = parse_suite_args(
        argv=argv,
        description=description,
        enable_format_on_success=bool(format_app),
    )

    config_path = resolve_path(args.config, suite_root, "config.toml")
    result_json_path = resolve_result_json_path(args, suite_root, suite_output_root)
    _cleanup_legacy_result_json_files(suite_output_root)

    result = None
    exit_code = 1
    temp_log_path = logs_root / TEMP_OUTPUT_LOG_NAME

    with temp_log_path.open("w", encoding="utf-8") as session_log_file:
        original_stdout = sys.stdout
        original_stderr = sys.stderr
        sys.stdout = TeeStream(original_stdout, session_log_file)
        sys.stderr = TeeStream(original_stderr, session_log_file)
        try:
            options = {
                "no_color": bool(args.no_color or args.agent),
                "show_output": args.show_output,
                "concise": bool(args.concise and not args.verbose),
            }
            if options["no_color"]:
                os.environ["TT_TEST_NO_COLOR"] = "1"
            result = main(
                config_path=config_path,
                build_dir_name=args.build_dir,
                bin_dir=args.bin_dir,
                options=options,
                return_result=True,
            )
            exit_code = int(result.get("exit_code", 1 if not result.get("success") else 0))
            format_on_success_default = bool(result.get("format_on_success_default", True))

            if (
                should_run_format_on_success(
                    args,
                    bool(format_app),
                    format_on_success_default,
                )
                and exit_code == 0
            ):
                format_exit_code = run_clang_format_after_success(
                    repo_root=repo_root,
                    app_name=str(format_app),
                )
                if isinstance(result, dict):
                    result["format_ran"] = True
                    result["format_exit_code"] = format_exit_code
                    result["format_success"] = format_exit_code == 0
                if format_exit_code != 0:
                    print(f"Warning: clang-format step failed with exit code {format_exit_code}")
            elif isinstance(result, dict) and bool(format_app):
                result["format_ran"] = False
        except KeyboardInterrupt:
            print("\n[Interrupted by User]")
            exit_code = 130
        finally:
            sys.stdout = original_stdout
            sys.stderr = original_stderr

    if not isinstance(result, dict):
        result = {}
    case_records = extract_case_records(result)

    output_log_path: Path | None = None
    output_full_log_path: Path | None = None
    try:
        output_log_path, output_full_log_path = persist_output_logs(
            logs_root=logs_root,
            temp_log_path=temp_log_path,
            case_records=case_records,
        )
        print(f"Python output log: {output_log_path}")
        print(f"Python full output log: {output_full_log_path}")
    except Exception as error:
        print(f"Warning: failed to persist Python output log: {error}")
        if exit_code == 0:
            exit_code = 1
    finally:
        try:
            if temp_log_path.exists():
                temp_log_path.unlink()
        except OSError:
            pass

    result["log_dir"] = str(logs_root.resolve())

    contract_errors = validate_output_contract(
        suite_output_root=suite_output_root,
        result_json_path=result_json_path,
        output_log_path=output_log_path,
        output_full_log_path=output_full_log_path,
        result_payload=result,
    )
    if contract_errors:
        print("Output contract check: FAILED")
        for error in contract_errors:
            print(f"  - {error}")
        if exit_code == 0:
            exit_code = 1
    else:
        print(f"Output contract check: OK ({suite_output_root.resolve()})")

    result["output_contract_ok"] = len(contract_errors) == 0
    result["output_contract_errors"] = contract_errors
    result["exit_code"] = exit_code
    if exit_code != 0:
        result["success"] = False
    else:
        result["success"] = bool(result.get("success", True))

    try:
        write_result_json(result_json_path, _build_minimal_result_payload(result))
    except Exception as error:
        print(f"Warning: failed to write result JSON: {error}")
        if exit_code == 0:
            exit_code = 1

    return exit_code
