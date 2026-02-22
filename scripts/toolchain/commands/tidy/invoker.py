import time
from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command


def resolve_tidy_paths(
    ctx: Context,
    app_name: str,
    build_dir_name: str | None = None,
) -> dict[str, Path]:
    app_dir = ctx.get_app_dir(app_name)
    resolved_build_dir_name = (build_dir_name or "").strip() or "build_tidy"
    build_dir = app_dir / resolved_build_dir_name
    return {
        "build_dir": build_dir,
        "log_path": build_dir / "build.log",
        "tasks_dir": build_dir / "tasks",
        "ninja_log_path": build_dir / ".ninja_log",
    }


def ensure_configured(
    ctx: Context,
    app_name: str,
    build_dir: Path,
    build_dir_name: str | None = None,
) -> tuple[int, bool, float]:
    from ..cmd_build import BuildCommand

    if (build_dir / "CMakeCache.txt").exists():
        return 0, False, 0.0

    print(f"--- Build directory {build_dir} not configured. Running auto-configure...")
    configure_start = time.perf_counter()
    builder = BuildCommand(ctx)
    ret = builder.configure(
        app_name=app_name,
        tidy=True,
        build_dir_name=build_dir_name,
    )
    configure_seconds = time.perf_counter() - configure_start
    return ret, True, configure_seconds


def resolve_build_options(
    ctx: Context,
    extra_args: list[str] | None,
    jobs: int | None,
    keep_going: bool | None,
) -> tuple[list[str], bool, int | None, bool]:
    filtered_args = [a for a in (extra_args or []) if a != "--"]
    has_target_override = "--target" in filtered_args
    configured_jobs = ctx.config.tidy.jobs
    configured_keep_going = ctx.config.tidy.keep_going
    effective_jobs = jobs if jobs is not None else configured_jobs
    effective_keep_going = configured_keep_going if keep_going is None else keep_going
    return filtered_args, has_target_override, effective_jobs, effective_keep_going


def build_tidy_command(
    app_name: str,
    build_dir: Path,
    filtered_args: list[str],
    has_target_override: bool,
    effective_jobs: int | None,
    effective_keep_going: bool,
) -> list[str]:
    cmd = ["cmake", "--build", str(build_dir)]
    default_target = "tidy_all" if app_name == "tracer_windows_cli" else "tidy"
    if not has_target_override:
        cmd += ["--target", default_target]
    if effective_jobs and effective_jobs > 0:
        cmd += [f"-j{effective_jobs}"]
    else:
        cmd += ["-j"]
    cmd += filtered_args
    if effective_keep_going:
        # Project preference: during tidy, continue checks even when
        # individual compilation units fail.
        cmd += ["--", "-k", "0"]
    return cmd


def run_tidy_build(ctx: Context, cmd: list[str], log_path: Path) -> tuple[int, float]:
    build_start = time.perf_counter()
    ret = run_command(cmd, env=ctx.setup_env(), log_file=log_path)
    build_seconds = time.perf_counter() - build_start
    return ret, build_seconds
