import os
import re
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from ..core.context import Context
from ..core.executor import run_command
from ..services import log_parser, task_sorter

NINJA_TIDY_TARGET_PATTERN = re.compile(r"^tidy_(check|fix)_step_(\d+)$")

class TidyCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        extra_args: list = None,
        jobs: int | None = None,
        parse_workers: int | None = None,
        keep_going: bool | None = None,
    ):
        from .build import BuildCommand
        
        app_dir = self.ctx.get_app_dir(app_name)
        build_dir = app_dir / "build_tidy"
        log_path = build_dir / "build.log"
        tasks_dir = build_dir / "tasks"
        ninja_log_path = build_dir / ".ninja_log"
        overall_start = time.perf_counter()
        configure_seconds = 0.0
        build_seconds = 0.0
        parse_seconds = 0.0
        split_stats = None
        did_auto_configure = False
        
        # 1. Ensure build is configured
        if not (build_dir / "CMakeCache.txt").exists():
            print(f"--- Build directory {build_dir} not configured. Running auto-configure...")
            did_auto_configure = True
            configure_start = time.perf_counter()
            builder = BuildCommand(self.ctx)
            ret = builder.configure(app_name, tidy=True)
            configure_seconds = time.perf_counter() - configure_start
            if ret != 0:
                print("--- Auto-configure failed. Aborting Tidy.")
                return ret
        
        # 2. Run Tidy Build with Real-time Output
        filtered_args = [a for a in (extra_args or []) if a != "--"]
        has_target_override = "--target" in filtered_args
        configured_jobs = self.ctx.config.tidy.jobs
        configured_keep_going = self.ctx.config.tidy.keep_going
        effective_jobs = jobs if jobs is not None else configured_jobs
        effective_keep_going = (
            configured_keep_going if keep_going is None else keep_going
        )
        cmd = ["cmake", "--build", str(build_dir)]
        if not has_target_override:
            cmd += ["--target", "tidy"]
        if effective_jobs and effective_jobs > 0:
            cmd += [f"-j{effective_jobs}"]
        else:
            cmd += ["-j"]
        cmd += filtered_args
        if effective_keep_going:
            # Project preference: during tidy, continue checks even when
            # individual compilation units fail.
            cmd += ["--", "-k", "0"]
        build_start = time.perf_counter()
        ret = run_command(cmd, env=self.ctx.setup_env(), log_file=log_path)
        build_seconds = time.perf_counter() - build_start
        if ret != 0:
            print(f"--- Tidy build finished with code {ret}. Processing logs anyway...")

        # 3. Process Logs
        if log_path.exists():
            parse_start = time.perf_counter()
            log_content = log_path.read_text(encoding="utf-8", errors="replace")
            try:
                split_stats = self._split_and_sort(
                    log_content,
                    tasks_dir,
                    parse_workers=parse_workers,
                )
            except ValueError as error:
                print(f"--- Tidy log split failed: {error}")
                return 1
            parse_seconds = time.perf_counter() - parse_start

        ninja_stats = self._read_ninja_timing(ninja_log_path)
        total_seconds = time.perf_counter() - overall_start
        self._print_timing_summary(
            did_auto_configure=did_auto_configure,
            configure_seconds=configure_seconds,
            build_seconds=build_seconds,
            parse_seconds=parse_seconds,
            total_seconds=total_seconds,
            split_stats=split_stats,
            ninja_stats=ninja_stats,
            jobs=effective_jobs,
        )

        return 0

    def split_only(
        self,
        app_name: str,
        parse_workers: int | None = None,
        max_lines: int | None = None,
        max_diags: int | None = None,
        batch_size: int | None = None,
    ) -> int:
        app_dir = self.ctx.get_app_dir(app_name)
        build_dir = app_dir / "build_tidy"
        log_path = build_dir / "build.log"
        tasks_dir = build_dir / "tasks"

        if not log_path.exists():
            print(f"--- tidy-split: build log not found: {log_path}")
            print("--- tidy-split: run `tidy` first to generate build.log.")
            return 1

        parse_start = time.perf_counter()
        log_content = log_path.read_text(encoding="utf-8", errors="replace")
        try:
            split_stats = self._split_and_sort(
                log_content,
                tasks_dir,
                parse_workers=parse_workers,
                max_lines=max_lines,
                max_diags=max_diags,
                batch_size=batch_size,
            )
        except ValueError as error:
            print(f"--- tidy-split: invalid split settings: {error}")
            return 1
        parse_seconds = time.perf_counter() - parse_start

        print(
            "--- tidy-split summary: "
            f"{self._format_seconds(parse_seconds)} "
            f"(workers={split_stats['workers']}, "
            f"max_lines={split_stats['max_lines']}, "
            f"max_diags={split_stats['max_diags']}, "
            f"batch_size={split_stats['batch_size']}, "
            f"tasks={split_stats['tasks']}, batches={split_stats['batches']})"
        )
        return 0

    def _split_and_sort(
        self,
        log_content: str,
        tasks_dir: Path,
        parse_workers: int | None = None,
        max_lines: int | None = None,
        max_diags: int | None = None,
        batch_size: int | None = None,
    ):
        tasks_dir.mkdir(parents=True, exist_ok=True)
        log_lines = log_content.splitlines()
        
        # 1. Group by Ninja task
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

        # 2. Extract diagnostics and group them
        processed = []
        effective_max_lines = (
            self.ctx.config.tidy.max_lines if max_lines is None else max_lines
        )
        effective_max_diags = (
            self.ctx.config.tidy.max_diags if max_diags is None else max_diags
        )
        effective_batch_size = (
            self.ctx.config.tidy.batch_size if batch_size is None else batch_size
        )
        if effective_max_lines <= 0:
            raise ValueError("max_lines must be > 0")
        if effective_max_diags <= 0:
            raise ValueError("max_diags must be > 0")
        if effective_batch_size <= 0:
            raise ValueError("batch_size must be > 0")

        workers = self._resolve_parse_workers(parse_workers)
        if workers > 1 and len(ninja_sections) > 1:
            with ThreadPoolExecutor(max_workers=workers) as executor:
                for section_tasks in executor.map(
                    self._process_ninja_section,
                    ninja_sections,
                    [effective_max_lines] * len(ninja_sections),
                    [effective_max_diags] * len(ninja_sections),
                ):
                    processed.extend(section_tasks)
        else:
            for section in ninja_sections:
                processed.extend(
                    self._process_ninja_section(
                        section,
                        effective_max_lines,
                        effective_max_diags,
                    )
                )


        # 3. Sort and Save
        processed.sort(key=lambda x: (x["score"], x["size"]))

        self._cleanup_old_tasks(tasks_dir)
        for idx, task in enumerate(processed, 1):
            batch_index = ((idx - 1) // effective_batch_size) + 1
            batch_dir = tasks_dir / f"batch_{batch_index:03d}"
            batch_dir.mkdir(parents=True, exist_ok=True)
            (batch_dir / f"task_{idx:03d}.log").write_text(
                task["content"], encoding="utf-8"
            )
        
        self._write_markdown_summary(processed, tasks_dir / "tasks_summary.md")
        total_batches = (
            ((len(processed) - 1) // effective_batch_size) + 1
            if processed
            else 0
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
        }

    def _cleanup_old_tasks(self, tasks_dir: Path) -> None:
        if not tasks_dir.exists():
            return

        for old_task in tasks_dir.rglob("task_*.log"):
            old_task.unlink()

        batch_dirs = [path for path in tasks_dir.glob("batch_*") if path.is_dir()]
        batch_dirs.sort(key=lambda path: path.name, reverse=True)
        for batch_dir in batch_dirs:
            if any(batch_dir.iterdir()):
                continue
            batch_dir.rmdir()

    def _resolve_parse_workers(self, cli_value: int | None) -> int:
        if cli_value is not None and cli_value > 0:
            return cli_value

        configured_workers = self.ctx.config.tidy.parse_workers
        if configured_workers > 0:
            return configured_workers

        cpu_count = os.cpu_count() or 1
        return min(8, max(1, cpu_count))

    def _process_ninja_section(
        self,
        section_lines: list[str],
        max_lines: int,
        max_diags: int,
    ) -> list[dict]:
        diags = log_parser.extract_diagnostics(section_lines)
        if not diags:
            return []

        processed = []
        current_batch = []
        batch_lines = 0

        def finalize_batch(batch: list[dict]):
            if not batch:
                return

            real_file = batch[0]["file"]
            p_score = task_sorter.calculate_priority_score(batch, real_file)
            summary = log_parser.generate_text_summary(batch)
            batch_lines_content = []
            for diag in batch:
                batch_lines_content.extend(diag["lines"])

            content = (
                f"File: {real_file}\n"
                + "=" * 60
                + "\n"
                + summary
                + "\n".join(batch_lines_content)
            )
            processed.append(
                {
                    "content": content,
                    "score": p_score,
                    "size": len(content),
                    "diag": batch,
                    "file": real_file,
                }
            )

        for diag in diags:
            diag_len = len(diag["lines"])
            if current_batch and (
                len(current_batch) >= max_diags
                or batch_lines + diag_len > max_lines
            ):
                finalize_batch(current_batch)
                current_batch = []
                batch_lines = 0

            current_batch.append(diag)
            batch_lines += diag_len

        finalize_batch(current_batch)
        return processed

    def _read_ninja_timing(self, ninja_log_path: Path) -> dict | None:
        if not ninja_log_path.exists():
            return None

        durations_ms_by_step = {}
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
                {"step": step_key, "seconds": duration_ms / 1000.0}
                for step_key, duration_ms in slowest
            ],
        }

    def _format_seconds(self, seconds: float) -> str:
        if seconds < 60:
            return f"{seconds:.2f}s"
        minutes = int(seconds // 60)
        remain_seconds = seconds - minutes * 60
        return f"{minutes}m{remain_seconds:.2f}s"

    def _print_timing_summary(
        self,
        did_auto_configure: bool,
        configure_seconds: float,
        build_seconds: float,
        parse_seconds: float,
        total_seconds: float,
        split_stats: dict | None,
        ninja_stats: dict | None,
        jobs: int | None,
    ):
        jobs_label = str(jobs) if jobs and jobs > 0 else "auto"
        print("--- Tidy timing summary ---")
        if did_auto_configure:
            print(f"auto-configure: {self._format_seconds(configure_seconds)}")
        print(f"build (jobs={jobs_label}): {self._format_seconds(build_seconds)}")
        if split_stats:
            print(
                "log split: "
                f"{self._format_seconds(parse_seconds)} "
                f"(sections={split_stats['sections']}, workers={split_stats['workers']}, "
                f"max_lines={split_stats['max_lines']}, "
                f"max_diags={split_stats['max_diags']}, "
                f"tasks={split_stats['tasks']}, batches={split_stats['batches']}, "
                f"batch_size={split_stats['batch_size']})"
            )
        else:
            print("log split: skipped (build.log not found)")
        print(f"total: {self._format_seconds(total_seconds)}")

        if not ninja_stats:
            return

        print(
            "ninja tidy steps: "
            f"count={ninja_stats['count']}, "
            f"wall={self._format_seconds(ninja_stats['wall_seconds'])}, "
            f"avg={self._format_seconds(ninja_stats['avg_seconds'])}, "
            f"p95={self._format_seconds(ninja_stats['p95_seconds'])}"
        )
        if ninja_stats["slowest"]:
            slowest_text = ", ".join(
                f"{item['step']}={self._format_seconds(item['seconds'])}"
                for item in ninja_stats["slowest"]
            )
            print(f"slowest steps: {slowest_text}")


    def _write_markdown_summary(self, processed: list, out_path: Path):
        lines = ["# Clang-Tidy Tasks Summary\n", "| ID | File | Difficulty Score | Warning Types |", "| --- | --- | --- | --- |"]
        for idx, item in enumerate(processed, 1):
            w_types = ", ".join(sorted(set(w["check"] for w in item["diag"])))
            lines.append(f"| {idx:03d} | {item['file']} | {item['score']:.2f} | {w_types} |")
        out_path.write_text("\n".join(lines), encoding="utf-8")
