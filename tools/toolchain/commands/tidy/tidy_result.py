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
from . import clang_tidy_config
from .task_log import list_task_paths, load_task_record
from .workspace import DEFAULT_TIDY_BUILD_DIR_NAME


RECOMMENDED_ACTION_RECHECK_FIRST = "recheck_first"


def _collect_task_logs(root: Path) -> list[Path]:
    if not root.exists():
        return []
    return list_task_paths(root)


def _build_queue_head(pending_logs: list[Path]) -> dict | None:
    if not pending_logs:
        return None
    head_path = pending_logs[0]
    parsed = load_task_record(head_path)
    return {
        "task_id": parsed.task_id,
        "batch_id": parsed.batch_id,
        "queue_batch_id": parsed.batch_id,
        "source_file": parsed.source_file or str(head_path),
        "task_log": str(head_path),
        "checks": list(parsed.checks),
    }


def _format_task_ref(batch_id: str, task_id: str) -> str:
    return f"{batch_id}/task_{task_id}"


def _format_task_ref_list(task_refs: list[str], fallback_batch_id: str) -> str:
    if task_refs:
        if len(task_refs) <= 3:
            return ", ".join(task_refs)
        preview = ", ".join(task_refs[:3])
        return f"{preview} (+{len(task_refs) - 3} more)"
    return fallback_batch_id


def _queue_head_label(queue_head: dict | None) -> str:
    if queue_head is None:
        return "unknown"
    return f"{queue_head.get('batch_id', '<BATCH_ID>')}/task_{queue_head.get('task_id', '<TASK_ID>')}"


def _queue_head_suffix(queue_head: dict | None) -> str:
    if queue_head is None:
        return ""
    queue_head_path = str(queue_head.get("task_log") or "").strip()
    if not queue_head_path:
        return ""
    return f" -> {queue_head_path}"


def _build_transition_summary(
    *,
    historical_batch_id: str,
    cleaned_task_refs: list[str],
    queue_head: dict | None,
    queue_identity_changed_after_refresh: bool,
    replacement_kind: str,
) -> str:
    selection_label = _format_task_ref_list(cleaned_task_refs, historical_batch_id)
    queue_head_label = _queue_head_label(queue_head)
    queue_head_suffix = _queue_head_suffix(queue_head)
    if queue_identity_changed_after_refresh:
        if queue_head is not None and replacement_kind == "new_batch_head":
            return (
                f"Refresh replaced closed selection {selection_label} with new queue head "
                f"{queue_head_label}{queue_head_suffix}."
            )
        if queue_head is not None and replacement_kind == "new_task_in_same_batch":
            return (
                f"Refresh replaced closed selection {selection_label} with new task "
                f"{queue_head_label}{queue_head_suffix} in the same batch."
            )
        return f"Refresh cleared the pending queue after closing {selection_label}."
    if queue_head is not None:
        return (
            f"Refresh completed after closing {selection_label}. Current queue head is "
            f"{queue_head_label}{queue_head_suffix}."
        )
    return f"Refresh completed after closing {selection_label}. No pending tasks remain."


def build_historical_batch_summary(
    tasks_dir: Path,
    *,
    historical_batch_id: str | None,
    historical_task_ids: list[str] | None,
    queue_head: dict | None,
) -> dict | None:
    normalized_batch = str(historical_batch_id or "").strip()
    if not normalized_batch:
        return None
    historical_pending = list_task_paths(tasks_dir, batch_id=normalized_batch)
    known_task_ids = [str(task_id).strip() for task_id in (historical_task_ids or []) if str(task_id).strip()]
    known_task_id_set = set(known_task_ids)
    cleaned_task_refs = [_format_task_ref(normalized_batch, task_id) for task_id in known_task_ids]
    queue_identity_changed_after_refresh = False
    replacement_kind = "unchanged"
    replacement_queue_head = None
    if queue_head is not None:
        queue_head_batch = str(queue_head.get("batch_id") or "").strip()
        queue_head_task = str(queue_head.get("task_id") or "").strip()
        if queue_head_batch != normalized_batch:
            queue_identity_changed_after_refresh = True
            replacement_kind = "new_batch_head"
            replacement_queue_head = dict(queue_head)
        elif known_task_id_set and queue_head_task not in known_task_id_set:
            queue_identity_changed_after_refresh = True
            replacement_kind = "new_task_in_same_batch"
            replacement_queue_head = dict(queue_head)
    elif not historical_pending and known_task_id_set:
        queue_identity_changed_after_refresh = True
        replacement_kind = "queue_cleared"
    transition_summary = _build_transition_summary(
        historical_batch_id=normalized_batch,
        cleaned_task_refs=cleaned_task_refs,
        queue_head=queue_head,
        queue_identity_changed_after_refresh=queue_identity_changed_after_refresh,
        replacement_kind=replacement_kind,
    )
    return {
        "batch_id": normalized_batch,
        "remaining_tasks": len(historical_pending),
        "still_pending": bool(historical_pending),
        "cleaned_task_ids": known_task_ids,
        "cleaned_task_refs": cleaned_task_refs,
        "historical_identity_stale": True,
        "refresh_requires_new_selection": True,
        "queue_identity_changed_after_refresh": queue_identity_changed_after_refresh,
        "replacement_kind": replacement_kind,
        "replacement_queue_head": replacement_queue_head,
        "transition_summary": transition_summary,
    }


def build_queue_reresolve_next_action(
    *,
    historical_batch: dict | None,
    queue_head: dict | None,
) -> str:
    historical_batch_id = str((historical_batch or {}).get("batch_id") or "").strip() or "<BATCH_ID>"
    transition_summary = str((historical_batch or {}).get("transition_summary") or "").strip()
    if not transition_summary:
        transition_summary = f"Refresh completed for {historical_batch_id}."
    next_action = (
        f"{transition_summary} The historical batch/task selection is now stale. "
        "You must re-resolve the current smallest pending task from tasks/ before continuing; "
        "do not reuse historical batch/task ids."
    )
    if queue_head is not None and not str(queue_head.get("task_log") or "").strip():
        next_action += f" Current queue head: {_queue_head_label(queue_head)}."
    return next_action


def _is_transient_compiler_diagnostic(checks: list[str], *, compiler_errors: bool) -> bool:
    if not compiler_errors or not checks:
        return False
    return all(str(check).strip().startswith("clang-diagnostic-") for check in checks)


def _build_blocking_files(ctx: Context, pending_logs: list[Path]) -> tuple[list[dict], dict, int]:
    blocking_files: list[dict] = []
    strategy_counts = {strategy: 0 for strategy in ALL_STRATEGIES}
    seen_files: set[str] = set()
    strategy_cfg = ctx.config.tidy.fix_strategy
    recheck_first_candidates = 0

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
        recommended_action = None
        if _is_transient_compiler_diagnostic(
            checks,
            compiler_errors=bool(parsed.summary.compiler_errors),
        ):
            recommended_action = RECOMMENDED_ACTION_RECHECK_FIRST
            recheck_first_candidates += 1
        blocking_files.append(
            {
                "task_id": parsed.task_id,
                "batch_id": parsed.batch_id,
                "queue_batch_id": parsed.batch_id,
                "task_log": str(log_path),
                "source_file": source_file,
                "checks": checks,
                "check_fix_strategy": check_strategies,
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
    queue_requires_reresolve: bool,
    queue_head: dict | None,
    historical_batch: dict | None,
    config_file: str | None,
    strict_config: bool,
) -> str:
    tidy_args = ""
    tidy_config_args = clang_tidy_config.build_cli_args(
        config_file=config_file,
        strict_config=strict_config,
    )
    tidy_config_suffix = ""
    if tidy_config_args:
        tidy_config_suffix = " " + " ".join(tidy_config_args)
    normalized_build_dir = (build_dir_name or "").strip()
    normalized_scope = (source_scope or "").strip()
    if normalized_build_dir and normalized_build_dir != DEFAULT_TIDY_BUILD_DIR_NAME:
        tidy_args += f" --tidy-build-dir {normalized_build_dir}"
    if normalized_scope:
        tidy_args += f" --source-scope {normalized_scope}"
    tidy_args += tidy_config_suffix

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

    if queue_requires_reresolve:
        return build_queue_reresolve_next_action(
            historical_batch=historical_batch,
            queue_head=queue_head,
        )

    first_block = blocking_files[0] if blocking_files else {}
    batch_id = first_block.get("batch_id") or "<BATCH_ID>"
    task_id = first_block.get("task_id") or "<TASK_ID>"
    task_log = first_block.get("task_log") or "<TASK_LOG>"
    primary_strategy = first_block.get("primary_fix_strategy", STRATEGY_MANUAL_ONLY)
    recommended_action = first_block.get("recommended_action")
    if recommended_action == RECOMMENDED_ACTION_RECHECK_FIRST:
        return (
            f"Re-check transient compiler task_{task_id} first: "
            f"python tools/run.py tidy-step --task-log {task_log}{tidy_config_suffix} --dry-run"
        )
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
    historical_batch_id: str | None = None,
    historical_task_ids: list[str] | None = None,
    queue_requires_reresolve: bool = False,
    config_file: str | None = None,
    strict_config: bool = False,
) -> Path:
    tidy_layout = ctx.get_tidy_layout(app_name, build_dir_name)
    tasks_dir = tidy_layout.tasks_dir
    tasks_done_dir = tidy_layout.tasks_done_dir
    result_path = tidy_layout.tidy_result_path

    pending_logs = _collect_task_logs(tasks_dir)
    archived_logs = _collect_task_logs(tasks_done_dir)
    blocking_files, strategy_counts, recheck_first_candidates = _build_blocking_files(
        ctx,
        pending_logs,
    )

    remaining_tasks = len(pending_logs)
    total_tasks = remaining_tasks + len(archived_logs)
    queue_head = _build_queue_head(pending_logs)
    historical_batch = build_historical_batch_summary(
        tasks_dir,
        historical_batch_id=historical_batch_id,
        historical_task_ids=historical_task_ids,
        queue_head=queue_head,
    )
    effective_queue_requires_reresolve = bool(queue_requires_reresolve and remaining_tasks > 0)
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
            queue_requires_reresolve=effective_queue_requires_reresolve,
            queue_head=queue_head,
            historical_batch=historical_batch,
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
            "total": total_tasks,
            "remaining": remaining_tasks,
            "archived": len(archived_logs),
        },
        "queue_requires_reresolve": effective_queue_requires_reresolve,
        "queue_head": queue_head,
        "historical_batch": historical_batch,
        "replacement_queue_head": (
            historical_batch.get("replacement_queue_head") if historical_batch else None
        ),
        "queue_transition_summary": (
            historical_batch.get("transition_summary") if historical_batch else None
        ),
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
    }
    tidy_shared.write_json_dict(result_path, payload)
    print(f"--- tidy-result: {result_path}")
    return result_path
