import os
import sys
from pathlib import Path
from typing import Any, Optional, Sequence

from core.main import main

from .args import parse_suite_args
from .formatting import (run_clang_format_after_success,
                         should_run_format_on_success)
from .io import TeeStream, resolve_path, write_result_json


def _resolve_result_json_path(args, suite_root: Path,
                              suite_output_root: Path) -> Path:
    canonical_result_json = (suite_output_root / "result.json").resolve()
    if not args.result_json:
        return canonical_result_json

    requested_result_json = Path(args.result_json)
    if not requested_result_json.is_absolute():
        requested_result_json = (suite_root / requested_result_json).resolve()

    if requested_result_json != canonical_result_json:
        print(
            "Warning: output contract enforces result JSON path at "
            f"{canonical_result_json}; ignoring --result-json={requested_result_json}"
        )

    return canonical_result_json


def _resolve_result_cases_json_path(suite_output_root: Path) -> Path:
    return (suite_output_root / "result_cases.json").resolve()


def _extract_case_records(result_payload: Any) -> list[dict[str, Any]]:
    if not isinstance(result_payload, dict):
        return []

    raw_records = result_payload.pop("_case_records", None)
    if not isinstance(raw_records, list):
        return []

    records: list[dict[str, Any]] = []
    for item in raw_records:
        if isinstance(item, dict):
            records.append(item)
    return records


def _build_result_cases_payload(
    result_payload: Any,
    case_records: list[dict[str, Any]],
) -> dict[str, Any]:
    failed_cases: list[dict[str, Any]] = [
        item for item in case_records if item.get("status") == "FAIL"
    ]

    total_cases = len(case_records)
    total_failed = len(failed_cases)
    if isinstance(result_payload, dict):
        total_cases = int(result_payload.get("total_tests", total_cases) or 0)
        total_failed = int(result_payload.get("total_failed", total_failed) or 0)

    return {
        "success": (total_failed == 0),
        "total_cases": total_cases,
        "total_failed": total_failed,
        "cases": failed_cases,
    }


def _validate_output_contract(
    suite_output_root: Path,
    result_json_path: Path,
    output_log_path: Optional[Path],
    result_payload: Any,
) -> list[str]:
    errors: list[str] = []
    expected_workspace = (suite_output_root / "workspace").resolve()
    expected_logs = (suite_output_root / "logs").resolve()
    expected_artifacts = (suite_output_root / "artifacts").resolve()
    expected_result_json = (suite_output_root / "result.json").resolve()
    expected_output_log = (expected_logs / "output.log").resolve()

    required_dirs = {
        "workspace": expected_workspace,
        "logs": expected_logs,
        "artifacts": expected_artifacts,
    }
    for dir_name, dir_path in required_dirs.items():
        if not dir_path.exists() or not dir_path.is_dir():
            errors.append(f"Missing required output directory: {dir_name} ({dir_path})")

    if result_json_path.resolve() != expected_result_json:
        errors.append(
            "Result JSON path is non-canonical: "
            f"{result_json_path.resolve()} != {expected_result_json}"
        )

    if output_log_path is None:
        errors.append("Missing required output file: logs/output.log (not generated)")
    else:
        if output_log_path.resolve() != expected_output_log:
            errors.append(
                "Python output log path is non-canonical: "
                f"{output_log_path.resolve()} != {expected_output_log}"
            )
        if not expected_output_log.exists():
            errors.append(f"Missing required output file: logs/output.log ({expected_output_log})")

    if isinstance(result_payload, dict):
        result_log_dir = result_payload.get("log_dir")
        if result_log_dir:
            try:
                resolved_log_dir = Path(result_log_dir).resolve()
                if resolved_log_dir != expected_logs:
                    errors.append(
                        "Result payload log_dir is non-canonical: "
                        f"{resolved_log_dir} != {expected_logs}"
                    )
            except (OSError, RuntimeError, TypeError, ValueError):
                errors.append(
                    "Result payload log_dir cannot be resolved: "
                    f"{result_log_dir}"
                )

    return errors


def run_suite(
    argv: Sequence[str],
    suite_root: Path,
    suite_name: str,
    description: str,
    format_app: Optional[str] = None,
    test_root: Optional[Path] = None,
) -> int:
    workspace_root = test_root if test_root else suite_root.parent
    repo_root = workspace_root.parent
    suite_output_root = workspace_root / "output" / suite_name
    logs_root = suite_output_root / "logs"
    logs_root.mkdir(parents=True, exist_ok=True)

    args = parse_suite_args(
        argv=argv,
        description=description,
        enable_format_on_success=bool(format_app),
    )

    config_path = resolve_path(args.config, suite_root, "config.toml")
    result_json_path = _resolve_result_json_path(args, suite_root,
                                                 suite_output_root)
    result_cases_json_path = _resolve_result_cases_json_path(suite_output_root)

    result = None
    exit_code = 1
    temp_log_path = logs_root / "python_output.latest.log"

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

            if should_run_format_on_success(args, bool(format_app)) and \
                    exit_code == 0:
                format_exit_code = run_clang_format_after_success(
                    repo_root=repo_root,
                    app_name=str(format_app),
                )
                if isinstance(result, dict):
                    result["format_ran"] = True
                    result["format_exit_code"] = format_exit_code
                    result["format_success"] = (format_exit_code == 0)
                if format_exit_code != 0:
                    print(
                        f"Warning: clang-format step failed with "
                        f"exit code {format_exit_code}"
                    )
            elif isinstance(result, dict) and bool(format_app):
                result["format_ran"] = False
        except KeyboardInterrupt:
            print("\n[Interrupted by User]")
            exit_code = 130
        finally:
            sys.stdout = original_stdout
            sys.stderr = original_stderr

    output_log_path: Optional[Path] = None
    try:
        output_log_path = (logs_root / "output.log").resolve()
        output_log_path.write_text(
            temp_log_path.read_text(encoding="utf-8"),
            encoding="utf-8",
        )
        print(f"Python output log: {output_log_path}")
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

    if not isinstance(result, dict):
        result = {}
    case_records = _extract_case_records(result)
    result["log_dir"] = str(logs_root.resolve())

    contract_errors = _validate_output_contract(
        suite_output_root=suite_output_root,
        result_json_path=result_json_path,
        output_log_path=output_log_path,
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

    result["output_contract_ok"] = (len(contract_errors) == 0)
    result["output_contract_errors"] = contract_errors
    result["exit_code"] = exit_code
    if exit_code != 0:
        result["success"] = False
    else:
        result["success"] = bool(result.get("success", True))

    try:
        cases_payload = _build_result_cases_payload(result, case_records)
        write_result_json(result_cases_json_path, cases_payload)
        write_result_json(result_json_path, result)
    except Exception as error:
        print(f"Warning: failed to write result JSON: {error}")
        if exit_code == 0:
            exit_code = 1

    return exit_code
