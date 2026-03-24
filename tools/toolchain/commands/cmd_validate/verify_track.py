from __future__ import annotations

from time import monotonic

from ...core.generated_paths import resolve_test_result_layout_for_app
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
from .step_runner import run_logged_step
from .summary import TrackResult, to_repo_relative


def collect_track_artifacts(ctx, app_name: str, build_dir_name: str | None) -> dict[str, str]:
    artifacts: dict[str, str] = {}
    if build_dir_name:
        build_layout = ctx.get_build_layout(app_name, build_dir_name)
        artifacts["build_root"] = to_repo_relative(ctx.repo_root, build_layout.root)
        artifacts["build_bin_dir"] = to_repo_relative(ctx.repo_root, build_layout.bin_dir)

    result_layout = ctx.get_test_result_layout_for_app(app_name)
    artifacts["result_root"] = to_repo_relative(ctx.repo_root, result_layout.root)
    artifacts["result_json"] = to_repo_relative(ctx.repo_root, result_layout.result_json_path)
    artifacts["result_log"] = to_repo_relative(ctx.repo_root, result_layout.output_log_path)
    artifacts["quality_gates_dir"] = to_repo_relative(
        ctx.repo_root,
        result_layout.quality_gates_dir,
    )
    return artifacts


def run_suite_step(
    *,
    ctx,
    track,
    track_index: int,
    writer,
    verbose: bool,
    build_dir_name: str,
    verify_cmd: VerifyCommand,
):
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
    return run_logged_step(
        repo_root=ctx.repo_root,
        track_name=track.name,
        track_index=track_index,
        step_name="suite_tests",
        writer=writer,
        verbose=verbose,
        action=lambda run_command_fn: run_command_fn(
            test_cmd,
            cwd=ctx.repo_root,
            env=ctx.setup_env(),
        ),
        fallback_command=" ".join(str(part) for part in test_cmd),
    )


def execute_verify_track(
    *,
    ctx,
    track,
    track_index: int,
    writer,
    verbose: bool,
    started_at: float,
) -> TrackResult:
    verify_cmd = VerifyCommand(ctx)
    builder = BuildCommand(ctx)
    steps = []
    suite_name = resolve_suite_name(track.app)

    build_step = run_logged_step(
        repo_root=ctx.repo_root,
        track_name=track.name,
        track_index=track_index,
        step_name="build",
        writer=writer,
        verbose=verbose,
        action=lambda run_command_fn: execute_build_stage(
            ctx=ctx,
            build_command_cls=BuildCommand,
            app_name=track.app,
            tidy=False,
            extra_args=None,
            cmake_args=track.cmake_args,
            build_dir_name=track.build_dir,
            profile_name=track.profile,
            concise=track.concise,
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
            repo_root=ctx.repo_root,
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
            artifacts=collect_track_artifacts(ctx, track.app, resolved_build_dir),
        )

    unit_step = run_logged_step(
        repo_root=ctx.repo_root,
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
            artifacts=collect_track_artifacts(ctx, track.app, resolved_build_dir),
        )

    suite_step = run_suite_step(
        ctx=ctx,
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
                artifacts=collect_track_artifacts(ctx, track.app, resolved_build_dir),
            )

    markdown_step = run_logged_step(
        repo_root=ctx.repo_root,
        track_name=track.name,
        track_index=track_index,
        step_name="markdown_gates",
        writer=writer,
        verbose=verbose,
        action=lambda run_command_fn: run_report_markdown_gates(
            repo_root=ctx.repo_root,
            setup_env_fn=ctx.setup_env,
            run_command_fn=run_command_fn,
            app_name=track.app,
            build_dir_name=resolved_build_dir,
            profile_name=track.profile,
            normalize_ext=tuple(ctx.config.quality.gate_audit.normalize_ext),
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
            artifacts=collect_track_artifacts(ctx, track.app, resolved_build_dir),
        )

    native_step = run_logged_step(
        repo_root=ctx.repo_root,
        track_name=track.name,
        track_index=track_index,
        step_name="native_runtime",
        writer=writer,
        verbose=verbose,
        action=lambda run_command_fn: run_native_core_runtime_tests(
            repo_root=ctx.repo_root,
            setup_env_fn=ctx.setup_env,
            run_command_fn=run_command_fn,
            app_name=track.app,
            build_dir_name=resolved_build_dir,
            profile_name=track.profile,
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
            artifacts=collect_track_artifacts(ctx, track.app, resolved_build_dir),
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
        artifacts=collect_track_artifacts(ctx, track.app, resolved_build_dir),
    )
