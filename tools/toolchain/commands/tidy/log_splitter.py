import re
import time
from collections.abc import Callable
from pathlib import Path

NINJA_TIDY_TARGET_PATTERN = re.compile(r"^tidy_(check|fix)_step_(\d+)$")


def split_from_log(
    log_path: Path,
    tasks_dir: Path,
    split_and_sort_fn: Callable[..., dict],
    parse_workers: int | None = None,
    max_lines: int | None = None,
    max_diags: int | None = None,
    batch_size: int | None = None,
) -> tuple[dict, float]:
    parse_start = time.perf_counter()
    log_content = log_path.read_text(encoding="utf-8", errors="replace")
    split_stats = split_and_sort_fn(
        log_content,
        tasks_dir,
        parse_workers=parse_workers,
        max_lines=max_lines,
        max_diags=max_diags,
        batch_size=batch_size,
    )
    parse_seconds = time.perf_counter() - parse_start
    return split_stats, parse_seconds


def read_ninja_timing(ninja_log_path: Path) -> dict | None:
    if not ninja_log_path.exists():
        return None

    durations_ms_by_step: dict[str, int] = {}
    min_start_ms = None
    max_end_ms = None

    with ninja_log_path.open("r", encoding="utf-8", errors="replace") as f:
        for raw_line in f:
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

            normalized_target = parts[3].replace("\\", "/")
            target_name = normalized_target.split("/")[-1]
            match = NINJA_TIDY_TARGET_PATTERN.match(target_name)
            if not match:
                continue

            phase = match.group(1)
            step_number = int(match.group(2))
            step_key = f"{phase}:{step_number:04d}"
            duration_ms = max(0, end_ms - start_ms)
            previous_duration = durations_ms_by_step.get(step_key)
            if previous_duration is None or duration_ms > previous_duration:
                durations_ms_by_step[step_key] = duration_ms

            if min_start_ms is None or start_ms < min_start_ms:
                min_start_ms = start_ms
            if max_end_ms is None or end_ms > max_end_ms:
                max_end_ms = end_ms

    if not durations_ms_by_step:
        return None

    all_durations_ms = sorted(durations_ms_by_step.values())
    count = len(all_durations_ms)
    total_duration_ms = sum(all_durations_ms)
    average_duration_ms = total_duration_ms / count
    p95_index = max(0, int(count * 0.95) - 1)
    p95_duration_ms = all_durations_ms[p95_index]
    wall_ms = max(0, (max_end_ms or 0) - (min_start_ms or 0))
    slowest = sorted(
        durations_ms_by_step.items(),
        key=lambda item: item[1],
        reverse=True,
    )[:5]

    return {
        "count": count,
        "wall_seconds": wall_ms / 1000.0,
        "avg_seconds": average_duration_ms / 1000.0,
        "p95_seconds": p95_duration_ms / 1000.0,
        "slowest": [
            {"step": step_key, "seconds": duration_ms / 1000.0} for step_key, duration_ms in slowest
        ],
    }


def format_seconds(seconds: float) -> str:
    if seconds < 60:
        return f"{seconds:.2f}s"
    minutes = int(seconds // 60)
    remain_seconds = seconds - minutes * 60
    return f"{minutes}m{remain_seconds:.2f}s"


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
        print(
            "log split: "
            f"{format_seconds(parse_seconds)} "
            f"(sections={split_stats['sections']}, workers={split_stats['workers']}, "
            f"max_lines={split_stats['max_lines']}, "
            f"max_diags={split_stats['max_diags']}, "
            f"tasks={split_stats['tasks']}, batches={split_stats['batches']}, "
            f"batch_size={split_stats['batch_size']})"
        )
    else:
        print("log split: skipped (build.log not found)")
    print(f"total: {format_seconds(total_seconds)}")

    if not ninja_stats:
        return

    print(
        "ninja tidy steps: "
        f"count={ninja_stats['count']}, "
        f"wall={format_seconds(ninja_stats['wall_seconds'])}, "
        f"avg={format_seconds(ninja_stats['avg_seconds'])}, "
        f"p95={format_seconds(ninja_stats['p95_seconds'])}"
    )
    if ninja_stats["slowest"]:
        slowest_text = ", ".join(
            f"{item['step']}={format_seconds(item['seconds'])}" for item in ninja_stats["slowest"]
        )
        print(f"slowest steps: {slowest_text}")
