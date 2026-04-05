from __future__ import annotations

import json
import time
from pathlib import Path

from ...core.context import Context
from ..cmd_build import BuildCommand
from ..tidy import invoker as tidy_invoker
from .split import DEFAULT_ANALYZE_BATCH_SIZE, split_sarif_report
from . import workspace as analyze_workspace
from .report_service import build_summary, merge_sarif_reports
from .unit_runner import collect_matched_entries, run_analysis_units


def execute_analyze_command(
    *,
    ctx: Context,
    app_name: str,
    jobs: int | None = None,
    source_scope: str | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    run_subprocess_fn,
) -> int:
    workspace = analyze_workspace.resolve_workspace(
        ctx,
        build_dir_name=build_dir_name,
        source_scope=source_scope,
    )
    analyze_layout = ctx.get_analyze_layout(app_name, workspace.build_dir_name)
    build_dir = ctx.get_build_dir(app_name, workspace.build_dir_name)
    analyze_layout.root.mkdir(parents=True, exist_ok=True)
    analyze_layout.reports_dir.mkdir(parents=True, exist_ok=True)
    analyze_layout.unit_reports_dir.mkdir(parents=True, exist_ok=True)

    overall_start = time.perf_counter()
    configure_seconds = 0.0
    did_auto_configure = False
    cache_path = build_dir / "CMakeCache.txt"
    if not cache_path.exists():
        print(f"--- Analyze build directory {build_dir} not configured. Running auto-configure...")
        configure_start = time.perf_counter()
        builder = BuildCommand(ctx)
        ret = builder.configure(
            app_name=app_name,
            tidy=False,
            source_scope=workspace.source_scope,
            build_dir_name=workspace.build_dir_name,
            profile_name=profile_name,
        )
        configure_seconds = time.perf_counter() - configure_start
        did_auto_configure = True
        if ret != 0:
            print("--- Auto-configure failed. Aborting analyze.")
            return ret

    filtered_jobs = jobs if jobs is not None else 0
    if workspace.prebuild_targets:
        prebuild_log_path = build_dir / "module_prereq_build.log"
        prebuild_cmd = tidy_invoker.build_module_prereq_command(
            build_dir,
            workspace.prebuild_targets,
            filtered_jobs if filtered_jobs > 0 else None,
        )
        print("--- Analyze module prebuild: " + ", ".join(workspace.prebuild_targets))
        prebuild_ret, _ = tidy_invoker.run_tidy_build(ctx, prebuild_cmd, prebuild_log_path)
        if prebuild_ret != 0:
            print(f"--- Analyze module prebuild failed with code {prebuild_ret}.")
            return prebuild_ret

    compile_commands_path = build_dir / "compile_commands.json"
    if not compile_commands_path.exists():
        print(f"--- Missing compile_commands.json: {compile_commands_path}")
        return 1

    payload = json.loads(compile_commands_path.read_text(encoding="utf-8"))
    if not isinstance(payload, list):
        print(f"--- Invalid compile_commands payload: {compile_commands_path}")
        return 1

    matched_entries = collect_matched_entries(
        payload,
        repo_root=ctx.repo_root,
        source_roots=workspace.source_roots,
    )

    analyze_layout.output_log_path.write_text("", encoding="utf-8")

    env = ctx.setup_env()
    report_paths, failed_units, analyzed_units = run_analysis_units(
        repo_root=ctx.repo_root,
        matched_entries=matched_entries,
        unit_reports_dir=analyze_layout.unit_reports_dir,
        output_log_path=analyze_layout.output_log_path,
        env=env,
        build_dir=build_dir,
        run_subprocess_fn=run_subprocess_fn,
    )

    merged_sarif = merge_sarif_reports(report_paths)
    analyze_layout.raw_report_path.write_text(
        json.dumps(merged_sarif, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    summary = build_summary(
        workspace_name=workspace.build_dir_name,
        source_scope=workspace.source_scope,
        build_dir=build_dir,
        raw_report_path=analyze_layout.raw_report_path,
        log_path=analyze_layout.output_log_path,
        matched_units=len(matched_entries),
        analyzed_units=analyzed_units,
        failed_units=failed_units,
        merged_sarif=merged_sarif,
    )
    analyze_layout.summary_json_path.write_text(
        json.dumps(summary, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    auto_split_stats: dict[str, object] | None = None
    if not failed_units:
        auto_split_stats = split_sarif_report(
            raw_report_path=analyze_layout.raw_report_path,
            issues_dir=analyze_layout.issues_dir,
            summary_path=analyze_layout.summary_json_path,
            workspace_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            batch_size=DEFAULT_ANALYZE_BATCH_SIZE,
        )

    total_seconds = time.perf_counter() - overall_start
    print(f"--- Analyze workspace: {analyze_layout.root}")
    print(f"--- Analyze units matched: {len(matched_entries)}")
    print(f"--- Analyze units completed: {analyzed_units}")
    print(f"--- Analyze units failed: {len(failed_units)}")
    print(f"--- Analyze results: {summary['totals']['results']}")
    print(f"--- Analyze raw SARIF: {analyze_layout.raw_report_path}")
    print(f"--- Analyze summary: {analyze_layout.summary_json_path}")
    if auto_split_stats is not None:
        print(
            "--- Analyze split summary: "
            f"issues={auto_split_stats['issues']}, "
            f"batches={auto_split_stats['batches']}, "
            f"batch_size={auto_split_stats['batch_size']}, "
            f"issues_dir={analyze_layout.issues_dir}"
        )
    print(
        "--- Analyze timing: "
        f"configure={configure_seconds:.2f}s "
        f"total={total_seconds:.2f}s "
        f"auto_configure={'yes' if did_auto_configure else 'no'}"
    )
    return 0 if not failed_units else 1


def split_only_analyze_command(
    *,
    ctx: Context,
    app_name: str,
    source_scope: str | None = None,
    build_dir_name: str | None = None,
    batch_size: int | None = None,
) -> int:
    workspace = analyze_workspace.resolve_workspace(
        ctx,
        build_dir_name=build_dir_name,
        source_scope=source_scope,
    )
    analyze_layout = ctx.get_analyze_layout(app_name, workspace.build_dir_name)
    raw_report_path = analyze_layout.raw_report_path
    if not raw_report_path.exists():
        print(f"--- analyze-split: raw SARIF not found: {raw_report_path}")
        print("--- analyze-split: run `analyze` first to generate reports/run.sarif.")
        return 1

    effective_batch_size = DEFAULT_ANALYZE_BATCH_SIZE if batch_size is None else batch_size
    try:
        split_stats = split_sarif_report(
            raw_report_path=raw_report_path,
            issues_dir=analyze_layout.issues_dir,
            summary_path=analyze_layout.summary_json_path,
            workspace_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            batch_size=effective_batch_size,
        )
    except ValueError as error:
        print(f"--- analyze-split: invalid split settings: {error}")
        return 1

    print(
        "--- analyze-split summary: "
        f"issues={split_stats['issues']}, "
        f"batches={split_stats['batches']}, "
        f"batch_size={split_stats['batch_size']}, "
        f"issues_dir={analyze_layout.issues_dir}"
    )
    return 0
