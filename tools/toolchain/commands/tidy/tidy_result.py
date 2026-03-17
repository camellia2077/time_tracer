from __future__ import annotations

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
from .task_log import list_task_paths, load_task_record
from .workspace import DEFAULT_TIDY_BUILD_DIR_NAME

def _collect_task_logs(root: Path) -> list[Path]:
    if not root.exists():
        return []
    return list_task_paths(root)


def _build_blocking_files(ctx: Context, pending_logs: list[Path]) -> tuple[list[dict], dict]:
    blocking_files: list[dict] = []
    strategy_counts = {strategy: 0 for strategy in ALL_STRATEGIES}
    seen_files: set[str] = set()
    strategy_cfg = ctx.config.tidy.fix_strategy

    for log_path in pending_logs:
        parsed = load_task_record(log_path)
        checks = list(parsed.checks)
        primary_strategy = resolve_primary_strategy(checks, strategy_cfg)
        strategy_counts[primary_strategy] = strategy_counts.get(primary_strategy, 0) + 1
        source_file = parsed.source_file or str(log_path)
        if source_file in seen_files:
            continue
        seen_files.add(source_file)
        check_strategies: dict[str, str] = {}
        for check in checks:
            check_strategies[check] = resolve_fix_strategy(check, strategy_cfg)
        blocking_files.append(
            {
                "task_id": parsed.task_id,
                "batch_id": parsed.batch_id,
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
    build_dir_name: str,
    source_scope: str | None,
) -> str:
    tidy_args = ""
    normalized_build_dir = (build_dir_name or "").strip()
    normalized_scope = (source_scope or "").strip()
    if normalized_build_dir and normalized_build_dir != DEFAULT_TIDY_BUILD_DIR_NAME:
        tidy_args += f" --tidy-build-dir {normalized_build_dir}"
    if normalized_scope:
        tidy_args += f" --source-scope {normalized_scope}"

    if pending_count <= 0:
        if stage == "tidy-close" and status == "completed":
            return "No pending tasks. Tidy close completed."
        if stage == "tidy-close" and status == "completed_tidy_only":
            return (
                "No pending tasks. Tidy-only close completed. "
                "If needed, run: python tools/run.py verify --app "
                f"{app_name} --build-dir build_fast --concise"
            )
        if verify_mode == "skip":
            return (
                "No pending tasks. Optional full close: "
                f"python tools/run.py tidy-close --app {app_name}{tidy_args} --keep-going --concise"
            )
        return (
            "No pending tasks. Close with: "
            f"python tools/run.py tidy-close --app {app_name}{tidy_args} --keep-going --concise"
        )

    first_block = blocking_files[0] if blocking_files else {}
    batch_id = first_block.get("batch_id") or "<BATCH_ID>"
    task_id = first_block.get("task_id") or "<TASK_ID>"
    primary_strategy = first_block.get("primary_fix_strategy", STRATEGY_MANUAL_ONLY)
    if primary_strategy == STRATEGY_MANUAL_ONLY:
        return (
            f"Handle manual-only task_{task_id} first, then run: "
            f"python tools/run.py tidy-batch --app {app_name}{tidy_args} --batch-id {batch_id} --preset sop"
        )
    return (
        f"Fix task_{task_id} ({primary_strategy}), then run: "
        f"python tools/run.py tidy-batch --app {app_name}{tidy_args} --batch-id {batch_id} --preset sop"
    )


def write_tidy_result(
    *,
    ctx: Context,
    app_name: str,
    stage: str,
    status: str,
    exit_code: int,
    build_dir_name: str = DEFAULT_TIDY_BUILD_DIR_NAME,
    verify_mode: str = "full",
    next_action: str | None = None,
    source_scope: str | None = None,
) -> Path:
    tidy_layout = ctx.get_tidy_layout(app_name, build_dir_name)
    tasks_dir = tidy_layout.tasks_dir
    tasks_done_dir = tidy_layout.tasks_done_dir
    result_path = tidy_layout.tidy_result_path

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
            build_dir_name=build_dir_name,
            source_scope=source_scope,
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
