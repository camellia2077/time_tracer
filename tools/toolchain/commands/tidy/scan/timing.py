from __future__ import annotations

import re
from pathlib import Path

NINJA_TIDY_TARGET_PATTERN = re.compile(r"^tidy_(check|fix)_step_(\d+)$")


def read_ninja_timing(ninja_log_path: Path) -> dict | None:
    if not ninja_log_path.exists():
        return None

    durations_ms_by_step: dict[str, int] = {}
    min_start_ms = None
    max_end_ms = None
    for raw_line in ninja_log_path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) < 4:
            continue
        try:
            start_ms = int(parts[0])
            end_ms = int(parts[1])
        except ValueError:
            continue
        target_name = parts[3].replace("\\", "/").split("/")[-1]
        match = NINJA_TIDY_TARGET_PATTERN.match(target_name)
        if not match:
            continue
        step_key = f"{match.group(1)}:{int(match.group(2)):04d}"
        duration_ms = max(0, end_ms - start_ms)
        durations_ms_by_step[step_key] = max(durations_ms_by_step.get(step_key, 0), duration_ms)
        min_start_ms = start_ms if min_start_ms is None else min(min_start_ms, start_ms)
        max_end_ms = end_ms if max_end_ms is None else max(max_end_ms, end_ms)

    if not durations_ms_by_step:
        return None
    durations = sorted(durations_ms_by_step.values())
    count = len(durations)
    p95_index = max(0, int(count * 0.95) - 1)
    slowest = sorted(durations_ms_by_step.items(), key=lambda item: item[1], reverse=True)[:5]
    return {
        "count": count,
        "wall_seconds": max(0, (max_end_ms or 0) - (min_start_ms or 0)) / 1000.0,
        "avg_seconds": sum(durations) / count / 1000.0,
        "p95_seconds": durations[p95_index] / 1000.0,
        "slowest": [
            {"step": step, "seconds": duration / 1000.0}
            for step, duration in slowest
        ],
    }


def format_seconds(seconds: float) -> str:
    if seconds < 60:
        return f"{seconds:.2f}s"
    minutes = int(seconds // 60)
    return f"{minutes}m{seconds - minutes * 60:.2f}s"


def print_timing_summary(
    did_auto_configure: bool,
    configure_seconds: float,
    build_seconds: float,
    parse_seconds: float,
    total_seconds: float,
    split_stats: dict | None,
    ninja_stats: dict | None,
    jobs: int | None,
) -> None:
    jobs_label = str(jobs) if jobs and jobs > 0 else "auto"
    print("--- Tidy timing summary ---")
    if did_auto_configure:
        print(f"auto-configure: {format_seconds(configure_seconds)}")
    print(f"build (jobs={jobs_label}): {format_seconds(build_seconds)}")
    if split_stats:
        input_label = "structured results"
        print(
            f"{input_label}: {format_seconds(parse_seconds)} "
            f"(sections={split_stats['sections']}, workers={split_stats['workers']}, "
            f"max_lines={split_stats['max_lines']}, max_diags={split_stats['max_diags']}, "
            f"tasks={split_stats['tasks']}, clusters={split_stats['clusters']})"
        )
    else:
        print("task input: unavailable")
    print(f"total: {format_seconds(total_seconds)}")
    if not ninja_stats:
        return
    print(
        "ninja tidy steps: "
        f"count={ninja_stats['count']}, wall={format_seconds(ninja_stats['wall_seconds'])}, "
        f"avg={format_seconds(ninja_stats['avg_seconds'])}, "
        f"p95={format_seconds(ninja_stats['p95_seconds'])}"
    )
    if ninja_stats["slowest"]:
        slowest = ", ".join(
            f"{item['step']}={format_seconds(item['seconds'])}"
            for item in ninja_stats["slowest"]
        )
        print(f"slowest steps: {slowest}")
