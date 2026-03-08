from __future__ import annotations

import json
from pathlib import Path

from ....services.suite_registry import resolve_result_output_name


def _cleanup_legacy_result_json_files(output_root: Path) -> None:
    for result_path in output_root.glob("result*.json"):
        if result_path.name == "result.json":
            continue
        try:
            result_path.unlink()
        except OSError:
            pass


def write_build_only_result_json(
    repo_root: Path,
    app_name: str,
    build_dir_name: str,
    success: bool,
    exit_code: int,
    duration_seconds: float,
    error_message: str = "",
    build_only: bool = True,
) -> None:
    output_name = resolve_result_output_name(app_name)
    output_root = repo_root / "test" / "output" / output_name
    logs_dir = output_root / "logs"
    output_root.mkdir(parents=True, exist_ok=True)
    logs_dir.mkdir(parents=True, exist_ok=True)
    _cleanup_legacy_result_json_files(output_root)

    result_payload = {
        "success": success,
        "exit_code": exit_code,
        "total_tests": 1,
        "total_failed": 0 if success else 1,
        "duration_seconds": round(duration_seconds, 3),
        "log_dir": str(logs_dir.resolve()),
        "build_only": build_only,
        "build_dir": build_dir_name,
    }
    if not success and error_message:
        result_payload["error_message"] = error_message
    (output_root / "result.json").write_text(
        json.dumps(result_payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
