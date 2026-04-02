from __future__ import annotations

import os
import sys
from pathlib import Path

from ....core.generated_paths import resolve_build_layout, resolve_test_result_layout
from ....services.suite_registry import resolve_result_output_name
from .verify_profile_policy import should_run_reporting_markdown_gates


def run_report_triplet_gates(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    cli_bin: Path,
    db_path: Path,
    output_name: str,
    cases_config_path: Path,
) -> int:
    result_layout = resolve_test_result_layout(repo_root, output_name)
    quality_gates_root = result_layout.quality_gates_dir
    specs: tuple[tuple[str, str, str], ...] = (
        ("tex", "latex", "tex"),
        ("typ", "typ", "typ"),
    )
    for format_name, export_dir_name, extension in specs:
        current_cases_dir = (
            quality_gates_root / "report_triplet_cases" / format_name / "current_v1"
        )
        golden_dir = repo_root / "test" / "golden" / "report_triplet" / format_name / "v1"
        export_root = result_layout.artifacts_dir / "reports" / export_dir_name
        audit_output_path = (
            quality_gates_root / "audits" / f"report-triplet-{format_name}-byte-audit.md"
        )

        collect_cmd = [
            sys.executable,
            "tools/toolchain/quality_gates/reporting/collect_report_triplet_cases.py",
            "--format",
            format_name,
            "--export-root",
            str(export_root),
            "--output-dir",
            str(current_cases_dir),
            "--cli-bin",
            str(cli_bin),
            "--db-path",
            str(db_path),
            "--cases-config",
            str(cases_config_path),
            "--strict-text-policy",
        ]
        collect_ret = run_command_fn(
            collect_cmd,
            cwd=repo_root,
            env=setup_env_fn(),
        )
        if collect_ret != 0:
            return collect_ret

        audit_cmd = [
            sys.executable,
            "tools/toolchain/quality_gates/reporting/report_consistency_audit.py",
            "--left-dir",
            str(golden_dir),
            "--right-dir",
            str(current_cases_dir),
            "--pattern",
            f"*.{extension}",
            "--output",
            str(audit_output_path),
            "--fail-on-diff",
        ]
        audit_ret = run_command_fn(
            audit_cmd,
            cwd=repo_root,
            env=setup_env_fn(),
        )
        if audit_ret != 0:
            return audit_ret
    return 0


def run_report_markdown_gates(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    app_name: str,
    build_dir_name: str,
    profile_name: str | None = None,
    normalize_ext: tuple[str, ...] = (".md",),
) -> int:
    output_name = resolve_result_output_name(app_name)
    if output_name != "artifact_windows_cli":
        return 0
    if not should_run_reporting_markdown_gates(profile_name):
        return 0

    result_layout = resolve_test_result_layout(repo_root, output_name)
    quality_gates_root = result_layout.quality_gates_dir
    cli_bin = resolve_build_layout(
        repo_root,
        "tracer_windows_rust_cli",
        build_dir_name,
    ).bin_dir / ("time_tracer_cli.exe" if os.name == "nt" else "time_tracer_cli")
    export_root = result_layout.artifacts_dir / "reports" / "markdown"
    db_path = result_layout.workspace_dir / "output" / "db" / "time_data.sqlite3"
    current_cases_dir = quality_gates_root / "report_markdown_cases" / "current_v1"
    golden_dir = repo_root / "test" / "golden" / "report_markdown" / "v1"
    render_check_output = quality_gates_root / "audits" / "report-md-golden-render-check.json"
    cases_config_path = (
        repo_root / "test" / "suites" / "tracer_windows_rust_cli" / "tests" / "gate_cases.toml"
    )

    collect_cmd = [
        sys.executable,
        "tools/toolchain/quality_gates/reporting/collect_report_markdown_cases.py",
        "--export-root",
        str(export_root),
        "--output-dir",
        str(current_cases_dir),
        "--cli-bin",
        str(cli_bin),
        "--db-path",
        str(db_path),
        "--cases-config",
        str(cases_config_path),
        "--strict-text-policy",
    ]
    collect_ret = run_command_fn(
        collect_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )
    if collect_ret != 0:
        return collect_ret

    render_cmd = [
        sys.executable,
        "tools/toolchain/quality_gates/reporting/report_markdown_render_snapshot_check.py",
        "--left-dir",
        str(golden_dir),
        "--right-dir",
        str(current_cases_dir),
        "--pattern",
        "*.md",
        "--output",
        str(render_check_output),
        "--fail-on-structure-diff",
    ]
    render_ret = run_command_fn(
        render_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )
    if render_ret != 0:
        return render_ret

    return run_report_triplet_gates(
        repo_root=repo_root,
        setup_env_fn=setup_env_fn,
        run_command_fn=run_command_fn,
        cli_bin=cli_bin,
        db_path=db_path,
        output_name=output_name,
        cases_config_path=cases_config_path,
    )
