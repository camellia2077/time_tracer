from __future__ import annotations

from .....services import tidy_state
from ... import tidy_result as tidy_result_summary
from ....shared import tidy as tidy_shared
from .queue_state import build_queue_head


def write_source_step_state(
    *,
    ctx,
    task_ctx,
    cluster,
    status: str,
    exit_code: int,
    recheck,
    workspace,
    config_file: str | None,
    strict_config: bool,
    next_action: str,
) -> None:
    """Persist the common tidy result, state, and source-cluster checkpoint."""
    tidy_layout = ctx.get_tidy_layout(task_ctx.app_name, workspace.build_dir_name)
    queue_head = build_queue_head(task_ctx.tasks_dir)
    result_path = tidy_result_summary.write_tidy_result(
        ctx=ctx,
        app_name=task_ctx.app_name,
        stage="tidy-source-step",
        status=status,
        exit_code=exit_code,
        build_dir_name=workspace.build_dir_name,
        source_scope=workspace.source_scope,
        config_file=config_file,
        strict_config=strict_config,
    )
    tidy_state.update_state(
        ctx=ctx,
        app_name=task_ctx.app_name,
        tidy_build_dir_name=workspace.build_dir_name,
        cluster_id=queue_head.get("cluster_id") if queue_head else None,
        cleaned_task_ids=list(cluster.task_ids) if status == "resolved" else None,
        extra_fields={
            "last_tidy_source_cluster": {
                "source_file": cluster.source_file,
                "task_ids": list(cluster.task_ids),
                "cluster_id": cluster.cluster_id,
                "status": status,
                "recheck_log": str(recheck.log_path),
            },
            "next_queue_head": queue_head,
            "queue_requires_reresolve": True,
            "next_action": next_action,
        },
    )
    state_path = tidy_layout.automation_dir / "source_cluster_last.json"
    tidy_shared.write_json_dict(
        state_path,
        {
            "status": status,
            "exit_code": exit_code,
            "source_file": cluster.source_file,
            "task_ids": list(cluster.task_ids),
            "cluster_id": cluster.cluster_id,
            "recheck_log": str(recheck.log_path),
            "queue_head": queue_head,
            "next_action": next_action,
            "tidy_result": str(result_path),
        },
    )
    print(f"--- tidy-source-step: state -> {state_path}")
