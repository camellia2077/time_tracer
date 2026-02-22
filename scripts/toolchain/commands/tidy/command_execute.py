import time


def execute_tidy_command(
    command,
    app_name: str,
    extra_args: list[str] | None = None,
    jobs: int | None = None,
    parse_workers: int | None = None,
    keep_going: bool | None = None,
    build_dir_name: str | None = None,
) -> int:
    paths = command._resolve_tidy_paths(app_name, build_dir_name=build_dir_name)
    build_dir = paths["build_dir"]
    log_path = paths["log_path"]
    tasks_dir = paths["tasks_dir"]
    ninja_log_path = paths["ninja_log_path"]
    overall_start = time.perf_counter()
    configure_seconds = 0.0
    build_seconds = 0.0
    parse_seconds = 0.0
    split_stats = None

    ret, did_auto_configure, configure_seconds = command._ensure_configured(
        app_name=app_name,
        build_dir=build_dir,
        build_dir_name=build_dir_name,
    )
    if ret != 0:
        print("--- Auto-configure failed. Aborting Tidy.")
        return ret

    (
        filtered_args,
        has_target_override,
        effective_jobs,
        effective_keep_going,
    ) = command._resolve_build_options(extra_args, jobs, keep_going)
    cmd = command._build_tidy_command(
        app_name,
        build_dir,
        filtered_args,
        has_target_override,
        effective_jobs,
        effective_keep_going,
    )
    ret, build_seconds = command._run_tidy_build(cmd, log_path)
    if ret != 0:
        print(f"--- Tidy build finished with code {ret}. Processing logs anyway...")

    if log_path.exists():
        try:
            split_stats, parse_seconds = command._split_from_log(
                log_path,
                tasks_dir,
                parse_workers=parse_workers,
            )
        except ValueError as error:
            print(f"--- Tidy log split failed: {error}")
            return 1

    ninja_stats = command._read_ninja_timing(ninja_log_path)
    total_seconds = time.perf_counter() - overall_start
    command._print_timing_summary(
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
