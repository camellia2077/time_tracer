from __future__ import annotations

import json
import shutil
import sys
import traceback
from contextlib import redirect_stderr, redirect_stdout
from dataclasses import asdict, dataclass, field
from datetime import UTC, datetime
from pathlib import Path
from time import monotonic

from ...core.context import Context
from ...core.executor import run_command
from ...services.log_analysis import extract_key_error_lines_from_log
from ...services.suite_registry import (
    needs_suite_build,
    resolve_suite_bin_dir,
    resolve_suite_name,
    resolve_suite_runner_name,
)
from ..cmd_build import BuildCommand
from ..cmd_quality.verify import VerifyCommand
from ..cmd_quality.verify_internal.verify_build_stage import execute_build_stage
from ..cmd_quality.verify_internal.verify_markdown_gate_runner import run_report_markdown_gates
from ..cmd_quality.verify_internal.verify_native_runner import run_native_core_runtime_tests
from ..cmd_quality.verify_internal.verify_result_writer import write_build_only_result_json
from ..cmd_quality.verify_internal.verify_suite_runner import build_suite_test_command
from .plan import TrackSpec, ValidationPlan, load_validation_plan, normalize_run_name
from .scope import resolve_scope_paths


def _utc_now_iso() -> str:
    return datetime.now(UTC).isoformat()


def _shell_join(cmd: list[str]) -> str:
    return " ".join(str(part) for part in cmd)


def _to_repo_relative(repo_root: Path, path: Path) -> str:
    try:
        return str(path.resolve().relative_to(repo_root.resolve())).replace("\\", "/")
    except ValueError:
        return str(path.resolve())


def _safe_segment(text: str) -> str:
    normalized = normalize_run_name(text).replace(".", "_")
    return normalized or "item"


@dataclass
class StepResult:
    name: str
    command: str
    status: str
    exit_code: int | None
    duration_ms: int
    log_path: str
    key_error_lines: list[str] = field(default_factory=list)


@dataclass
class TrackResult:
    name: str
    kind: str
    app: str
    profile: str | None
    build_dir: str | None
    cmake_args: list[str]
    status: str
    exit_code: int | None
    duration_ms: int
    steps: list[StepResult] = field(default_factory=list)
    artifacts: dict[str, str] = field(default_factory=dict)


@dataclass
class _StepCapture:
    command_texts: list[str] = field(default_factory=list)

    def run_command(self, cmd: list[str], **kwargs) -> int:
        self.command_texts.append(_shell_join(cmd))
        return run_command(cmd, **kwargs)


class _TeeStream:
    def __init__(self, primary, mirror):
        self._primary = primary
        self._mirror = mirror

    @property
    def encoding(self):
        encoding = getattr(self._primary, "encoding", None)
        if encoding:
            return encoding
        return getattr(self._mirror, "encoding", None)

    def write(self, text: str) -> int:
        written = self._primary.write(text)
        self._mirror.write(text)
        return written

    def flush(self) -> None:
        self._primary.flush()
        self._mirror.flush()


class ValidationSummaryWriter:
    def __init__(self, repo_root: Path, layout, plan: ValidationPlan, scope_paths: list[str]):
        self.repo_root = repo_root
        self.layout = layout
        self.plan = plan
        self.scope_paths = list(scope_paths)
        self._lines: list[str] = [
            f"run_name: {plan.run_name}",
            f"plan_path: {_to_repo_relative(repo_root, plan.plan_path)}",
            f"scope_paths: {', '.join(scope_paths)}",
            "",
        ]
        self.flush_output_log()

    def flush_output_log(self) -> None:
        self.layout.logs_dir.mkdir(parents=True, exist_ok=True)
        self.layout.output_log_path.write_text(
            "\n".join(self._lines).rstrip() + "\n",
            encoding="utf-8",
        )

    def add_line(self, text: str = "") -> None:
        self._lines.append(text)
        self.flush_output_log()

    def add_track_start(self, *, index: int, total: int, track: TrackSpec) -> None:
        self.add_line(f"[track {index}/{total}] {track.name} kind={track.kind} app={track.app}")

    def add_step(self, *, track_name: str, step: StepResult) -> None:
        prefix = "[ok]" if step.status == "completed" else "[failed]"
        self.add_line(
            f"{prefix} {track_name}/{step.name} exit={step.exit_code} "
            f"duration_ms={step.duration_ms} log={step.log_path}"
        )
        if step.key_error_lines:
            for line in step.key_error_lines:
                self.add_line(f"  key_error: {line}")

    def add_track_end(self, *, track: TrackResult) -> None:
        self.add_line(
            f"[track done] {track.name} status={track.status} "
            f"exit={track.exit_code} duration_ms={track.duration_ms}"
        )
        self.add_line("")

    def add_run_end(self, *, success: bool, exit_code: int) -> None:
        self.add_line(f"success: {str(success).lower()}")
        self.add_line(f"exit_code: {exit_code}")
        self.add_line(f"summary_json: {_to_repo_relative(self.repo_root, self.layout.summary_json_path)}")

    def append_full_log(self, *, track_name: str, step_name: str, step_log_path: Path) -> None:
        try:
            content = step_log_path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            content = ""
        with self.layout.output_full_log_path.open("a", encoding="utf-8") as handle:
            handle.write(f"===== {track_name}/{step_name} =====\n")
            if content:
                handle.write(content.rstrip() + "\n")
            handle.write("\n")

    def write_summary(self, payload: dict) -> None:
        self.layout.summary_json_path.write_text(
            json.dumps(payload, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )


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
            scope_paths = resolve_scope_paths(self.ctx.repo_root, raw_paths, paths_file)
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
        started_at_iso = _utc_now_iso()
        track_results: list[TrackResult] = []
        total_tracks = len(plan.tracks)
        exit_code = 0

        for index, track in enumerate(plan.tracks, start=1):
            writer.add_track_start(index=index, total=total_tracks, track=track)
            print(f"--- validate: [{index}/{total_tracks}] {track.name} ({track.kind}) starting")
            track_result = self._execute_track(
                track=track,
                track_index=index,
                writer=writer,
                verbose=verbose,
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
            _to_repo_relative(self.ctx.repo_root, plan_path),
        ]
        for item in raw_paths:
            cmd.extend(["--paths", item])
        if paths_file:
            cmd.extend(["--paths-file", paths_file])
        if run_name:
            cmd.extend(["--run-name", run_name])
        if verbose:
            cmd.append("--verbose")
        return _shell_join(cmd)

    def _execute_track(
        self,
        *,
        track: TrackSpec,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
    ) -> TrackResult:
        started_at = monotonic()
        if track.kind == "configure":
            return self._run_configure_track(
                track=track,
                track_index=track_index,
                writer=writer,
                verbose=verbose,
                started_at=started_at,
            )
        if track.kind == "build":
            return self._run_build_track(
                track=track,
                track_index=track_index,
                writer=writer,
                verbose=verbose,
                started_at=started_at,
            )
        return self._run_verify_track(
            track=track,
            track_index=track_index,
            writer=writer,
            verbose=verbose,
            started_at=started_at,
        )

    def _run_configure_track(
        self,
        *,
        track: TrackSpec,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
        started_at: float,
    ) -> TrackResult:
        builder = BuildCommand(self.ctx)
        resolved_build_dir = builder.resolve_build_dir_name(
            tidy=False,
            build_dir_name=track.build_dir,
            profile_name=track.profile,
            app_name=track.app,
        )
        step = self._run_logged_step(
            track_name=track.name,
            track_index=track_index,
            step_name="configure",
            writer=writer,
            verbose=verbose,
            action=lambda run_command_fn: builder.configure(
                app_name=track.app,
                tidy=False,
                cmake_args=track.cmake_args,
                build_dir_name=track.build_dir,
                profile_name=track.profile,
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
            artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
        )

    def _run_build_track(
        self,
        *,
        track: TrackSpec,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
        started_at: float,
    ) -> TrackResult:
        builder = BuildCommand(self.ctx)
        resolved_build_dir = builder.resolve_build_dir_name(
            tidy=False,
            build_dir_name=track.build_dir,
            profile_name=track.profile,
            app_name=track.app,
        )
        step = self._run_logged_step(
            track_name=track.name,
            track_index=track_index,
            step_name="build",
            writer=writer,
            verbose=verbose,
            action=lambda run_command_fn: builder.build(
                app_name=track.app,
                tidy=False,
                cmake_args=track.cmake_args,
                build_dir_name=track.build_dir,
                profile_name=track.profile,
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
            artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
        )

    def _run_verify_track(
        self,
        *,
        track: TrackSpec,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
        started_at: float,
    ) -> TrackResult:
        verify_cmd = VerifyCommand(self.ctx)
        builder = BuildCommand(self.ctx)
        steps: list[StepResult] = []
        suite_name = resolve_suite_name(track.app)

        build_step = self._run_logged_step(
            track_name=track.name,
            track_index=track_index,
            step_name="build",
            writer=writer,
            verbose=verbose,
            action=lambda run_command_fn: execute_build_stage(
                ctx=self.ctx,
                build_command_cls=BuildCommand,
                app_name=track.app,
                tidy=False,
                extra_args=None,
                cmake_args=track.cmake_args,
                build_dir_name=track.build_dir,
                profile_name=track.profile,
                kill_build_procs=track.kill_build_procs,
                run_command_fn=run_command_fn,
            )[0],
        )
        steps.append(build_step)

        resolved_build_dir = builder.resolve_build_dir_name(
            tidy=False,
            build_dir_name=track.build_dir,
            profile_name=track.profile,
            app_name=track.app,
        )

        if build_step.exit_code != 0:
            write_build_only_result_json(
                repo_root=self.ctx.repo_root,
                app_name=track.app,
                build_dir_name=resolved_build_dir,
                success=False,
                exit_code=int(build_step.exit_code or 1),
                duration_seconds=monotonic() - started_at,
                error_message="Build failed during validate verify track.",
                build_only=(suite_name is None),
            )
            return TrackResult(
                name=track.name,
                kind=track.kind,
                app=track.app,
                profile=track.profile,
                build_dir=resolved_build_dir,
                cmake_args=list(track.cmake_args),
                status="failed",
                exit_code=build_step.exit_code,
                duration_ms=int((monotonic() - started_at) * 1000),
                steps=steps,
                artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
            )

        if track.verify_scope in {"unit", "batch"}:
            unit_step = self._run_logged_step(
                track_name=track.name,
                track_index=track_index,
                step_name="unit_checks",
                writer=writer,
                verbose=verbose,
                action=lambda run_command_fn: verify_cmd.run_unit_scope_checks(
                    run_command_fn=run_command_fn
                ),
            )
            steps.append(unit_step)
            if unit_step.exit_code != 0:
                return TrackResult(
                    name=track.name,
                    kind=track.kind,
                    app=track.app,
                    profile=track.profile,
                    build_dir=resolved_build_dir,
                    cmake_args=list(track.cmake_args),
                    status="failed",
                    exit_code=unit_step.exit_code,
                    duration_ms=int((monotonic() - started_at) * 1000),
                    steps=steps,
                    artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
                )

        if track.verify_scope == "task":
            task_step = self._run_logged_step(
                track_name=track.name,
                track_index=track_index,
                step_name="task_checks",
                writer=writer,
                verbose=verbose,
                action=lambda run_command_fn: verify_cmd.run_task_scope_checks(
                    app_name=track.app,
                    build_dir_name=resolved_build_dir,
                    run_command_fn=run_command_fn,
                ),
            )
            steps.append(task_step)
            status = "completed" if task_step.exit_code == 0 else "failed"
            return TrackResult(
                name=track.name,
                kind=track.kind,
                app=track.app,
                profile=track.profile,
                build_dir=resolved_build_dir,
                cmake_args=list(track.cmake_args),
                status=status,
                exit_code=task_step.exit_code,
                duration_ms=int((monotonic() - started_at) * 1000),
                steps=steps,
                artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
            )

        if track.verify_scope in {"artifact", "batch"}:
            suite_step = self._run_suite_step(
                track=track,
                track_index=track_index,
                writer=writer,
                verbose=verbose,
                build_dir_name=resolved_build_dir,
                verify_cmd=verify_cmd,
            )
            if suite_step is not None:
                steps.append(suite_step)
                if suite_step.exit_code != 0:
                    return TrackResult(
                        name=track.name,
                        kind=track.kind,
                        app=track.app,
                        profile=track.profile,
                        build_dir=resolved_build_dir,
                        cmake_args=list(track.cmake_args),
                        status="failed",
                        exit_code=suite_step.exit_code,
                        duration_ms=int((monotonic() - started_at) * 1000),
                        steps=steps,
                        artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
                    )

                markdown_step = self._run_logged_step(
                    track_name=track.name,
                    track_index=track_index,
                    step_name="markdown_gates",
                    writer=writer,
                    verbose=verbose,
                    action=lambda run_command_fn: run_report_markdown_gates(
                        repo_root=self.ctx.repo_root,
                        setup_env_fn=self.ctx.setup_env,
                        run_command_fn=run_command_fn,
                        app_name=track.app,
                        build_dir_name=resolved_build_dir,
                        normalize_ext=tuple(self.ctx.config.quality.gate_audit.normalize_ext),
                    ),
                )
                steps.append(markdown_step)
                if markdown_step.exit_code != 0:
                    return TrackResult(
                        name=track.name,
                        kind=track.kind,
                        app=track.app,
                        profile=track.profile,
                        build_dir=resolved_build_dir,
                        cmake_args=list(track.cmake_args),
                        status="failed",
                        exit_code=markdown_step.exit_code,
                        duration_ms=int((monotonic() - started_at) * 1000),
                        steps=steps,
                        artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
                    )

                native_step = self._run_logged_step(
                    track_name=track.name,
                    track_index=track_index,
                    step_name="native_runtime",
                    writer=writer,
                    verbose=verbose,
                    action=lambda run_command_fn: run_native_core_runtime_tests(
                        repo_root=self.ctx.repo_root,
                        setup_env_fn=self.ctx.setup_env,
                        run_command_fn=run_command_fn,
                        app_name=track.app,
                        build_dir_name=resolved_build_dir,
                    ),
                )
                steps.append(native_step)
                if native_step.exit_code != 0:
                    return TrackResult(
                        name=track.name,
                        kind=track.kind,
                        app=track.app,
                        profile=track.profile,
                        build_dir=resolved_build_dir,
                        cmake_args=list(track.cmake_args),
                        status="failed",
                        exit_code=native_step.exit_code,
                        duration_ms=int((monotonic() - started_at) * 1000),
                        steps=steps,
                        artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
                    )
            else:
                write_build_only_result_json(
                    repo_root=self.ctx.repo_root,
                    app_name=track.app,
                    build_dir_name=resolved_build_dir,
                    success=True,
                    exit_code=0,
                    duration_seconds=monotonic() - started_at,
                    build_only=True,
                )

        return TrackResult(
            name=track.name,
            kind=track.kind,
            app=track.app,
            profile=track.profile,
            build_dir=resolved_build_dir,
            cmake_args=list(track.cmake_args),
            status="completed",
            exit_code=0,
            duration_ms=int((monotonic() - started_at) * 1000),
            steps=steps,
            artifacts=self._collect_track_artifacts(track.app, resolved_build_dir),
        )

    def _run_suite_step(
        self,
        *,
        track: TrackSpec,
        track_index: int,
        writer: ValidationSummaryWriter,
        verbose: bool,
        build_dir_name: str,
        verify_cmd: VerifyCommand,
    ) -> StepResult | None:
        test_cmd = build_suite_test_command(
            app_name=track.app,
            build_dir_name=build_dir_name,
            profile_name=track.profile,
            concise=track.concise,
            skip_suite_build=True,
            resolve_suite_name_fn=resolve_suite_name,
            resolve_suite_runner_name_fn=resolve_suite_runner_name,
            resolve_suite_bin_dir_fn=resolve_suite_bin_dir,
            needs_suite_build_fn=needs_suite_build,
            resolve_suite_config_override_fn=verify_cmd._resolve_suite_config_override,
        )
        if test_cmd is None:
            return None
        return self._run_logged_step(
            track_name=track.name,
            track_index=track_index,
            step_name="suite_tests",
            writer=writer,
            verbose=verbose,
            action=lambda run_command_fn: run_command_fn(
                test_cmd,
                cwd=self.ctx.repo_root,
                env=self.ctx.setup_env(),
            ),
            fallback_command=_shell_join(test_cmd),
        )

    def _run_logged_step(
        self,
        *,
        track_name: str,
        track_index: int,
        step_name: str,
        writer: ValidationSummaryWriter,
        verbose: bool,
        action,
        fallback_command: str = "",
    ) -> StepResult:
        step_started_at = monotonic()
        track_segment = f"{track_index:02d}_{_safe_segment(track_name)}"
        step_segment = _safe_segment(step_name)
        step_log_path = writer.layout.tracks_dir / track_segment / f"{step_segment}.log"
        step_log_path.parent.mkdir(parents=True, exist_ok=True)

        capture = _StepCapture()
        exit_code = 0
        with step_log_path.open("w", encoding="utf-8") as log_handle:
            stdout_stream = log_handle if not verbose else _TeeStream(log_handle, sys.stdout)
            stderr_stream = log_handle if not verbose else _TeeStream(log_handle, sys.stderr)
            with redirect_stdout(stdout_stream), redirect_stderr(stderr_stream):
                try:
                    exit_code = int(action(capture.run_command))
                except Exception:
                    traceback.print_exc()
                    exit_code = 1

        command_text = " && ".join(capture.command_texts) if capture.command_texts else fallback_command
        status = "completed" if exit_code == 0 else "failed"
        key_error_lines = extract_key_error_lines_from_log(step_log_path) if exit_code != 0 else []
        step_result = StepResult(
            name=step_name,
            command=command_text,
            status=status,
            exit_code=exit_code,
            duration_ms=int((monotonic() - step_started_at) * 1000),
            log_path=_to_repo_relative(self.ctx.repo_root, step_log_path),
            key_error_lines=key_error_lines,
        )
        writer.add_step(track_name=track_name, step=step_result)
        writer.append_full_log(track_name=track_name, step_name=step_name, step_log_path=step_log_path)
        return step_result

    def _collect_track_artifacts(self, app_name: str, build_dir_name: str | None) -> dict[str, str]:
        artifacts: dict[str, str] = {}
        if build_dir_name:
            build_layout = self.ctx.get_build_layout(app_name, build_dir_name)
            artifacts["build_root"] = _to_repo_relative(self.ctx.repo_root, build_layout.root)
            artifacts["build_bin_dir"] = _to_repo_relative(self.ctx.repo_root, build_layout.bin_dir)

        result_layout = self.ctx.get_test_result_layout_for_app(app_name)
        artifacts["result_root"] = _to_repo_relative(self.ctx.repo_root, result_layout.root)
        artifacts["result_json"] = _to_repo_relative(self.ctx.repo_root, result_layout.result_json_path)
        artifacts["result_log"] = _to_repo_relative(self.ctx.repo_root, result_layout.output_log_path)
        artifacts["quality_gates_dir"] = _to_repo_relative(
            self.ctx.repo_root,
            result_layout.quality_gates_dir,
        )
        return artifacts

    def _build_failures(self, track_results: list[TrackResult]) -> list[dict]:
        failures: list[dict] = []
        for track in track_results:
            for step in track.steps:
                if step.status == "completed":
                    continue
                failures.append(
                    {
                        "track": track.name,
                        "step": step.name,
                        "exit_code": step.exit_code,
                        "command": step.command,
                        "log_path": step.log_path,
                        "key_error_lines": list(step.key_error_lines),
                    }
                )
                break
        return failures

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
        final_success = success if success is not None else all(
            track.status == "completed" for track in track_results
        )
        payload = {
            "schema_version": 1,
            "command": command_text,
            "plan_path": _to_repo_relative(self.ctx.repo_root, plan.plan_path),
            "scope_paths": list(scope_paths),
            "success": final_success,
            "exit_code": 0 if final_success else exit_code,
            "started_at": started_at_iso,
            "finished_at": _utc_now_iso(),
            "duration_ms": int((monotonic() - started_at) * 1000),
            "tracks": [asdict(track) for track in track_results],
            "failures": self._build_failures(track_results),
        }
        writer.write_summary(payload)
