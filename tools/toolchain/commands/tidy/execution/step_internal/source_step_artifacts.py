from __future__ import annotations

from dataclasses import replace

from ...queue.task_log import task_view_paths
from ...queue.task_model import (
    build_task_draft_from_diagnostics,
    finalize_task_record,
    render_text,
    render_toon,
    task_record_to_dict,
)
from ....shared import tidy as tidy_shared


def refresh_cluster_artifacts(*, task_ctx, cluster, diagnostics: list[dict]) -> None:
    """Replace the current cluster snapshot after a focused re-check."""
    draft = build_task_draft_from_diagnostics(diagnostics)
    if draft is None:
        return
    draft = replace(draft, source_file=cluster.source_file)
    first_path = cluster.task_paths[0]
    first_record = cluster.task_records[0]
    refreshed = finalize_task_record(
        draft,
        task_id=first_record.task_id,
        cluster_id=first_record.cluster_id,
        scan_id=first_record.scan_id,
        queue_generation=task_ctx.current_queue_generation,
        workspace=task_ctx.tidy_build_dir_name,
        source_scope=task_ctx.source_scope,
    )
    existing_suffixes = {path.suffix.lower() for path in task_view_paths(first_path)}
    tidy_shared.write_json_dict(first_path, task_record_to_dict(refreshed))
    base_path = first_path.with_suffix("")
    if ".log" in existing_suffixes:
        base_path.with_suffix(".log").write_text(render_text(refreshed), encoding="utf-8")
    if ".toon" in existing_suffixes:
        base_path.with_suffix(".toon").write_text(render_toon(refreshed), encoding="utf-8")
    for extra_path in cluster.task_paths[1:]:
        for artifact in task_view_paths(extra_path):
            artifact.unlink()
