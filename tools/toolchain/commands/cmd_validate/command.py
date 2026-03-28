from __future__ import annotations

import shutil
from pathlib import Path
from time import monotonic

from ...core.context import Context
from ...core.path_display import to_repo_relative
from ..cmd_build import BuildCommand
from .plan import ValidationPlan, load_validation_plan
from .scope import resolve_scope_paths
from .step_runner import run_logged_step, shell_join
from .summary import (
    TrackResult,
    ValidationSummaryWriter,
    utc_now_iso,
    write_summary_payload,
)
from .verify_track import collect_track_artifacts, execute_verify_track


class ValidateCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        plan_path: str,
        raw_paths: list[str] | None = None,
        paths_file: str | None = None,
        run_name: str | None = None,
        verbose: bool = False,
    ) -> int:
        try:
            plan = load_validation_plan(Path(plan_path), run_name_override=run_name)
            effective_raw_paths = list(raw_paths or [])
            effective_paths_file = paths_file
            if not effective_raw_paths and not effective_paths_file:
                effective_raw_paths = list(plan.scope.paths)
                effective_paths_file = plan.scope.paths_file
            scope_paths = resolve_scope_paths(
                self.ctx.repo_root,
                effective_raw_paths,
                effective_paths_file,
            )
        except (FileNotFoundError, ValueError, OSError) as exc:
            print(f"Error: {exc}")
            return 2

        layout = self.ctx.get_validation_layout(plan.run_name)
        if layout.root.exists():
            shutil.rmtree(layout.root, ignore_errors=True)
        layout.logs_dir.mkdir(parents=True, exist_ok=True)
        layout.tracks_dir.mkdir(parents=True, exist_ok=True)

        writer = ValidationSummaryWriter(self.ctx.repo_root, layout, plan, scope_paths)
        command_text = self._build_command_text(
            plan_path=plan.plan_path,
            raw_paths=raw_paths or [],
            paths_file=paths_file,
            run_name=run_name,
            verbose=verbose,
        )
        started_at = monotonic()
        started_at_iso = utc_now_iso()
        track_results: list[TrackResult] = []
        total_tracks = len(plan.tracks)
        exit_code = 0

        for index, track in enumerate(plan.tracks, start=1):
            writer.add_track_start(index=index, total=total_tracks, track=track)
            print(f"--- validate: [{index}/{total_tracks}] {track.name} ({track.kind}) starting")
            track_result = self._execute_track(
                plan=plan,
                track_index=index,
                writer=writer,
                verbose=verbose,
                started_at=started_at,
            )
            track_results.append(track_result)
            writer.add_track_end(track=track_result)

            if track_result.status == "completed":
                print(
                    f"--- validate: [{index}/{total_tracks}] {track.name} completed "
                    f"in {track_result.duration_ms}ms"
                )
            else:
                exit_code = int(track_result.exit_code or 1)
                failed_step = next(
                    (step for step in track_result.steps if step.status != "completed"),
                    None,
                )
                step_name = failed_step.name if failed_step else "unknown"
                print(
                    f"--- validate: [{index}/{total_tracks}] {track.name} failed at "
                    f"{step_name} (exit {exit_code})"
                )
                if failed_step is not None and failed_step.key_error_lines:
                    for line in failed_step.key_error_lines:
                        print(f"  - {line}")
                print(f"--- validate: summary_json={layout.summary_json_path}")
                print(f"--- validate: full_log={layout.output_full_log_path}")
                if not plan.continue_on_failure:
                    break

            self._write_summary(
                writer=writer,
                command_text=command_text,
                plan=plan,
                scope_paths=scope_paths,
                track_results=track_results,
                started_at_iso=started_at_iso,
                started_at=started_at,
                exit_code=exit_code,
            )

        success = all(track.status == "completed" for track in track_results) and len(track_results) == total_tracks
        final_exit_code = 0 if success else exit_code or 1
        writer.add_run_end(success=success, exit_code=final_exit_code)
        self._write_summary(
            writer=writer,
            command_text=command_text,
            plan=plan,
            scope_paths=scope_paths,
            track_results=track_results,
            started_at_iso=started_at_iso,
            started_at=started_at,
            exit_code=final_exit_code,
            success=success,
        )
        if success:
            print(
                f"--- validate: completed successfully ({len(track_results)}/{total_tracks} tracks) "
                f"summary={layout.summary_json_path}"
            )
        else:
            print(f"--- validate: completed with failures summary={layout.summary_json_path}")
        return final_exit_code

    def _build_command_text(
        self,
        *,
        plan_path: Path,
        raw_paths: list[str],
        paths_file: str | None,
        run_name: str | None,
        verbose: bool,
    ) -> str:
        cmd = [
            "python",
            "tools/run.py",
            "validate",
            "--plan",
            to_repo_relative(self.ctx.repo_root, plan_path),
        ]
        for item in raw_paths:
            cmd.extend(["--paths", item])
        if paths_file:
            cmd.extend(["--paths-file", paths_file])
        if run_name:
            cmd.extend(["--run-name", run_name])
        if verbose:
            cmd.append("--verbose")
        return shell_join(cmd)

    def _execute_track(
        self,
        *,
        plan: ValidationPlan,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
        started_at: float,
    ) -> TrackResult:
        track = plan.tracks[track_index - 1]
        if track.kind == "configure":
            return self._run_builder_track(
                track=track,
                track_index=track_index,
                writer=writer,
                verbose=verbose,
                started_at=started_at,
                step_name="configure",
                builder_method="configure",
            )
        if track.kind == "build":
            return self._run_builder_track(
                track=track,
                track_index=track_index,
                writer=writer,
                verbose=verbose,
                started_at=started_at,
                step_name="build",
                builder_method="build",
            )
        return execute_verify_track(
            ctx=self.ctx,
            track=track,
            track_index=track_index,
            writer=writer,
            verbose=verbose,
            started_at=started_at,
        )

    def _run_builder_track(
        self,
        *,
        track,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
        started_at: float,
        step_name: str,
        builder_method: str,
    ) -> TrackResult:
        builder = BuildCommand(self.ctx)
        resolved_build_dir = builder.resolve_build_dir_name(
            tidy=False,
            build_dir_name=track.build_dir,
            profile_name=track.profile,
            app_name=track.app,
        )
        action = getattr(builder, builder_method)
        step = run_logged_step(
            repo_root=self.ctx.repo_root,
            track_name=track.name,
            track_index=track_index,
            step_name=step_name,
            writer=writer,
            verbose=verbose,
            action=lambda run_command_fn: action(
                app_name=track.app,
                tidy=False,
                cmake_args=track.cmake_args,
                build_dir_name=track.build_dir,
                profile_name=track.profile,
                concise=track.concise,
                kill_build_procs=track.kill_build_procs,
                run_command_fn=run_command_fn,
            ),
        )
        return TrackResult(
            name=track.name,
            kind=track.kind,
            app=track.app,
            profile=track.profile,
            build_dir=resolved_build_dir,
            cmake_args=list(track.cmake_args),
            status="completed" if step.exit_code == 0 else "failed",
            exit_code=step.exit_code,
            duration_ms=int((monotonic() - started_at) * 1000),
            steps=[step],
            artifacts=collect_track_artifacts(self.ctx, track.app, resolved_build_dir),
        )

    def _write_summary(
        self,
        *,
        writer: ValidationSummaryWriter,
        command_text: str,
        plan: ValidationPlan,
        scope_paths: list[str],
        track_results: list[TrackResult],
        started_at_iso: str,
        started_at: float,
        exit_code: int,
        success: bool | None = None,
    ) -> None:
        write_summary_payload(
            writer=writer,
            repo_root=self.ctx.repo_root,
            command_text=command_text,
            plan=plan,
            scope_paths=scope_paths,
            track_results=track_results,
            started_at_iso=started_at_iso,
            started_at=started_at,
            exit_code=exit_code,
            success=success,
        )
