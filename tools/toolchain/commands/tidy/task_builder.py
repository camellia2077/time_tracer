import os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from ...core.config import TidyFixStrategyConfig
from ...core.context import Context
from ...services import log_parser, task_sorter
from ..shared import tidy as tidy_shared
from .fix_strategy import resolve_primary_strategy
from .task_model import (
    TaskDraft,
    build_task_draft_from_diagnostics,
    finalize_task_record,
    render_text,
    render_toon,
    task_record_to_dict,
)


def split_and_sort(
    ctx: Context,
    log_content: str,
    tasks_dir: Path,
    parse_workers: int | None = None,
    max_lines: int | None = None,
    max_diags: int | None = None,
    batch_size: int | None = None,
    task_view: str = "text",
    workspace_name: str = "",
    source_scope: str | None = None,
) -> dict:
    tasks_dir.mkdir(parents=True, exist_ok=True)
    log_lines = log_content.splitlines()
    ninja_sections = group_ninja_sections(log_lines)
    (
        effective_max_lines,
        effective_max_diags,
        effective_batch_size,
    ) = resolve_split_limits(ctx, max_lines, max_diags, batch_size)
    workers = resolve_parse_workers(ctx, parse_workers)

    processed = collect_section_tasks(
        ninja_sections,
        effective_max_lines,
        effective_max_diags,
        workers,
    )
    processed.sort(key=lambda x: (x["score"], x["size"]))

    total_batches = write_task_batches(
        processed,
        tasks_dir,
        effective_batch_size,
        fix_strategy_config=ctx.config.tidy.fix_strategy,
        task_view=task_view,
        workspace_name=workspace_name,
        source_scope=source_scope,
    )
    print(
        f"--- Created {len(processed)} granular tasks in {tasks_dir} "
        f"(batches={total_batches}, batch_size={effective_batch_size})"
    )
    return {
        "sections": len(ninja_sections),
        "workers": workers,
        "tasks": len(processed),
        "batches": total_batches,
        "batch_size": effective_batch_size,
        "max_lines": effective_max_lines,
        "max_diags": effective_max_diags,
        "task_view": task_view,
    }


def group_ninja_sections(log_lines: list[str]) -> list[list[str]]:
    ninja_sections = []
    curr_section = []
    for line in log_lines:
        if log_parser.TASK_START_PATTERN.match(line.strip()):
            if curr_section:
                ninja_sections.append(curr_section)
            curr_section = [line]
        else:
            curr_section.append(line)
    if curr_section:
        ninja_sections.append(curr_section)
    return ninja_sections


def resolve_split_limits(
    ctx: Context,
    max_lines: int | None,
    max_diags: int | None,
    batch_size: int | None,
) -> tuple[int, int, int]:
    effective_max_lines = ctx.config.tidy.max_lines if max_lines is None else max_lines
    effective_max_diags = ctx.config.tidy.max_diags if max_diags is None else max_diags
    effective_batch_size = ctx.config.tidy.batch_size if batch_size is None else batch_size
    if effective_max_lines <= 0:
        raise ValueError("max_lines must be > 0")
    if effective_max_diags <= 0:
        raise ValueError("max_diags must be > 0")
    if effective_batch_size <= 0:
        raise ValueError("batch_size must be > 0")
    return effective_max_lines, effective_max_diags, effective_batch_size


def collect_section_tasks(
    ninja_sections: list[list[str]],
    max_lines: int,
    max_diags: int,
    workers: int,
) -> list[dict]:
    processed: list[dict] = []
    if workers > 1 and len(ninja_sections) > 1:
        with ThreadPoolExecutor(max_workers=workers) as executor:
            for section_tasks in executor.map(
                process_ninja_section,
                ninja_sections,
                [max_lines] * len(ninja_sections),
                [max_diags] * len(ninja_sections),
            ):
                processed.extend(section_tasks)
        return processed

    for section in ninja_sections:
        processed.extend(process_ninja_section(section, max_lines, max_diags))
    return processed


def write_task_batches(
    processed: list[dict],
    tasks_dir: Path,
    batch_size: int,
    fix_strategy_config: TidyFixStrategyConfig,
    task_view: str,
    workspace_name: str,
    source_scope: str | None,
) -> int:
    cleanup_old_tasks(tasks_dir)
    selected_views = _resolve_task_views(task_view)
    write_json = _should_write_json(task_view)
    for idx, task in enumerate(processed, 1):
        batch_index = ((idx - 1) // batch_size) + 1
        task_id = f"{idx:03d}"
        batch_name = f"batch_{batch_index:03d}"
        batch_dir = tasks_dir / f"batch_{batch_index:03d}"
        batch_dir.mkdir(parents=True, exist_ok=True)
        record = finalize_task_record(
            task["draft"],
            task_id=task_id,
            batch_id=batch_name,
            workspace=workspace_name,
            source_scope=source_scope,
        )
        base_path = batch_dir / f"task_{task_id}"
        if write_json:
            tidy_shared.write_json_dict(
                base_path.with_suffix(".json"),
                task_record_to_dict(record),
            )
        if "text" in selected_views:
            base_path.with_suffix(".log").write_text(render_text(record), encoding="utf-8")
        if "toon" in selected_views:
            base_path.with_suffix(".toon").write_text(render_toon(record), encoding="utf-8")

    write_markdown_summary(
        processed,
        tasks_dir / "tasks_summary.md",
        fix_strategy_config=fix_strategy_config,
    )
    if not processed:
        return 0
    return ((len(processed) - 1) // batch_size) + 1


def cleanup_old_tasks(tasks_dir: Path) -> None:
    if not tasks_dir.exists():
        return

    for old_task in tasks_dir.rglob("task_*.*"):
        old_task.unlink()

    batch_dirs = [path for path in tasks_dir.glob("batch_*") if path.is_dir()]
    batch_dirs.sort(key=lambda path: path.name, reverse=True)
    for batch_dir in batch_dirs:
        if any(batch_dir.iterdir()):
            continue
        batch_dir.rmdir()


def resolve_parse_workers(ctx: Context, cli_value: int | None) -> int:
    if cli_value is not None and cli_value > 0:
        return cli_value

    configured_workers = ctx.config.tidy.parse_workers
    if configured_workers > 0:
        return configured_workers

    cpu_count = os.cpu_count() or 1
    return min(8, max(1, cpu_count))


def process_ninja_section(
    section_lines: list[str],
    max_lines: int,
    max_diags: int,
) -> list[dict]:
    diags = log_parser.extract_diagnostics(section_lines)
    if not diags:
        return []

    processed: list[dict] = []
    current_batch: list[dict] = []
    batch_lines = 0

    def finalize_batch(batch: list[dict]) -> None:
        if not batch:
            return

        real_file = batch[0]["file"]
        p_score = task_sorter.calculate_priority_score(batch, real_file)
        raw_lines: list[str] = []
        for diag in batch:
            raw_lines.extend(diag["lines"])
        draft = build_task_draft_from_diagnostics(batch, raw_lines=raw_lines)
        if draft is None:
            return

        processed.append(
            {
                "draft": draft,
                "score": p_score,
                "size": len("\n".join(raw_lines)),
                "diag": [diagnostic.to_log_parser_dict() for diagnostic in draft.diagnostics],
                "file": real_file,
            }
        )

    for diag in diags:
        diag_len = len(diag["lines"])
        if current_batch and (
            len(current_batch) >= max_diags or batch_lines + diag_len > max_lines
        ):
            finalize_batch(current_batch)
            current_batch = []
            batch_lines = 0

        current_batch.append(diag)
        batch_lines += diag_len

    finalize_batch(current_batch)
    return processed


def write_markdown_summary(
    processed: list,
    out_path: Path,
    fix_strategy_config: TidyFixStrategyConfig,
) -> None:
    lines = [
        "# Clang-Tidy Tasks Summary\n",
        "| ID | File | Difficulty Score | Warning Types | Fix Strategy |",
        "| --- | --- | --- | --- | --- |",
    ]
    for idx, item in enumerate(processed, 1):
        draft: TaskDraft = item["draft"]
        checks = list(draft.checks)
        w_types = ", ".join(checks)
        strategy = resolve_primary_strategy(checks, fix_strategy_config)
        lines.append(
            f"| {idx:03d} | {item['file']} | {item['score']:.2f} | {w_types} | {strategy} |"
        )
    out_path.write_text("\n".join(lines), encoding="utf-8")


def _resolve_task_views(task_view: str) -> tuple[str, ...]:
    normalized = (task_view or "").strip().lower()
    if normalized == "json":
        return ()
    if normalized == "toon":
        return ("toon",)
    if normalized == "text+toon":
        return ("text", "toon")
    return ("text",)


def _should_write_json(task_view: str) -> bool:
    normalized = (task_view or "").strip().lower()
    return normalized in {"json", "text+toon"}
