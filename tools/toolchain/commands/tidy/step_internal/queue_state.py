from __future__ import annotations

from pathlib import Path

from ....core.context import Context
from ....services import batch_state
from .. import tidy_result as tidy_result_summary
from ...shared import tidy as tidy_shared
from ..tasking.task_log import list_task_paths, load_task_record


def sync_queue_snapshot_after_close(
    ctx: Context,
    *,
    app_name: str,
    tidy_build_dir_name: str,
    source_scope: str | None,
    closed_batch_id: str,
    closed_task_id: str,
    closed_task_path: Path,
    recheck_log_path: Path,
    queue_head_after_close: dict | None,
    config_file: str | None,
    strict_config: bool,
) -> None:
    transition_summary = build_close_transition_summary(
        batch_id=closed_batch_id,
        task_id=closed_task_id,
        queue_head=queue_head_after_close,
    )
    result_path = tidy_result_summary.write_tidy_result(
        ctx=ctx,
        app_name=app_name,
        stage="tidy-step",
        status="task_archived",
        exit_code=0,
        build_dir_name=tidy_build_dir_name,
        source_scope=source_scope,
        verify_mode="full",
        config_file=config_file,
        strict_config=strict_config,
    )
    result_payload = tidy_shared.read_json_dict(result_path) or {}
    next_action = str(result_payload.get("next_action") or "").strip() or None
    active_batch_id = str((queue_head_after_close or {}).get("batch_id") or "").strip() or None
    state_path = batch_state.update_state(
        ctx=ctx,
        app_name=app_name,
        tidy_build_dir_name=tidy_build_dir_name,
        batch_id=active_batch_id,
        cleaned_task_ids=[closed_task_id],
        last_verify_success=True,
        extra_fields={
            "batch_id": active_batch_id,
            "queue_batch_id": active_batch_id,
            "last_tidy_step_ok": True,
            "last_tidy_step_task": {
                "batch_id": closed_batch_id,
                "task_id": closed_task_id,
                "task_log": str(closed_task_path),
                "recheck_log": str(recheck_log_path),
            },
            "queue_requires_reresolve": False,
            "historical_selection_stale": False,
            "reparse_required_reason": None,
            "next_queue_head": queue_head_after_close,
            "replacement_queue_head": None,
            "queue_transition_summary": transition_summary,
            "historical_batch": None,
            "next_action": next_action,
        },
    )
    print(f"--- tidy-step: queue snapshot updated -> {result_path}")
    print(f"--- tidy-step: batch state updated -> {state_path}")


def build_queue_head(tasks_dir: Path) -> dict | None:
    pending_logs = list_task_paths(tasks_dir)
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


def build_single_task_close_next_action(
    *,
    batch_id: str,
    task_id: str,
    queue_head: dict | None,
) -> str:
    next_action = (
        f"Single-task batch {batch_id}/task_{task_id} is closed. Stop here and "
        "re-resolve the current queue from tasks/ before continuing; do not keep using "
        f"{batch_id}/task_{task_id} as the active selection."
    )
    if queue_head is not None:
        next_action += (
            f" Current queue head: {queue_head.get('batch_id', '<BATCH_ID>')}/"
            f"task_{queue_head.get('task_id', '<TASK_ID>')} -> "
            f"{queue_head.get('task_log', '')}"
        )
    else:
        next_action += " No pending tasks remain in tasks/."
    return next_action


def build_close_transition_summary(
    *,
    batch_id: str,
    task_id: str,
    queue_head: dict | None,
) -> str:
    summary = f"Closed {batch_id}/task_{task_id}."
    if queue_head is not None:
        summary += (
            f" Current queue head is {queue_head.get('batch_id', '<BATCH_ID>')}/"
            f"task_{queue_head.get('task_id', '<TASK_ID>')} -> "
            f"{queue_head.get('task_log', '')}."
        )
    else:
        summary += " No pending tasks remain."
    return summary
