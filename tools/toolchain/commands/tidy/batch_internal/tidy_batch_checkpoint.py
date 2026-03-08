from __future__ import annotations

from ....services import batch_state


def load_checkpoint(
    *,
    ctx,
    app_name: str,
    batch_id: str,
    stage_order: tuple[str, ...],
    tidy_build_dir_name: str,
) -> dict | None:
    state_path = batch_state.state_path(ctx, app_name, tidy_build_dir_name)
    state = batch_state.load_state(state_path, app_name)
    checkpoint = state.get("tidy_batch_checkpoint")
    if not isinstance(checkpoint, dict):
        return None
    if checkpoint.get("batch_id") != batch_id:
        return None
    if checkpoint.get("next_stage") not in stage_order:
        return None
    return checkpoint


def save_checkpoint(
    *,
    ctx,
    app_name: str,
    tidy_build_dir_name: str,
    batch_id: str,
    next_stage: str,
    task_ids: list[str],
    verify_success: bool | None,
) -> None:
    checkpoint = {
        "batch_id": batch_id,
        "next_stage": next_stage,
        "task_ids": task_ids,
    }
    batch_state.update_state(
        ctx=ctx,
        app_name=app_name,
        tidy_build_dir_name=tidy_build_dir_name,
        batch_id=batch_id,
        cleaned_task_ids=[],
        last_verify_success=verify_success,
        extra_fields={"tidy_batch_checkpoint": checkpoint},
    )
