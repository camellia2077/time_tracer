from __future__ import annotations

import json
from pathlib import Path


def _build_verify_phase_summary(
    verify_phases: list[dict[str, object]],
) -> dict[str, int]:
    summary = {
        "total": len(verify_phases),
        "passed": 0,
        "failed": 0,
        "skipped": 0,
    }
    for phase in verify_phases:
        status = str(phase.get("status", "") or "").strip().lower()
        if status == "passed":
            summary["passed"] += 1
        elif status == "failed":
            summary["failed"] += 1
        elif status == "skipped":
            summary["skipped"] += 1
    return summary

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
    verify_phases: list[dict[str, object]] | None = None,
) -> None:
    from ....core.generated_paths import resolve_test_result_layout_for_app

    layout = resolve_test_result_layout_for_app(repo_root, app_name)
    output_root = layout.root
    logs_dir = layout.logs_dir
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
    normalized_verify_phases = list(verify_phases or [])
    if normalized_verify_phases:
        result_payload["verify_phases"] = normalized_verify_phases
        result_payload["verify_phase_summary"] = _build_verify_phase_summary(
            normalized_verify_phases
        )
    layout.result_json_path.write_text(
        json.dumps(result_payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )


def merge_verify_phase_summary_into_result_json(
    repo_root: Path,
    app_name: str,
    verify_phases: list[dict[str, object]],
) -> None:
    from ....core.generated_paths import resolve_test_result_layout_for_app

    if not verify_phases:
        return

    layout = resolve_test_result_layout_for_app(repo_root, app_name)
    result_path = layout.result_json_path
    if result_path.exists():
        try:
            payload = json.loads(result_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            payload = {}
    else:
        payload = {}

    payload["verify_phases"] = list(verify_phases)
    payload["verify_phase_summary"] = _build_verify_phase_summary(list(verify_phases))
    result_path.parent.mkdir(parents=True, exist_ok=True)
    result_path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
