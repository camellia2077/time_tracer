from pathlib import Path

from ...core.context import Context
from . import (
    command_execute as tidy_command_execute,
    command_split as tidy_command_split,
    invoker as tidy_invoker,
    log_splitter as tidy_log_splitter,
    task_builder as tidy_task_builder,
)


class TidyCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    # --- Build/invocation helpers ---
    def _resolve_tidy_paths(
        self,
        app_name: str,
        build_dir_name: str | None = None,
    ) -> dict[str, Path]:
        return tidy_invoker.resolve_tidy_paths(
            self.ctx,
            app_name,
            build_dir_name=build_dir_name,
        )

    def _ensure_configured(
        self,
        app_name: str,
        build_dir: Path,
        build_dir_name: str | None = None,
    ) -> tuple[int, bool, float]:
        return tidy_invoker.ensure_configured(
            self.ctx,
            app_name,
            build_dir,
            build_dir_name=build_dir_name,
        )

    def _resolve_build_options(
        self,
        extra_args: list[str] | None,
        jobs: int | None,
        keep_going: bool | None,
    ) -> tuple[list[str], bool, int | None, bool]:
        return tidy_invoker.resolve_build_options(self.ctx, extra_args, jobs, keep_going)

    def _build_tidy_command(
        self,
        app_name: str,
        build_dir: Path,
        filtered_args: list[str],
        has_target_override: bool,
        effective_jobs: int | None,
        effective_keep_going: bool,
    ) -> list[str]:
        return tidy_invoker.build_tidy_command(
            app_name,
            build_dir,
            filtered_args,
            has_target_override,
            effective_jobs,
            effective_keep_going,
        )

    def _run_tidy_build(self, cmd: list[str], log_path: Path) -> tuple[int, float]:
        return tidy_invoker.run_tidy_build(self.ctx, cmd, log_path)

    # --- Log split helpers ---
    def _split_from_log(
        self,
        log_path: Path,
        tasks_dir: Path,
        parse_workers: int | None = None,
        max_lines: int | None = None,
        max_diags: int | None = None,
        batch_size: int | None = None,
    ) -> tuple[dict, float]:
        return tidy_log_splitter.split_from_log(
            log_path,
            tasks_dir,
            split_and_sort_fn=self._split_and_sort,
            parse_workers=parse_workers,
            max_lines=max_lines,
            max_diags=max_diags,
            batch_size=batch_size,
        )

    def _split_and_sort(
        self,
        log_content: str,
        tasks_dir: Path,
        parse_workers: int | None = None,
        max_lines: int | None = None,
        max_diags: int | None = None,
        batch_size: int | None = None,
    ) -> dict:
        return tidy_task_builder.split_and_sort(
            self.ctx,
            log_content,
            tasks_dir,
            parse_workers=parse_workers,
            max_lines=max_lines,
            max_diags=max_diags,
            batch_size=batch_size,
        )

    def _group_ninja_sections(self, log_lines: list[str]) -> list[list[str]]:
        return tidy_task_builder.group_ninja_sections(log_lines)

    def _resolve_split_limits(
        self,
        max_lines: int | None,
        max_diags: int | None,
        batch_size: int | None,
    ) -> tuple[int, int, int]:
        return tidy_task_builder.resolve_split_limits(
            self.ctx,
            max_lines=max_lines,
            max_diags=max_diags,
            batch_size=batch_size,
        )

    def _collect_section_tasks(
        self,
        ninja_sections: list[list[str]],
        max_lines: int,
        max_diags: int,
        workers: int,
    ) -> list[dict]:
        return tidy_task_builder.collect_section_tasks(
            ninja_sections,
            max_lines,
            max_diags,
            workers,
        )

    def _write_task_batches(
        self,
        processed: list[dict],
        tasks_dir: Path,
        batch_size: int,
    ) -> int:
        return tidy_task_builder.write_task_batches(processed, tasks_dir, batch_size)

    def _cleanup_old_tasks(self, tasks_dir: Path) -> None:
        tidy_task_builder.cleanup_old_tasks(tasks_dir)

    def _resolve_parse_workers(self, cli_value: int | None) -> int:
        return tidy_task_builder.resolve_parse_workers(self.ctx, cli_value)

    def _process_ninja_section(
        self,
        section_lines: list[str],
        max_lines: int,
        max_diags: int,
    ) -> list[dict]:
        return tidy_task_builder.process_ninja_section(
            section_lines,
            max_lines,
            max_diags,
        )

    # --- Timing/report helpers ---
    def _read_ninja_timing(self, ninja_log_path: Path) -> dict | None:
        return tidy_log_splitter.read_ninja_timing(ninja_log_path)

    def _format_seconds(self, seconds: float) -> str:
        return tidy_log_splitter.format_seconds(seconds)

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
    ) -> None:
        tidy_log_splitter.print_timing_summary(
            did_auto_configure=did_auto_configure,
            configure_seconds=configure_seconds,
            build_seconds=build_seconds,
            parse_seconds=parse_seconds,
            total_seconds=total_seconds,
            split_stats=split_stats,
            ninja_stats=ninja_stats,
            jobs=jobs,
        )

    def _write_markdown_summary(self, processed: list, out_path: Path) -> None:
        tidy_task_builder.write_markdown_summary(processed, out_path)

    # --- Public entrypoints ---
    def execute(
        self,
        app_name: str,
        extra_args: list[str] | None = None,
        jobs: int | None = None,
        parse_workers: int | None = None,
        keep_going: bool | None = None,
        build_dir_name: str | None = None,
    ) -> int:
        return tidy_command_execute.execute_tidy_command(
            command=self,
            app_name=app_name,
            extra_args=extra_args,
            jobs=jobs,
            parse_workers=parse_workers,
            keep_going=keep_going,
            build_dir_name=build_dir_name,
        )

    def split_only(
        self,
        app_name: str,
        parse_workers: int | None = None,
        max_lines: int | None = None,
        max_diags: int | None = None,
        batch_size: int | None = None,
        build_dir_name: str | None = None,
    ) -> int:
        return tidy_command_split.split_only_tidy_command(
            command=self,
            app_name=app_name,
            parse_workers=parse_workers,
            max_lines=max_lines,
            max_diags=max_diags,
            batch_size=batch_size,
            build_dir_name=build_dir_name,
        )
