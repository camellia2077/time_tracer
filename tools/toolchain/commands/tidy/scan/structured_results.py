from __future__ import annotations

import json
import time
from pathlib import Path

from ....services import task_sorter
from ..queue import task_builder
from ..queue.task_model import build_task_draft_from_diagnostics


def collect_structured_results(
    ctx,
    results_dir: Path,
    tasks_dir: Path,
    *,
    task_view: str | None,
    workspace_name: str,
    source_scope: str | None,
) -> tuple[dict, float]:
    start = time.perf_counter()
    result_paths = sorted(results_dir.glob("check_*.json"))
    if not result_paths:
        raise FileNotFoundError(f"no structured clang-tidy results under {results_dir}")

    processed: list[dict] = []
    for result_path in result_paths:
        payload = _read_result(result_path)
        diagnostics = list(payload.get("diagnostics", []))
        if not diagnostics:
            continue
        source_file = str(payload.get("source_file", "")).strip()
        for diagnostic in diagnostics:
            if not diagnostic.get("file"):
                diagnostic["file"] = source_file
            diagnostic.setdefault("lines", [diagnostic.get("message", "")])
        raw_lines = [line for diagnostic in diagnostics for line in diagnostic.get("lines", [])]
        draft = build_task_draft_from_diagnostics(diagnostics, raw_lines=raw_lines)
        if draft is None:
            continue
        real_file = str(draft.source_file or source_file).strip()
        processed.append(
            {
                "draft": draft,
                "score": task_sorter.calculate_priority_score(diagnostics, real_file),
                "size": len("\n".join(raw_lines)),
                "diag": diagnostics,
                "file": real_file,
            }
        )

    processed.sort(key=lambda item: (item["score"], item["size"]))
    resolved_task_view = task_builder.resolve_task_view(None, tasks_dir=tasks_dir)
    if task_view:
        resolved_task_view = task_builder.resolve_task_view(task_view, tasks_dir=tasks_dir)
    cluster_count = task_builder.write_source_clusters(
        processed,
        tasks_dir,
        fix_strategy_config=ctx.config.tidy.fix_strategy,
        task_view=resolved_task_view,
        workspace_name=workspace_name,
        source_scope=source_scope,
    )
    elapsed = time.perf_counter() - start
    return (
        {
            "sections": len(result_paths),
            "workers": 1,
            "tasks": len(processed),
            "clusters": cluster_count,
            "max_lines": ctx.config.tidy.max_lines,
            "max_diags": ctx.config.tidy.max_diags,
            "task_view": resolved_task_view,
            "input": "structured",
        },
        elapsed,
    )


def _read_result(path: Path) -> dict:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"invalid structured clang-tidy result: {path}: {exc}") from exc
    if not isinstance(payload, dict):
        raise ValueError(f"structured clang-tidy result must be an object: {path}")
    if not isinstance(payload.get("diagnostics", []), list):
        raise ValueError(f"structured clang-tidy diagnostics must be a list: {path}")
    return payload
