import json
from copy import deepcopy
from pathlib import Path

from ...services import batch_state


def _append_unique_reasons(target: list[str], extra: list[str]) -> None:
    for reason in extra:
        if reason not in target:
            target.append(reason)


def _filter_auto_reasons(
    reasons: list[str],
    *,
    allow_no_such_file: bool,
    allow_glob_mismatch: bool,
) -> list[str]:
    filtered: list[str] = []
    for reason in reasons:
        if reason == "auto_no_such_file" and not allow_no_such_file:
            continue
        if reason == "auto_glob_mismatch" and not allow_glob_mismatch:
            continue
        filtered.append(reason)
    return filtered


def _rename_report_mtime_ns(build_dir: Path) -> int | None:
    report_path = build_dir / "rename" / "rename_apply_report.json"
    if not report_path.exists():
        return None
    try:
        return report_path.stat().st_mtime_ns
    except OSError:
        return None


def _high_already_renamed_summary(
    build_dir: Path,
    *,
    ratio_threshold: float,
    min_count: int,
) -> str | None:
    report_path = build_dir / "rename" / "rename_apply_report.json"
    if not report_path.exists():
        return None
    try:
        payload = json.loads(report_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    results = payload.get("results", [])
    if not isinstance(results, list):
        return None

    total = 0
    already_renamed = 0
    for item in results:
        if not isinstance(item, dict):
            continue
        total += 1
        if item.get("reason") == "already_renamed":
            already_renamed += 1
    if total <= 0:
        return None
    ratio = already_renamed / total
    if already_renamed < max(1, min_count):
        return None
    if ratio < ratio_threshold:
        return None
    return f"{already_renamed}/{total} ({ratio:.1%})"


def execute_refresh_command(
    command,
    app_name: str,
    batch_id: str | None = None,
    full_every: int = 3,
    neighbor_scope: str = "none",
    jobs: int | None = None,
    parse_workers: int | None = None,
    keep_going: bool | None = None,
    force_full: bool = False,
    final_full: bool = False,
    build_dir_name: str | None = None,
    dry_run: bool = False,
) -> int:
    resolved_build_dir_name = (build_dir_name or "").strip() or "build_tidy"
    app_dir = command.ctx.get_app_dir(app_name)
    build_dir = app_dir / resolved_build_dir_name
    tasks_done_dir = build_dir / "tasks_done"
    state_path = build_dir / "refresh_state.json"
    compile_commands_path = build_dir / "compile_commands.json"

    normalized_batch = None
    if batch_id:
        try:
            normalized_batch = command._normalize_batch_name(batch_id)
        except ValueError as exc:
            print(f"--- tidy-refresh: {exc}")
            return 2

    if not normalized_batch and not force_full and not final_full:
        print(
            "--- tidy-refresh: --batch-id is required unless "
            "--force-full or --final-full is provided."
        )
        return 2

    state = command._load_state(state_path)
    next_state = deepcopy(state)
    command._ensure_processed_batches(next_state)
    tidy_cfg = command.ctx.config.tidy

    should_run_incremental = bool(normalized_batch)
    touched_files: list[Path] = []
    incremental_files: list[Path] = []
    build_log_mtime_ns = command._build_log_mtime_ns(build_dir)
    last_seen_build_log_mtime_ns_raw = state.get("last_seen_build_log_mtime_ns")
    try:
        last_seen_build_log_mtime_ns = (
            int(last_seen_build_log_mtime_ns_raw)
            if last_seen_build_log_mtime_ns_raw is not None
            else None
        )
    except (TypeError, ValueError):
        last_seen_build_log_mtime_ns = None
    rename_report_mtime_ns = _rename_report_mtime_ns(build_dir)
    last_seen_rename_report_mtime_ns_raw = state.get("last_seen_rename_report_mtime_ns")
    try:
        last_seen_rename_report_mtime_ns = (
            int(last_seen_rename_report_mtime_ns_raw)
            if last_seen_rename_report_mtime_ns_raw is not None
            else None
        )
    except (TypeError, ValueError):
        last_seen_rename_report_mtime_ns = None

    if should_run_incremental:
        batch_dir = tasks_done_dir / normalized_batch
        if not batch_dir.exists():
            print(f"--- tidy-refresh: batch directory not found: {batch_dir}")
            return 1

        touched_files = command._collect_batch_files(batch_dir)
        if not touched_files:
            print(
                "--- tidy-refresh: no files found in batch logs, "
                f"skip incremental refresh ({normalized_batch})."
            )
        else:
            ensure_ret = command._ensure_compile_commands(
                app_name=app_name,
                build_dir=build_dir,
                build_dir_name=resolved_build_dir_name,
            )
            if ensure_ret != 0:
                return ensure_ret
            if not compile_commands_path.exists():
                print(f"--- tidy-refresh: missing compile commands: {compile_commands_path}")
                return 1
            compile_units = command._load_compile_units(compile_commands_path)
            incremental_files = command._resolve_incremental_files(
                touched_files=touched_files,
                compile_units=compile_units,
                app_dir=app_dir,
                neighbor_scope=neighbor_scope,
            )

    already_processed_batch = command._register_batch(next_state, normalized_batch)
    if normalized_batch and already_processed_batch:
        print(
            f"--- tidy-refresh: batch {normalized_batch} already counted in "
            "refresh_state.json; cadence counter will not increase."
        )

    cadence_is_due = command._cadence_due(
        state=next_state,
        batch_name=normalized_batch,
        already_processed=already_processed_batch,
        full_every=full_every,
    )
    full_reasons = command._resolve_full_reasons(
        force_full=force_full,
        final_full=final_full,
        cadence_is_due=cadence_is_due,
        full_every=full_every,
    )
    has_new_build_log = (
        build_log_mtime_ns is not None and build_log_mtime_ns != last_seen_build_log_mtime_ns
    )
    if has_new_build_log:
        build_log_auto_reasons = command._detect_build_log_auto_reasons(build_dir)
        filtered_build_log_auto_reasons = _filter_auto_reasons(
            build_log_auto_reasons,
            allow_no_such_file=tidy_cfg.auto_full_on_no_such_file,
            allow_glob_mismatch=tidy_cfg.auto_full_on_glob_mismatch,
        )
        if filtered_build_log_auto_reasons:
            _append_unique_reasons(full_reasons, filtered_build_log_auto_reasons)
            print(
                "--- tidy-refresh: auto full tidy trigger from build log "
                f"(reasons={','.join(filtered_build_log_auto_reasons)})."
            )
    has_new_rename_report = (
        rename_report_mtime_ns is not None
        and rename_report_mtime_ns != last_seen_rename_report_mtime_ns
    )
    if has_new_rename_report and tidy_cfg.auto_full_on_high_already_renamed:
        summary = _high_already_renamed_summary(
            build_dir,
            ratio_threshold=float(tidy_cfg.auto_full_already_renamed_ratio),
            min_count=int(tidy_cfg.auto_full_already_renamed_min),
        )
        if summary:
            _append_unique_reasons(full_reasons, ["auto_high_already_renamed"])
            print(
                "--- tidy-refresh: detected high `already_renamed`, "
                f"auto switching to full tidy ({summary})."
            )

    should_run_full = bool(full_reasons)
    effective_keep_going = command.ctx.config.tidy.keep_going if keep_going is None else keep_going

    if dry_run:
        command._print_preview(
            batch_name=normalized_batch,
            touched_files=touched_files,
            incremental_files=incremental_files,
            neighbor_scope=neighbor_scope,
            full_every=full_every,
            cadence_due=cadence_is_due,
            full_reasons=full_reasons,
            keep_going=effective_keep_going,
        )
        return 0

    if should_run_incremental:
        incremental_ret = command._run_incremental_tidy(
            build_dir=build_dir,
            batch_name=normalized_batch,
            files=incremental_files,
            keep_going=effective_keep_going,
        )
        if incremental_ret != 0:
            incremental_auto_reasons = command._detect_incremental_auto_reasons(
                build_dir=build_dir,
                batch_name=normalized_batch,
            )
            filtered_incremental_auto_reasons = _filter_auto_reasons(
                incremental_auto_reasons,
                allow_no_such_file=tidy_cfg.auto_full_on_no_such_file,
                allow_glob_mismatch=tidy_cfg.auto_full_on_glob_mismatch,
            )
            if not filtered_incremental_auto_reasons:
                return incremental_ret
            _append_unique_reasons(full_reasons, filtered_incremental_auto_reasons)
            should_run_full = True
            print(
                "--- tidy-refresh: incremental anomalies detected, "
                "auto switching to full tidy "
                f"(reasons={','.join(filtered_incremental_auto_reasons)})."
            )

    if should_run_full:
        print(f"--- tidy-refresh: running full tidy (reason={','.join(full_reasons)}).")
        full_ret = command._run_full_tidy(
            app_name=app_name,
            jobs=jobs,
            parse_workers=parse_workers,
            keep_going=effective_keep_going,
            build_dir_name=resolved_build_dir_name,
        )
        if full_ret != 0:
            return full_ret
        next_state["batches_since_full"] = 0
        next_state["last_full_reason"] = ",".join(full_reasons)
        next_state["last_full_batch"] = normalized_batch
        next_state["last_full_at"] = command._utc_now_iso()

    next_state["full_every"] = full_every
    next_state["last_seen_build_log_mtime_ns"] = command._build_log_mtime_ns(build_dir)
    next_state["last_seen_rename_report_mtime_ns"] = _rename_report_mtime_ns(build_dir)
    next_state["updated_at"] = command._utc_now_iso()
    command._write_state(state_path, next_state)
    print(f"--- tidy-refresh: state updated -> {state_path}")
    batch_state_path = batch_state.update_state(
        ctx=command.ctx,
        app_name=app_name,
        batch_id=normalized_batch,
        last_refresh_ok=True,
        extra_fields={"last_refresh_build_dir": resolved_build_dir_name},
    )
    print(f"--- tidy-refresh: batch state updated -> {batch_state_path}")
    return 0
