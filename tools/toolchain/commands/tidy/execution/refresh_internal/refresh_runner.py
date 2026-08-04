"""Helpers shared by focused source-cluster re-checks."""

from pathlib import Path

from .....core.context import Context
from .....core.executor import run_command
from ....cmd_build import BuildCommand
from ...command import TidyCommand
from ... import workspace as tidy_workspace
from ....clang.tidy import compile_db as analysis_compile_db
from ...scan import invoker as tidy_invoker


def ensure_analysis_compile_db(
    ctx: Context,
    app_name: str,
    build_dir: Path,
    build_dir_name: str,
    source_scope: str | None,
    config_file: str | None = None,
    strict_config: bool = False,
) -> int:
    if not (build_dir / "compile_commands.json").exists():
        print(
            "--- tidy-source-step: missing compile_commands.json, "
            "running tidy configure..."
        )
        ret = BuildCommand(ctx).configure(
            app_name=app_name,
            tidy=True,
            source_scope=source_scope,
            config_file=config_file,
            strict_config=strict_config,
            build_dir_name=build_dir_name,
        )
        if ret != 0:
            return ret

    workspace = tidy_workspace.resolve_workspace(
        ctx,
        build_dir_name=build_dir_name,
        source_scope=source_scope,
    )
    if workspace.prebuild_targets:
        prebuild_cmd = tidy_invoker.build_module_prereq_command(
            build_dir,
            workspace.prebuild_targets,
            tidy_invoker.resolve_effective_tidy_jobs(ctx, None, mode="task"),
        )
        prebuild_log = build_dir / "module_prereq_build.log"
        print("--- tidy-source-step: module prebuild " + ", ".join(workspace.prebuild_targets))
        ret, _ = tidy_invoker.run_tidy_build(ctx, prebuild_cmd, prebuild_log)
        if ret != 0:
            print(f"--- tidy-source-step: module prebuild failed with code {ret}")
            return ret

    try:
        analysis_compile_db.ensure_analysis_compile_db(build_dir)
    except (FileNotFoundError, OSError, ValueError) as error:
        print(f"--- tidy-source-step: failed to prepare analysis compile db: {error}")
        return 1
    return 0


def run_full_tidy(
    ctx: Context,
    app_name: str,
    jobs: int | None,
    keep_going: bool,
    concise: bool,
    source_scope: str | None,
    build_dir_name: str,
    task_view: str | None = None,
    config_file: str | None = None,
    strict_config: bool = False,
) -> int:
    return TidyCommand(ctx).execute(
        app_name=app_name,
        extra_args=[],
        jobs=jobs,
        concise=concise,
        keep_going=keep_going,
        source_scope=source_scope,
        build_dir_name=build_dir_name,
        task_view=task_view,
        config_file=config_file,
        strict_config=strict_config,
    )
