import time

from ...core.context import Context
from ...core.executor import kill_build_processes
from ..clang.tidy import compile_db as analysis_compile_db
from .scan import (
    invoker as tidy_invoker,
    structured_results as tidy_structured_results,
    timing as tidy_timing,
)


def execute_tidy_command(
    ctx: Context,
    app_name: str,
    extra_args: list[str] | None = None,
    jobs: int | None = None,
    concise: bool = False,
    profile_name: str | None = None,
    kill_build_procs: bool = False,
    keep_going: bool | None = None,
    source_scope: str | None = None,
    build_dir_name: str | None = None,
    task_view: str | None = None,
    prebuild_targets: list[str] | None = None,
    config_file: str | None = None,
    strict_config: bool = False,
) -> int:
    paths = tidy_invoker.resolve_tidy_paths(ctx, app_name, build_dir_name=build_dir_name)
    build_dir = paths["build_dir"]
    log_path = paths["log_path"]
    tasks_dir = paths["tasks_dir"]
    structured_results_dir = paths.get(
        "structured_results_dir",
        build_dir / "structured_tidy_results",
    )
    ninja_log_path = paths["ninja_log_path"]
    output_mode = "quiet" if concise else "live"
    overall_start = time.perf_counter()
    configure_seconds = 0.0
    build_seconds = 0.0
    parse_seconds = 0.0
    split_stats = None

    structured_results_dir.mkdir(parents=True, exist_ok=True)
    for result_path in structured_results_dir.glob("*.json"):
        result_path.unlink()

    if kill_build_procs:
        kill_build_processes()

    ret, did_auto_configure, configure_seconds = tidy_invoker.ensure_configured(
        ctx,
        app_name=app_name,
        build_dir=build_dir,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        concise=concise,
        log_path=log_path,
    )
    if ret != 0:
        print("--- Auto-configure failed. Aborting Tidy.")
        return ret

    (
        filtered_args,
        has_target_override,
        effective_jobs,
        effective_keep_going,
    ) = tidy_invoker.resolve_build_options(
        ctx,
        extra_args,
        jobs,
        keep_going,
        job_mode="full",
    )
    resolved_prebuild_targets = [target for target in (prebuild_targets or []) if str(target).strip()]
    if resolved_prebuild_targets:
        prebuild_log_path = log_path
        prebuild_cmd = tidy_invoker.build_module_prereq_command(
            build_dir,
            resolved_prebuild_targets,
            effective_jobs,
        )
        print(
            "--- Tidy module prebuild: "
            + ", ".join(resolved_prebuild_targets)
        )
        prebuild_ret, _ = tidy_invoker.run_tidy_build(
            ctx,
            prebuild_cmd,
            prebuild_log_path,
            output_mode=output_mode,
        )
        if prebuild_ret != 0:
            print(f"--- Tidy module prebuild failed with code {prebuild_ret}.")
            return prebuild_ret

    try:
        compile_db_dir = analysis_compile_db.ensure_analysis_compile_db(build_dir)
    except (FileNotFoundError, OSError, ValueError) as error:
        print(f"--- Failed to prepare analysis compile db: {error}")
        return 1

    cmd = tidy_invoker.build_tidy_command(
        app_name,
        build_dir,
        filtered_args,
        has_target_override,
        effective_jobs,
        effective_keep_going,
    )
    ret, build_seconds = tidy_invoker.run_tidy_build(
        ctx,
        cmd,
        log_path,
        output_mode=output_mode,
    )
    if ret != 0:
        print(f"--- Tidy build finished with code {ret}.")

    structured_check_results = sorted(structured_results_dir.glob("check_*.json"))
    if structured_check_results:
        try:
            split_stats, parse_seconds = tidy_structured_results.collect_structured_results(
                ctx,
                structured_results_dir,
                tasks_dir,
                task_view=task_view,
                workspace_name=build_dir_name or "",
                source_scope=source_scope,
            )
            print(
                f"--- Tidy structured results: {len(structured_check_results)} invocation(s)"
            )
        except (OSError, ValueError) as error:
            print(f"--- Tidy structured result collection failed: {error}")
            return ret if ret != 0 else 1
    elif not structured_check_results:
        print("--- Structured clang-tidy results are unavailable.")
        print("--- Re-run tidy after restoring the structured clang-tidy wrapper.")
        return ret if ret != 0 else 1

    ninja_stats = tidy_timing.read_ninja_timing(ninja_log_path)
    total_seconds = time.perf_counter() - overall_start
    tidy_timing.print_timing_summary(
        did_auto_configure=did_auto_configure,
        configure_seconds=configure_seconds,
        build_seconds=build_seconds,
        parse_seconds=parse_seconds,
        total_seconds=total_seconds,
        split_stats=split_stats,
        ninja_stats=ninja_stats,
        jobs=effective_jobs,
    )

    return ret
