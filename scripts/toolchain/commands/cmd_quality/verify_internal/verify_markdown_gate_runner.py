from __future__ import annotations

import os
import sys
from pathlib import Path

from ....services.suite_registry import resolve_result_output_name


def run_report_triplet_gates(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    cli_bin: Path,
    db_path: Path,
    output_name: str,
    cases_config_path: Path,
) -> int:
    specs: tuple[tuple[str, str, str], ...] = (
        ("md", "markdown", "md"),
        ("tex", "latex", "tex"),
        ("typ", "typ", "typ"),
    )
    for format_name, export_dir_name, extension in specs:
        current_cases_dir = repo_root / "temp" / "report_triplet_cases" / format_name / "current_v1"
        golden_dir = repo_root / "test" / "golden" / "report_triplet" / format_name / "v1"
        export_root = (
            repo_root / "test" / "output" / output_name / "artifacts" / "reports" / export_dir_name
        )

        collect_cmd = [
            sys.executable,
            "scripts/toolchain/quality_gates/reporting/collect_report_triplet_cases.py",
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
            "scripts/toolchain/quality_gates/reporting/report_consistency_audit.py",
            "--left-dir",
            str(golden_dir),
            "--right-dir",
            str(current_cases_dir),
            "--pattern",
            f"*.{extension}",
            "--output",
            f"temp/report-triplet-{format_name}-byte-audit.md",
            "--fail-on-diff",
        ]
        if extension == "md":
            audit_cmd.extend(["--normalize-ext", ".md"])
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
) -> int:
    output_name = resolve_result_output_name(app_name)
    if output_name != "artifact_windows_cli":
        return 0

    app_root = repo_root / "apps" / "tracer_cli" / "windows" / "rust_cli"
    cli_bin = (
        app_root
        / build_dir_name
        / "bin"
        / ("time_tracer_cli.exe" if os.name == "nt" else "time_tracer_cli")
    )
    export_root = repo_root / "test" / "output" / output_name / "artifacts" / "reports" / "markdown"
    db_path = (
        repo_root
        / "test"
        / "output"
        / output_name
        / "workspace"
        / "output"
        / "db"
        / "time_data.sqlite3"
    )
    current_cases_dir = repo_root / "temp" / "report_markdown_cases" / "current_v1"
    golden_dir = repo_root / "test" / "golden" / "report_markdown" / "v1"
    cases_config_path = (
        repo_root / "test" / "suites" / "tracer_windows_rust_cli" / "tests" / "gate_cases.toml"
    )

    collect_cmd = [
        sys.executable,
        "scripts/toolchain/quality_gates/reporting/collect_report_markdown_cases.py",
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
        "scripts/toolchain/quality_gates/reporting/report_consistency_audit.py",
        "--left-dir",
        str(golden_dir),
        "--right-dir",
        str(current_cases_dir),
        "--pattern",
        "*.md",
        "--output",
        "temp/report-md-golden-byte-audit.md",
        "--fail-on-diff",
        "--normalize-ext",
        ".md",
    ]
    audit_ret = run_command_fn(
        audit_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )
    if audit_ret != 0:
        return audit_ret

    render_cmd = [
        sys.executable,
        "scripts/toolchain/quality_gates/reporting/report_markdown_render_snapshot_check.py",
        "--left-dir",
        str(golden_dir),
        "--right-dir",
        str(current_cases_dir),
        "--pattern",
        "*.md",
        "--output",
        "temp/report-md-golden-render-check.json",
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
