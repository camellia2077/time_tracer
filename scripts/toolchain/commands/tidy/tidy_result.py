from __future__ import annotations

import re
from datetime import UTC, datetime
from pathlib import Path

from ...core.context import Context
from ..shared import tidy as tidy_shared
from .fix_strategy import (
    ALL_STRATEGIES,
    STRATEGY_MANUAL_ONLY,
    resolve_fix_strategy,
    resolve_primary_strategy,
)

_TASK_ID_PATTERN = re.compile(r"^task_(\d+)\.log$")
_CHECK_TYPE_PATTERN = re.compile(r"([A-Za-z0-9_.-]+)\(\d+\)")
_CHECK_BRACKET_PATTERN = re.compile(r"\[([A-Za-z0-9_.-]+)\]")


def _parse_task_log(task_log_path: Path) -> dict:
    task_id_match = _TASK_ID_PATTERN.match(task_log_path.name)
    task_id = task_id_match.group(1).zfill(3) if task_id_match else ""
    batch_id = task_log_path.parent.name if task_log_path.parent else ""
    try:
        lines = task_log_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    except OSError:
        lines = []

    source_file = ""
    checks: list[str] = []

    for line in lines:
        if line.startswith("File: ") and not source_file:
            source_file = line[len("File: ") :].strip()
            continue
        if line.startswith("Types: "):
            checks.extend(_CHECK_TYPE_PATTERN.findall(line))

    if not checks:
        for line in lines:
            checks.extend(_CHECK_BRACKET_PATTERN.findall(line))

    normalized_checks: list[str] = []
    for check in checks:
        text = check.strip()
        if not text or text in normalized_checks:
            continue
        normalized_checks.append(text)

    return {
        "task_id": task_id,
        "batch_id": batch_id,
        "source_file": source_file,
        "checks": normalized_checks,
    }


def _collect_task_logs(root: Path) -> list[Path]:
    if not root.exists():
        return []
    return sorted(root.rglob("task_*.log"), key=lambda item: str(item))


def _build_blocking_files(ctx: Context, pending_logs: list[Path]) -> tuple[list[dict], dict]:
    blocking_files: list[dict] = []
    strategy_counts = {strategy: 0 for strategy in ALL_STRATEGIES}
    seen_files: set[str] = set()
    strategy_cfg = ctx.config.tidy.fix_strategy

    for log_path in pending_logs:
        parsed = _parse_task_log(log_path)
        checks = parsed["checks"]
        primary_strategy = resolve_primary_strategy(checks, strategy_cfg)
        strategy_counts[primary_strategy] = strategy_counts.get(primary_strategy, 0) + 1
        source_file = parsed["source_file"] or str(log_path)
        if source_file in seen_files:
            continue
        seen_files.add(source_file)
        check_strategies: dict[str, str] = {}
        for check in checks:
            check_strategies[check] = resolve_fix_strategy(check, strategy_cfg)
        blocking_files.append(
            {
                "task_id": parsed["task_id"],
                "batch_id": parsed["batch_id"],
                "source_file": source_file,
                "checks": checks,
                "check_fix_strategy": check_strategies,
                "primary_fix_strategy": primary_strategy,
            }
        )
    return blocking_files, strategy_counts


def _default_next_action(
    app_name: str,
    pending_count: int,
    blocking_files: list[dict],
    stage: str,
    status: str,
    verify_mode: str,
) -> str:
    if pending_count <= 0:
        if stage == "tidy-close" and status == "completed":
            return "No pending tasks. Tidy close completed."
        if stage == "tidy-close" and status == "completed_tidy_only":
            return (
                "No pending tasks. Tidy-only close completed. "
                "If needed, run: python scripts/run.py verify --app "
                f"{app_name} --build-dir build_fast --concise"
            )
        if verify_mode == "skip":
            return (
                "No pending tasks. Optional full close: "
                f"python scripts/run.py tidy-close --app {app_name} --keep-going --concise"
            )
        return (
            "No pending tasks. Close with: "
            f"python scripts/run.py tidy-close --app {app_name} --keep-going --concise"
        )

    first_block = blocking_files[0] if blocking_files else {}
    batch_id = first_block.get("batch_id") or "<BATCH_ID>"
    task_id = first_block.get("task_id") or "<TASK_ID>"
    primary_strategy = first_block.get("primary_fix_strategy", STRATEGY_MANUAL_ONLY)
    if primary_strategy == STRATEGY_MANUAL_ONLY:
        return (
            f"Handle manual-only task_{task_id} first, then run: "
            f"python scripts/run.py tidy-batch --app {app_name} --batch-id {batch_id} --preset sop"
        )
    return (
        f"Fix task_{task_id} ({primary_strategy}), then run: "
        f"python scripts/run.py tidy-batch --app {app_name} --batch-id {batch_id} --preset sop"
    )


def write_tidy_result(
    *,
    ctx: Context,
    app_name: str,
    stage: str,
    status: str,
    exit_code: int,
    build_dir_name: str = "build_tidy",
    verify_mode: str = "full",
    next_action: str | None = None,
) -> Path:
    app_dir = ctx.get_app_dir(app_name)
    build_tidy_dir = app_dir / build_dir_name
    tasks_dir = build_tidy_dir / "tasks"
    tasks_done_dir = build_tidy_dir / "tasks_done"
    result_path = build_tidy_dir / "tidy_result.json"

    pending_logs = _collect_task_logs(tasks_dir)
    archived_logs = _collect_task_logs(tasks_done_dir)
    blocking_files, strategy_counts = _build_blocking_files(ctx, pending_logs)

    remaining_tasks = len(pending_logs)
    total_tasks = remaining_tasks + len(archived_logs)
    if next_action is None:
        next_action = _default_next_action(
            app_name=app_name,
            pending_count=remaining_tasks,
            blocking_files=blocking_files,
            stage=stage,
            status=status,
            verify_mode=verify_mode,
        )

    payload = {
        "generated_at": datetime.now(UTC).isoformat(),
        "app": app_name,
        "stage": stage,
        "status": status,
        "exit_code": int(exit_code),
        "verify_mode": verify_mode,
        "tasks": {
            "total": total_tasks,
            "remaining": remaining_tasks,
            "archived": len(archived_logs),
        },
        "blocking_summary": strategy_counts,
        "blocking_files": blocking_files[:20],
        "fix_strategy_rules": {
            "auto_fix": ctx.config.tidy.fix_strategy.auto_fix,
            "safe_refactor": ctx.config.tidy.fix_strategy.safe_refactor,
            "nolint_allowed": ctx.config.tidy.fix_strategy.nolint_allowed,
            "manual_only": ctx.config.tidy.fix_strategy.manual_only,
        },
        "next_action": next_action,
    }
    tidy_shared.write_json_dict(result_path, payload)
    print(f"--- tidy-result: {result_path}")
    return result_path
