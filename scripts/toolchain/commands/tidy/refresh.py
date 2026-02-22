from pathlib import Path

from ...core.context import Context
from . import (
    refresh_execute as tidy_refresh_execute,
    refresh_mapper as tidy_refresh_mapper,
    refresh_runner as tidy_refresh_runner,
    refresh_state as tidy_refresh_state,
)


class TidyRefreshCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    # --- Mapper helpers ---
    def _normalize_batch_name(self, batch_id: str) -> str:
        return tidy_refresh_mapper.normalize_batch_name(batch_id)

    def _collect_batch_files(self, batch_dir: Path) -> list[Path]:
        return tidy_refresh_mapper.collect_batch_files(batch_dir)

    def _load_compile_units(self, compile_commands_path: Path) -> list[Path]:
        return tidy_refresh_mapper.load_compile_units(compile_commands_path)

    def _resolve_incremental_files(
        self,
        touched_files: list[Path],
        compile_units: list[Path],
        app_dir: Path,
        neighbor_scope: str,
    ) -> list[Path]:
        return tidy_refresh_mapper.resolve_incremental_files(
            touched_files=touched_files,
            compile_units=compile_units,
            app_dir=app_dir,
            neighbor_scope=neighbor_scope,
        )

    def _module_key(self, file_path: Path, app_dir: Path) -> str:
        return tidy_refresh_mapper.module_key(file_path, app_dir)

    def _path_key(self, path: Path) -> str:
        return tidy_refresh_mapper.path_key(path)

    # --- State helpers ---
    def _load_state(self, state_path: Path) -> dict:
        return tidy_refresh_state.load_state(state_path)

    def _ensure_processed_batches(self, state: dict) -> list[str]:
        return tidy_refresh_state.ensure_processed_batches(state)

    def _register_batch(self, state: dict, batch_name: str | None) -> bool:
        return tidy_refresh_state.register_batch(state, batch_name)

    def _cadence_due(
        self,
        state: dict,
        batch_name: str | None,
        already_processed: bool,
        full_every: int,
    ) -> bool:
        return tidy_refresh_state.cadence_due(
            state=state,
            batch_name=batch_name,
            already_processed=already_processed,
            full_every=full_every,
        )

    def _resolve_full_reasons(
        self,
        force_full: bool,
        final_full: bool,
        cadence_is_due: bool,
        full_every: int,
    ) -> list[str]:
        return tidy_refresh_state.resolve_full_reasons(
            force_full=force_full,
            final_full=final_full,
            cadence_is_due=cadence_is_due,
            full_every=full_every,
        )

    def _write_state(self, state_path: Path, state: dict) -> None:
        tidy_refresh_state.write_state(state_path, state)

    def _print_preview(
        self,
        batch_name: str | None,
        touched_files: list[Path],
        incremental_files: list[Path],
        neighbor_scope: str,
        full_every: int,
        cadence_due: bool,
        full_reasons: list[str],
        keep_going: bool,
    ) -> None:
        tidy_refresh_state.print_preview(
            batch_name=batch_name,
            touched_files=touched_files,
            incremental_files=incremental_files,
            neighbor_scope=neighbor_scope,
            full_every=full_every,
            cadence_due=cadence_due,
            full_reasons=full_reasons,
            keep_going=keep_going,
        )

    def _utc_now_iso(self) -> str:
        return tidy_refresh_state.utc_now_iso()

    # --- Runner helpers ---
    def _ensure_compile_commands(
        self,
        app_name: str,
        build_dir: Path,
        build_dir_name: str,
    ) -> int:
        return tidy_refresh_runner.ensure_compile_commands(
            ctx=self.ctx,
            app_name=app_name,
            build_dir=build_dir,
            build_dir_name=build_dir_name,
        )

    def _run_incremental_tidy(
        self,
        build_dir: Path,
        batch_name: str,
        files: list[Path],
        keep_going: bool,
    ) -> int:
        return tidy_refresh_runner.run_incremental_tidy(
            ctx=self.ctx,
            build_dir=build_dir,
            batch_name=batch_name,
            files=files,
            keep_going=keep_going,
        )

    def _detect_incremental_auto_reasons(
        self,
        build_dir: Path,
        batch_name: str,
    ) -> list[str]:
        return tidy_refresh_runner.detect_incremental_auto_reasons(
            build_dir=build_dir,
            batch_name=batch_name,
        )

    def _detect_build_log_auto_reasons(self, build_dir: Path) -> list[str]:
        return tidy_refresh_runner.detect_build_log_auto_reasons(build_dir=build_dir)

    def _build_log_mtime_ns(self, build_dir: Path) -> int | None:
        return tidy_refresh_runner.build_log_mtime_ns(build_dir=build_dir)

    def _run_full_tidy(
        self,
        app_name: str,
        jobs: int | None,
        parse_workers: int | None,
        keep_going: bool,
        build_dir_name: str,
    ) -> int:
        return tidy_refresh_runner.run_full_tidy(
            ctx=self.ctx,
            app_name=app_name,
            jobs=jobs,
            parse_workers=parse_workers,
            keep_going=keep_going,
            build_dir_name=build_dir_name,
        )

    def _chunk_paths(self, paths: list[Path], chunk_size: int) -> list[list[Path]]:
        return tidy_refresh_runner.chunk_paths(paths, chunk_size)

    # --- Public entrypoint ---
    def execute(
        self,
        app_name: str,
        batch_id: str | None = None,
        full_every: int = 3,
        neighbor_scope: str = "none",
        jobs: int | None = None,
        parse_workers: int | None = None,
        keep_going: bool | None = None,
        force_full: bool = False,
        final_full: bool = False,
        build_dir_name: str | None = None,
        dry_run: bool = False,
    ) -> int:
        return tidy_refresh_execute.execute_refresh_command(
            command=self,
            app_name=app_name,
            batch_id=batch_id,
            full_every=full_every,
            neighbor_scope=neighbor_scope,
            jobs=jobs,
            parse_workers=parse_workers,
            keep_going=keep_going,
            force_full=force_full,
            final_full=final_full,
            build_dir_name=build_dir_name,
            dry_run=dry_run,
        )
