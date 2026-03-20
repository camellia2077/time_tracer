from __future__ import annotations

from pathlib import Path

from ...core.generated_paths import resolve_test_result_layout_for_app
from ...services.log_analysis import extract_key_error_lines_from_log


def resolve_result_paths(repo_root: Path, app_name: str) -> dict[str, Path]:
    layout = resolve_test_result_layout_for_app(repo_root, app_name)
    return {
        "output_root": layout.root,
        "result_json": layout.result_json_path,
        "output_log": layout.output_log_path,
    }


def print_result_paths(
    *,
    app_name: str,
    repo_root: Path,
    state_json: Path | None = None,
) -> None:
    paths = resolve_result_paths(repo_root=repo_root, app_name=app_name)
    print("--- result visibility")
    if state_json is not None:
        print(f"state_json: {state_json}")
    print(f"result_json (summary): {paths['result_json']}")
    print(f"output_log (aggregated log): {paths['output_log']}")
def print_failure_report(
    *,
    command: str,
    exit_code: int,
    next_action: str,
    app_name: str,
    repo_root: Path,
    state_json: Path | None = None,
    log_path: Path | None = None,
    fallback_key_error_hint: str = "See command output above.",
    include_result_json: bool = True,
) -> None:
    paths = resolve_result_paths(repo_root=repo_root, app_name=app_name)
    resolved_log_path = log_path or paths["output_log"]
    key_error_lines = extract_key_error_lines_from_log(resolved_log_path)

    print("=== FAILURE REPORT ===")
    print(f"command: {command}")
    print(f"exit_code: {exit_code}")
    if state_json is not None:
        print(f"state_json: {state_json}")
    if include_result_json:
        print(f"result_json: {paths['result_json']}")
    print(f"output_log: {resolved_log_path}")
    print("key_error_lines:")
    if key_error_lines:
        for line in key_error_lines:
            print(f"  - {line}")
    else:
        print(f"  - {fallback_key_error_hint}")
    print(f"next_action: {next_action}")
    print("======================")
