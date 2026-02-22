def split_only_tidy_command(
    command,
    app_name: str,
    parse_workers: int | None = None,
    max_lines: int | None = None,
    max_diags: int | None = None,
    batch_size: int | None = None,
    build_dir_name: str | None = None,
) -> int:
    paths = command._resolve_tidy_paths(app_name, build_dir_name=build_dir_name)
    log_path = paths["log_path"]
    tasks_dir = paths["tasks_dir"]

    if not log_path.exists():
        print(f"--- tidy-split: build log not found: {log_path}")
        print("--- tidy-split: run `tidy` first to generate build.log.")
        return 1

    try:
        split_stats, parse_seconds = command._split_from_log(
            log_path,
            tasks_dir,
            parse_workers=parse_workers,
            max_lines=max_lines,
            max_diags=max_diags,
            batch_size=batch_size,
        )
    except ValueError as error:
        print(f"--- tidy-split: invalid split settings: {error}")
        return 1

    print(
        "--- tidy-split summary: "
        f"{command._format_seconds(parse_seconds)} "
        f"(workers={split_stats['workers']}, "
        f"max_lines={split_stats['max_lines']}, "
        f"max_diags={split_stats['max_diags']}, "
        f"batch_size={split_stats['batch_size']}, "
        f"tasks={split_stats['tasks']}, batches={split_stats['batches']})"
    )
    return 0
