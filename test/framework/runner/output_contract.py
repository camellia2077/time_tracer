from pathlib import Path
from typing import Any

from .log_persistence import FULL_OUTPUT_LOG_NAME, OUTPUT_LOG_NAME


def validate_output_contract(
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
    expected_output_log = (expected_logs / OUTPUT_LOG_NAME).resolve()
    expected_output_full_log = (expected_logs / FULL_OUTPUT_LOG_NAME).resolve()

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
        errors.append(f"Missing required output file: logs/{FULL_OUTPUT_LOG_NAME} (not generated)")
    else:
        if output_full_log_path.resolve() != expected_output_full_log:
            errors.append(
                "Full output log path is non-canonical: "
                f"{output_full_log_path.resolve()} != {expected_output_full_log}"
            )
        if not expected_output_full_log.exists():
            errors.append(
                "Missing required output file: "
                f"logs/{FULL_OUTPUT_LOG_NAME} ({expected_output_full_log})"
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
