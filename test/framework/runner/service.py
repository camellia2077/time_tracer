import os
import sys
from collections.abc import Sequence
from pathlib import Path
from typing import Any

from core.main import main

from .args import parse_suite_args
from .formatting import run_clang_format_after_success, should_run_format_on_success
from .io import TeeStream, resolve_path, write_result_json

_OUTPUT_LOG_NAME = "output.log"
_FULL_OUTPUT_LOG_NAME = "output_full.log"
_TEMP_OUTPUT_LOG_NAME = "python_output.latest.log"


def _resolve_result_json_path(args, suite_root: Path, suite_output_root: Path) -> Path:
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
    output_log_path: Path | None,
    output_full_log_path: Path | None,
    result_payload: Any,
) -> list[str]:
    errors: list[str] = []
    expected_workspace = (suite_output_root / "workspace").resolve()
    expected_logs = (suite_output_root / "logs").resolve()
    expected_artifacts = (suite_output_root / "artifacts").resolve()
    expected_result_json = (suite_output_root / "result.json").resolve()
    expected_output_log = (expected_logs / _OUTPUT_LOG_NAME).resolve()
    expected_output_full_log = (expected_logs / _FULL_OUTPUT_LOG_NAME).resolve()

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

    if output_full_log_path is None:
        errors.append(f"Missing required output file: logs/{_FULL_OUTPUT_LOG_NAME} (not generated)")
    else:
        if output_full_log_path.resolve() != expected_output_full_log:
            errors.append(
                "Full output log path is non-canonical: "
                f"{output_full_log_path.resolve()} != {expected_output_full_log}"
            )
        if not expected_output_full_log.exists():
            errors.append(
                "Missing required output file: "
                f"logs/{_FULL_OUTPUT_LOG_NAME} ({expected_output_full_log})"
            )

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
                errors.append(f"Result payload log_dir cannot be resolved: {result_log_dir}")

    return errors


def _collect_case_log_paths(
    logs_root: Path,
    case_records: list[dict[str, Any]],
) -> list[Path]:
    root_resolved = logs_root.resolve()
    seen: set[Path] = set()
    collected: list[Path] = []

    for item in case_records:
        if not isinstance(item, dict):
            continue
        log_file = str(item.get("log_file", "")).strip()
        if not log_file:
            continue

        candidate = (logs_root / log_file).resolve()
        try:
            candidate.relative_to(root_resolved)
        except ValueError:
            continue
        if not candidate.is_file() or candidate in seen:
            continue
        seen.add(candidate)
        collected.append(candidate)

    if collected:
        collected.sort(key=lambda path: path.relative_to(root_resolved).as_posix())
        return collected

    ignored_names = {_OUTPUT_LOG_NAME, _FULL_OUTPUT_LOG_NAME, _TEMP_OUTPUT_LOG_NAME}
    for path in logs_root.rglob("*.log"):
        if not path.is_file():
            continue
        if path.name in ignored_names:
            continue
        resolved = path.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)
        collected.append(resolved)

    collected.sort(key=lambda path: path.relative_to(root_resolved).as_posix())
    return collected


def _persist_output_logs(
    logs_root: Path,
    temp_log_path: Path,
    case_records: list[dict[str, Any]],
) -> tuple[Path, Path]:
    concise_output_path = (logs_root / _OUTPUT_LOG_NAME).resolve()
    concise_text = temp_log_path.read_text(encoding="utf-8")
    concise_output_path.write_text(concise_text, encoding="utf-8")

    full_output_path = (logs_root / _FULL_OUTPUT_LOG_NAME).resolve()
    case_log_paths = _collect_case_log_paths(logs_root, case_records)
    with full_output_path.open("w", encoding="utf-8") as handle:
        handle.write("=== Session Output (Concise) ===\n")
        handle.write(concise_text)
        if not concise_text.endswith("\n"):
            handle.write("\n")

        handle.write("\n=== Case Logs (Full Test Content) ===\n")
        for case_log_path in case_log_paths:
            relative_path = case_log_path.relative_to(logs_root).as_posix()
            handle.write(f"\n--- {relative_path} ---\n")
            case_log_content = case_log_path.read_text(encoding="utf-8", errors="ignore")
            handle.write(case_log_content)
            if not case_log_content.endswith("\n"):
                handle.write("\n")

    return concise_output_path, full_output_path


def run_suite(
    argv: Sequence[str],
    suite_root: Path,
    suite_name: str,
    description: str,
    format_app: str | None = None,
    test_root: Path | None = None,
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
    result_json_path = _resolve_result_json_path(args, suite_root, suite_output_root)
    result_cases_json_path = _resolve_result_cases_json_path(suite_output_root)

    result = None
    exit_code = 1
    temp_log_path = logs_root / _TEMP_OUTPUT_LOG_NAME

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

            if should_run_format_on_success(args, bool(format_app)) and exit_code == 0:
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
    case_records = _extract_case_records(result)

    output_log_path: Path | None = None
    output_full_log_path: Path | None = None
    try:
        output_log_path, output_full_log_path = _persist_output_logs(
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

    contract_errors = _validate_output_contract(
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
        cases_payload = _build_result_cases_payload(result, case_records)
        write_result_json(result_cases_json_path, cases_payload)
        write_result_json(result_json_path, result)
    except Exception as error:
        print(f"Warning: failed to write result JSON: {error}")
        if exit_code == 0:
            exit_code = 1

    return exit_code
