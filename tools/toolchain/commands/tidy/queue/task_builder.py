import re
import shutil
from pathlib import Path

from ....core.config import TidyFixStrategyConfig
from ...shared import tidy as tidy_shared
from ..fix_strategy import resolve_primary_strategy
from .task_queue import next_queue_generation, write_queue_state
from .source_cluster import cluster_id_for_source
from .task_model import (
    TaskDraft,
    finalize_task_record,
    render_text,
    render_toon,
    task_record_to_dict,
)

DEFAULT_TASK_VIEW = "toon"
_SUPPORTED_TASK_VIEWS = {"json", "text", "toon", "text+toon"}
_TASK_ARTIFACT_PATTERN = re.compile(r"^task_(\d+)\.(?:json|log|toon)$")
def write_source_clusters(
    processed: list[dict],
    tasks_dir: Path,
    fix_strategy_config: TidyFixStrategyConfig,
    task_view: str | None,
    workspace_name: str,
    source_scope: str | None,
) -> int:
    resolved_task_view = resolve_task_view(task_view, tasks_dir=tasks_dir)
    cleanup_old_tasks(tasks_dir)
    clusters_dir = tasks_dir / "clusters"
    clusters_dir.mkdir(parents=True, exist_ok=True)
    queue_generation = next_queue_generation(tasks_dir)
    scan_id = f"scan_{queue_generation:06d}"
    selected_views = _resolve_task_views(resolved_task_view)
    grouped: dict[str, tuple[str, list[dict]]] = {}
    for task in processed:
        source_file = str(task["draft"].source_file or task["file"] or "").strip()
        key = source_file.lower().replace("\\", "/")
        if key not in grouped:
            grouped[key] = (source_file, [])
        grouped[key][1].append(task)

    for source_file, source_tasks in grouped.values():
        cluster_name = cluster_id_for_source(source_file)
        cluster_dir = clusters_dir / cluster_name
        cluster_dir.mkdir(parents=True, exist_ok=True)
        tidy_shared.write_json_dict(
            cluster_dir / "cluster.json",
            {
                "version": 1,
                "cluster_id": cluster_name,
                "scan_id": scan_id,
                "source_file": source_file,
                "task_count": len(source_tasks),
                "status": "pending",
            },
        )
        for task_number, task in enumerate(source_tasks, 1):
            task_id = f"{task_number:03d}"
            record = finalize_task_record(
                task["draft"],
                task_id=task_id,
                cluster_id=cluster_name,
                scan_id=scan_id,
                queue_generation=queue_generation,
                workspace=workspace_name,
                source_scope=source_scope,
            )
            base_path = cluster_dir / f"task_{task_id}"
            tidy_shared.write_json_dict(base_path.with_suffix(".json"), task_record_to_dict(record))
            if "text" in selected_views:
                base_path.with_suffix(".log").write_text(render_text(record), encoding="utf-8")
            if "toon" in selected_views:
                base_path.with_suffix(".toon").write_text(render_toon(record), encoding="utf-8")

    write_markdown_summary(
        processed,
        tasks_dir / "tasks_summary.md",
        fix_strategy_config=fix_strategy_config,
        start_task_number=1,
    )
    # Queue generation is persisted separately from task records so task-local
    # commands can reject historical selections after a refresh/full tidy
    # rebuild rewrites the tasks/ tree.
    write_queue_state(
        tasks_dir,
        queue_generation=queue_generation,
        task_count=len(processed),
        cluster_count=len(grouped),
        scan_id=scan_id,
        task_view=resolved_task_view,
    )
    tidy_shared.write_json_dict(
        tasks_dir / "scan_manifest.json",
        {
            "version": 1,
            "scan_id": scan_id,
            "queue_generation": queue_generation,
            "workspace": workspace_name,
            "source_scope": source_scope,
            "task_count": len(processed),
            "cluster_count": len(grouped),
        },
    )
    if not processed:
        return 0
    return len(grouped)


def cleanup_old_tasks(tasks_dir: Path) -> None:
    if not tasks_dir.exists():
        return

    for child in list(tasks_dir.iterdir()):
        if child.name in {"archive", "queue_state.json", "scan_manifest.json"}:
            continue
        if child.is_dir():
            shutil.rmtree(child)
        elif child.name.startswith("task_") or child.name in {"tasks_summary.md"}:
            child.unlink()


def resolve_task_view(
    task_view: str | None,
    *,
    tasks_dir: Path | None = None,
    default: str = DEFAULT_TASK_VIEW,
) -> str:
    normalized = normalize_task_view(task_view)
    if normalized is not None:
        return normalized
    inferred = infer_task_view_from_existing_tasks(tasks_dir)
    if inferred is not None:
        return inferred
    return default


def normalize_task_view(task_view: str | None) -> str | None:
    normalized = (task_view or "").strip().lower()
    if not normalized:
        return None
    if normalized not in _SUPPORTED_TASK_VIEWS:
        raise ValueError(
            f"unsupported task_view={task_view!r}; expected one of "
            f"{', '.join(sorted(_SUPPORTED_TASK_VIEWS))}"
        )
    return normalized


def infer_task_view_from_existing_tasks(tasks_dir: Path | None) -> str | None:
    if tasks_dir is None or not tasks_dir.exists():
        return None

    has_json = False
    has_text = False
    has_toon = False
    current_tasks_dir = tasks_dir / "clusters"
    if not current_tasks_dir.exists():
        return None
    for task_path in current_tasks_dir.rglob("task_*.*"):
        if _TASK_ARTIFACT_PATTERN.match(task_path.name) is None:
            continue
        suffix = task_path.suffix.lower()
        if suffix == ".json":
            has_json = True
        elif suffix == ".log":
            has_text = True
        elif suffix == ".toon":
            has_toon = True
        if has_text and has_toon:
            return "text+toon"

    if has_toon:
        return "toon"
    if has_text:
        return "text"
    if has_json:
        return "json"
    return None


def write_markdown_summary(
    processed: list,
    out_path: Path,
    fix_strategy_config: TidyFixStrategyConfig,
    start_task_number: int = 1,
) -> None:
    lines = [
        "# Clang-Tidy Tasks Summary\n",
        "| ID | File | Difficulty Score | Warning Types | Fix Strategy |",
        "| --- | --- | --- | --- | --- |",
    ]
    for idx, item in enumerate(processed, start_task_number):
        draft: TaskDraft = item["draft"]
        checks = list(draft.checks)
        w_types = ", ".join(checks)
        strategy = resolve_primary_strategy(checks, fix_strategy_config)
        lines.append(
            f"| {idx:03d} | {item['file']} | {item['score']:.2f} | {w_types} | {strategy} |"
        )
    out_path.write_text("\n".join(lines), encoding="utf-8")


def _resolve_task_views(task_view: str) -> tuple[str, ...]:
    normalized = normalize_task_view(task_view) or DEFAULT_TASK_VIEW
    if normalized == "json":
        return ()
    if normalized == "toon":
        return ("toon",)
    if normalized == "text+toon":
        return ("text", "toon")
    return ("text",)
