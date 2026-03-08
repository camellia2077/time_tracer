from __future__ import annotations

import re
from pathlib import Path

from ...services.suite_registry import resolve_result_output_name


def resolve_result_paths(repo_root: Path, app_name: str) -> dict[str, Path]:
    suite_or_app = resolve_result_output_name(app_name)
    output_root = repo_root / "test" / "output" / suite_or_app
    return {
        "output_root": output_root,
        "result_json": output_root / "result.json",
        "output_log": output_root / "logs" / "output.log",
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


def _extract_key_error_lines_from_log(log_path: Path, limit: int = 5) -> list[str]:
    if not log_path.exists() or not log_path.is_file():
        return []

    try:
        lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return []

    if not lines:
        return []

    pattern = re.compile(r"(error|failed|fatal|exception|traceback|not found)", re.IGNORECASE)
    selected: list[str] = []
    seen: set[str] = set()

    for line in reversed(lines):
        text = line.strip()
        if not text:
            continue
        if pattern.search(text):
            if text not in seen:
                selected.append(text)
                seen.add(text)
            if len(selected) >= limit:
                break

    if not selected:
        for line in reversed(lines):
            text = line.strip()
            if not text:
                continue
            if text not in seen:
                selected.append(text)
                seen.add(text)
            if len(selected) >= min(3, limit):
                break

    selected.reverse()
    return selected


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
) -> None:
    paths = resolve_result_paths(repo_root=repo_root, app_name=app_name)
    resolved_log_path = log_path or paths["output_log"]
    key_error_lines = _extract_key_error_lines_from_log(resolved_log_path)

    print("=== FAILURE REPORT ===")
    print(f"command: {command}")
    print(f"exit_code: {exit_code}")
    if state_json is not None:
        print(f"state_json: {state_json}")
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
