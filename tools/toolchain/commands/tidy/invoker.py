import os
import time
from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command
from . import analysis_compile_db, clang_tidy_config, workspace as tidy_workspace

_TIDY_HEADER_FILTER_CACHE_KEY = "TT_CLANG_TIDY_HEADER_FILTER"
_AUTO_FULL_TIDY_JOB_CAP = 6
_AUTO_TASK_BATCH_JOB_CAP = 8


def resolve_effective_tidy_jobs(
    ctx: Context,
    jobs: int | None,
    *,
    mode: str,
) -> int:
    if mode not in {"full", "task_batch"}:
        raise ValueError(f"unsupported tidy job mode: {mode}")

    configured_jobs = (
        ctx.config.tidy.jobs_full
        if mode == "full"
        else ctx.config.tidy.jobs_task_batch
    )
    requested_jobs = configured_jobs if jobs is None else jobs
    if requested_jobs > 0:
        return requested_jobs

    cpu_count = os.cpu_count() or 1
    if mode == "full":
        # jobs=0 now means auto-throttled parallelism, not bare Ninja `-j`.
        # Full tidy fans out many clang-tidy custom commands, so bounded
        # concurrency avoids the long-tail memory/IO stalls that made
        # tidy-close --stabilize look hung after stdout went quiet.
        return max(1, min(_AUTO_FULL_TIDY_JOB_CAP, max(1, cpu_count // 3)))

    # Task/batch helpers work on smaller scopes than full tidy, so they can use
    # a slightly wider bounded fan-out without reintroducing unbounded builds.
    return max(1, min(_AUTO_TASK_BATCH_JOB_CAP, max(1, cpu_count // 2)))


def _resolve_tidy_header_filter_regex(ctx: Context) -> str:
    configured = (ctx.config.tidy.header_filter_regex or "").strip()
    if configured:
        return configured
    return r"^(?!.*[\\/]_deps[\\/]).*"


def _read_cmake_cache_value(cache_path: Path, key: str) -> str | None:
    try:
        for raw_line in cache_path.read_text(encoding="utf-8", errors="replace").splitlines():
            if not raw_line.startswith(f"{key}:"):
                continue
            _, _, value = raw_line.partition("=")
            return value.strip()
    except OSError:
        return None
    return None


def resolve_tidy_paths(
    ctx: Context,
    app_name: str,
    build_dir_name: str | None = None,
) -> dict[str, Path]:
    resolved_build_dir_name = (
        (build_dir_name or "").strip() or tidy_workspace.DEFAULT_TIDY_BUILD_DIR_NAME
    )
    tidy_layout = ctx.get_tidy_layout(app_name, resolved_build_dir_name)
    build_dir = tidy_layout.root
    return {
        "build_dir": build_dir,
        "log_path": build_dir / "build.log",
        "tasks_dir": tidy_layout.tasks_dir,
        "ninja_log_path": build_dir / ".ninja_log",
    }


def ensure_configured(
    ctx: Context,
    app_name: str,
    build_dir: Path,
    source_scope: str | None = None,
    config_file: str | None = None,
    strict_config: bool = False,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    concise: bool = False,
    log_path: Path | None = None,
) -> tuple[int, bool, float]:
    from ..cmd_build import BuildCommand

    cache_path = build_dir / "CMakeCache.txt"
    expected_filter = _resolve_tidy_header_filter_regex(ctx)
    expected_scope, expected_roots = tidy_workspace.source_scope_cache_values(ctx, source_scope)
    expected_targets = tidy_workspace.source_scope_cache_targets_value(ctx, source_scope)
    expected_config_file = clang_tidy_config.resolve_config_cache_value(
        ctx,
        config_file=config_file,
        strict_config=strict_config,
    )
    if cache_path.exists():
        current_filter = _read_cmake_cache_value(cache_path, _TIDY_HEADER_FILTER_CACHE_KEY)
        current_scope = (
            _read_cmake_cache_value(cache_path, tidy_workspace.CMAKE_CACHE_KEY_SOURCE_SCOPE) or ""
        ).strip()
        current_roots = tidy_workspace.normalize_cache_roots_value(
            _read_cmake_cache_value(cache_path, tidy_workspace.CMAKE_CACHE_KEY_SOURCE_ROOTS)
        )
        current_targets = tidy_workspace.normalize_cache_targets_value(
            _read_cmake_cache_value(cache_path, tidy_workspace.CMAKE_CACHE_KEY_SOURCE_TARGETS)
        )
        current_config_file = (
            _read_cmake_cache_value(cache_path, clang_tidy_config.CMAKE_CACHE_KEY_CONFIG_FILE)
            or ""
        ).strip()
        current_compile_db_dir = (
            _read_cmake_cache_value(
                cache_path,
                analysis_compile_db.CMAKE_CACHE_KEY_ANALYSIS_COMPILE_DB_DIR,
            )
            or ""
        ).strip()
        expected_compile_db_dir = analysis_compile_db.resolve_compile_db_cache_value(build_dir)
        if (
            current_filter == expected_filter
            and current_scope == expected_scope
            and current_roots == expected_roots
            and current_targets == expected_targets
            and current_config_file.replace("\\", "/") == expected_config_file
            and current_compile_db_dir.replace("\\", "/") == expected_compile_db_dir
        ):
            return 0, False, 0.0
        print(
            "--- Build directory "
            f"{build_dir} tidy header filter is stale "
            f"(cache keys={_TIDY_HEADER_FILTER_CACHE_KEY}, "
            f"{tidy_workspace.CMAKE_CACHE_KEY_SOURCE_SCOPE}, "
            f"{tidy_workspace.CMAKE_CACHE_KEY_SOURCE_ROOTS}, "
            f"{tidy_workspace.CMAKE_CACHE_KEY_SOURCE_TARGETS}, "
            f"{clang_tidy_config.CMAKE_CACHE_KEY_CONFIG_FILE}, "
            f"{analysis_compile_db.CMAKE_CACHE_KEY_ANALYSIS_COMPILE_DB_DIR}). "
            "Running auto-configure..."
        )
    else:
        print(f"--- Build directory {build_dir} not configured. Running auto-configure...")

    configure_start = time.perf_counter()
    builder = BuildCommand(ctx)
    ret = builder.configure(
        app_name=app_name,
        tidy=True,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        concise=concise,
    )
    configure_seconds = time.perf_counter() - configure_start
    return ret, True, configure_seconds


def resolve_build_options(
    ctx: Context,
    extra_args: list[str] | None,
    jobs: int | None,
    keep_going: bool | None,
    *,
    job_mode: str = "full",
) -> tuple[list[str], bool, int | None, bool]:
    filtered_args = [a for a in (extra_args or []) if a != "--"]
    has_target_override = "--target" in filtered_args
    configured_keep_going = ctx.config.tidy.keep_going
    effective_jobs = resolve_effective_tidy_jobs(ctx, jobs, mode=job_mode)
    effective_keep_going = configured_keep_going if keep_going is None else keep_going
    return filtered_args, has_target_override, effective_jobs, effective_keep_going


def build_tidy_command(
    _app_name: str,
    build_dir: Path,
    filtered_args: list[str],
    has_target_override: bool,
    effective_jobs: int | None,
    effective_keep_going: bool,
) -> list[str]:
    cmd = ["cmake", "--build", str(build_dir)]
    default_target = "tidy"
    if not has_target_override:
        cmd += ["--target", default_target]
    if effective_jobs and effective_jobs > 0:
        cmd += [f"-j{effective_jobs}"]
    else:
        # Keep tidy fan-out deterministic. Bare `-j` lets Ninja choose an
        # effectively unbounded worker count, which is too aggressive for
        # clang-tidy-heavy full runs on Windows.
        cmd += ["-j1"]
    cmd += filtered_args
    if effective_keep_going:
        # Project preference: during tidy, continue checks even when
        # individual compilation units fail.
        cmd += ["--", "-k", "0"]
    return cmd


def build_module_prereq_command(
    build_dir: Path,
    prebuild_targets: list[str],
    effective_jobs: int | None,
) -> list[str]:
    cmd = ["cmake", "--build", str(build_dir), "--target", *prebuild_targets]
    if effective_jobs and effective_jobs > 0:
        cmd += [f"-j{effective_jobs}"]
    else:
        cmd += ["-j"]
    return cmd


def run_tidy_build(
    ctx: Context,
    cmd: list[str],
    log_path: Path,
    *,
    output_mode: str = "live",
) -> tuple[int, float]:
    build_start = time.perf_counter()
    ret = run_command(
        cmd,
        env=ctx.setup_env(),
        log_file=log_path,
        output_mode=output_mode,
    )
    build_seconds = time.perf_counter() - build_start
    return ret, build_seconds
