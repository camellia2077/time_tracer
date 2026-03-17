import time


def execute_tidy_command(
    command,
    app_name: str,
    extra_args: list[str] | None = None,
    jobs: int | None = None,
    parse_workers: int | None = None,
    keep_going: bool | None = None,
    source_scope: str | None = None,
    build_dir_name: str | None = None,
    task_view: str = "text",
    prebuild_targets: list[str] | None = None,
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
        source_scope=source_scope,
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
    resolved_prebuild_targets = [target for target in (prebuild_targets or []) if str(target).strip()]
    if resolved_prebuild_targets:
        prebuild_log_path = build_dir / "module_prereq_build.log"
        prebuild_cmd = command._build_module_prereq_command(
            build_dir,
            resolved_prebuild_targets,
            effective_jobs,
        )
        print(
            "--- Tidy module prebuild: "
            + ", ".join(resolved_prebuild_targets)
        )
        prebuild_ret, _ = command._run_tidy_build(prebuild_cmd, prebuild_log_path)
        if prebuild_ret != 0:
            print(f"--- Tidy module prebuild failed with code {prebuild_ret}.")
            return prebuild_ret

    try:
        command._ensure_analysis_compile_db(build_dir)
    except (FileNotFoundError, OSError, ValueError) as error:
        print(f"--- Failed to prepare analysis compile db: {error}")
        return 1

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
                task_view=task_view,
                workspace_name=build_dir_name or "",
                source_scope=source_scope,
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
