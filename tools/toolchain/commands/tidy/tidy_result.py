from __future__ import annotations

from datetime import UTC, datetime
from pathlib import Path

from ...core.context import Context
from ..shared import tidy as tidy_shared
from ..clang.tidy import config as clang_tidy_config
from .fix_strategy import (
    ALL_STRATEGIES,
    STRATEGY_MANUAL_ONLY,
    resolve_fix_strategy,
    resolve_primary_strategy,
)
from .queue.source_cluster import collect_source_clusters
from .queue.task_log import list_task_paths, load_task_record
from .workspace import DEFAULT_TIDY_BUILD_DIR_NAME


RECOMMENDED_ACTION_RECHECK_FIRST = "recheck_first"


def _collect_task_logs(root: Path) -> list[Path]:
    return list_task_paths(root) if root.exists() else []


def _build_queue_head(pending_logs: list[Path]) -> dict | None:
    if not pending_logs:
        return None
    head_path = pending_logs[0]
    parsed = load_task_record(head_path)
    return {
        "cluster_id": parsed.cluster_id,
        "task_id": parsed.task_id,
        "source_file": parsed.source_file or str(head_path),
        "task_log": str(head_path),
        "checks": list(parsed.checks),
    }


def _queue_head_label(queue_head: dict | None) -> str:
    if queue_head is None:
        return "unknown"
    return f"{queue_head.get('cluster_id', '<CLUSTER_ID>')}/task_{queue_head.get('task_id', '<TASK_ID>')}"


def _build_blocking_files(ctx: Context, pending_logs: list[Path]) -> tuple[list[dict], dict, int]:
    blocking_files: list[dict] = []
    strategy_counts = {strategy: 0 for strategy in ALL_STRATEGIES}
    recheck_first_candidates = 0
    strategy_cfg = ctx.config.tidy.fix_strategy
    pending_root = pending_logs[0].parents[2] if pending_logs else None
    clusters = collect_source_clusters(pending_root) if pending_root else ()
    for cluster in clusters:
        first = cluster.task_records[0]
        checks = list(dict.fromkeys(check for record in cluster.task_records for check in record.checks))
        primary_strategy = resolve_primary_strategy(checks, strategy_cfg)
        strategy_counts[primary_strategy] = strategy_counts.get(primary_strategy, 0) + 1
        if first.summary.compiler_errors and checks and all(
            str(check).startswith("clang-diagnostic-") for check in checks
        ):
            recheck_first_candidates += 1
            recommended_action = RECOMMENDED_ACTION_RECHECK_FIRST
        else:
            recommended_action = None
        blocking_files.append(
            {
                "cluster_id": cluster.cluster_id,
                "task_ids": list(cluster.task_ids),
                "task_log": str(cluster.task_paths[0]),
                "source_file": cluster.source_file,
                "checks": checks,
                "check_fix_strategy": {
                    check: resolve_fix_strategy(check, strategy_cfg) for check in checks
                },
                "primary_fix_strategy": primary_strategy,
                "recommended_action": recommended_action,
            }
        )
    return blocking_files, strategy_counts, recheck_first_candidates


def _default_next_action(
    app_name: str,
    pending_count: int,
    blocking_files: list[dict],
    stage: str,
    status: str,
    verify_mode: str,
    build_dir_name: str,
    source_scope: str | None,
    config_file: str | None,
    strict_config: bool,
) -> str:
    tidy_args = ""
    if build_dir_name and build_dir_name != DEFAULT_TIDY_BUILD_DIR_NAME:
        tidy_args += f" --tidy-build-dir {build_dir_name}"
    if source_scope:
        tidy_args += f" --source-scope {source_scope}"
    config_args = clang_tidy_config.build_cli_args(
        config_file=config_file,
        strict_config=strict_config,
    )
    if config_args:
        tidy_args += " " + " ".join(config_args)

    if pending_count == 0:
        if stage == "tidy-close" and status.startswith("completed"):
            return "No pending tasks. Final gate completed."
        return (
            "No pending tasks. Run final gate: "
            f"python tools/run.py tidy-close --app {app_name}{tidy_args} --keep-going --concise"
        )

    first = blocking_files[0] if blocking_files else {}
    task_log = first.get("task_log", "<TASK_LOG>")
    task_ids = ",".join(first.get("task_ids", [])) or "<TASK_ID>"
    if first.get("recommended_action") == RECOMMENDED_ACTION_RECHECK_FIRST:
        return (
            f"Re-check current source cluster {first.get('cluster_id', '<CLUSTER_ID>')} "
            f"(tasks={task_ids}) first: python tools/run.py tidy-source-step "
            f"--task-log {task_log} --dry-run{tidy_args}"
        )
    strategy = first.get("primary_fix_strategy", STRATEGY_MANUAL_ONLY)
    return (
        f"Process current source cluster {first.get('cluster_id', '<CLUSTER_ID>')} "
        f"(tasks={task_ids}, strategy={strategy}): python tools/run.py "
        f"tidy-source-step --task-log {task_log}{tidy_args}"
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
    queue_requires_reresolve: bool = False,
    config_file: str | None = None,
    strict_config: bool = False,
    final_gate: dict | None = None,
) -> Path:
    tidy_layout = ctx.get_tidy_layout(app_name, build_dir_name)
    pending_logs = _collect_task_logs(tidy_layout.tasks_dir)
    archived_logs = _collect_task_logs(tidy_layout.archive_dir)
    blocking_files, strategy_counts, recheck_first_candidates = _build_blocking_files(ctx, pending_logs)
    queue_head = _build_queue_head(pending_logs)
    if next_action is None:
        next_action = _default_next_action(
            app_name=app_name,
            pending_count=len(pending_logs),
            blocking_files=blocking_files,
            stage=stage,
            status=status,
            verify_mode=verify_mode,
            build_dir_name=build_dir_name,
            source_scope=source_scope,
            config_file=config_file,
            strict_config=strict_config,
        )

    payload = {
        "generated_at": datetime.now(UTC).isoformat(),
        "app": app_name,
        "stage": stage,
        "status": status,
        "exit_code": int(exit_code),
        "verify_mode": verify_mode,
        "tasks": {
            "total": len(pending_logs) + len(archived_logs),
            "remaining": len(pending_logs),
            "archived": len(archived_logs),
        },
        "clusters": {
            "remaining": len(blocking_files),
            "queue_empty": not pending_logs,
        },
        "queue_requires_reresolve": bool(queue_requires_reresolve),
        "queue_head": queue_head,
        "blocking_summary": strategy_counts,
        "recheck_first_candidates": recheck_first_candidates,
        "blocking_files": blocking_files[:20],
        "fix_strategy_rules": {
            "auto_fix": ctx.config.tidy.fix_strategy.auto_fix,
            "safe_refactor": ctx.config.tidy.fix_strategy.safe_refactor,
            "nolint_allowed": ctx.config.tidy.fix_strategy.nolint_allowed,
            "manual_only": ctx.config.tidy.fix_strategy.manual_only,
        },
        "next_action": next_action,
        "final_gate": final_gate,
    }
    tidy_shared.write_json_dict(tidy_layout.tidy_result_path, payload)
    print(f"--- tidy-result: {tidy_layout.tidy_result_path}")
    return tidy_layout.tidy_result_path
