from __future__ import annotations

import os
import sys
from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command
from ...core.generated_paths import resolve_build_layout, resolve_test_result_layout
from ...services.suite_registry import resolve_result_output_name
from .refresh_golden import RefreshGoldenCommand

REPORTING_GOLDEN_DB_SNAPSHOT_NAME = "reporting_golden_db.sqlite3"


class ReportMarkdownGateCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def _sync_and_print(
        self,
        *,
        current_dir: Path,
        golden_dir: Path,
        pattern: str,
        scope_name: str,
    ) -> None:
        sync_stats = RefreshGoldenCommand(self.ctx)._sync_snapshot_dir(
            current_dir,
            golden_dir,
            pattern,
        )
        print(
            f"--- report-markdown-gate: refreshed {scope_name} "
            f"(added={len(sync_stats.added)}, updated={len(sync_stats.updated)}, removed={len(sync_stats.removed)})"
        )

    def execute(
        self,
        *,
        app_name: str,
        build_dir_name: str | None,
        refresh_golden: bool,
        cases_config_path: str | None = None,
    ) -> int:
        if app_name not in {"tracer_core", "tracer_core_shell"}:
            print(
                "Error: report-markdown-gate currently supports "
                "--app tracer_core / --app tracer_core_shell only."
            )
            return 2

        resolved_build_dir = (build_dir_name or "").strip() or "build_fast"
        output_name = resolve_result_output_name(app_name)
        if output_name != "artifact_windows_cli":
            print(f"Error: unsupported output mapping for app `{app_name}`: {output_name}")
            return 2

        repo_root = self.ctx.repo_root
        cli_name = "time_tracer_cli.exe" if os.name == "nt" else "time_tracer_cli"
        cli_bin = resolve_build_layout(
            repo_root,
            "tracer_windows_rust_cli",
            resolved_build_dir,
        ).bin_dir / cli_name
        result_layout = resolve_test_result_layout(repo_root, output_name)
        snapshot_db_path = (
            result_layout.workspace_dir
            / "output"
            / "db_snapshots"
            / REPORTING_GOLDEN_DB_SNAPSHOT_NAME
        )
        # Keep manual gate runs aligned with verify:
        # prefer pre-exchange reporting snapshot to avoid DB drift after
        # `exchange import ... (replace all)` fixtures (for example config-refresh.tracer).
        db_path = (
            snapshot_db_path
            if snapshot_db_path.is_file()
            else result_layout.workspace_dir / "output" / "db" / "time_data.sqlite3"
        )
        export_root = result_layout.artifacts_dir / "reports" / "markdown"
        quality_gates_root = result_layout.quality_gates_dir
        current_cases_dir = quality_gates_root / "report_markdown_cases" / "current_v1"
        golden_dir = repo_root / "test" / "golden" / "report_markdown" / "v1"
        render_check_output = (
            quality_gates_root / "audits" / "report-md-golden-render-check.json"
        )
        resolved_cases_config = (
            Path(cases_config_path)
            if cases_config_path
            else repo_root
            / "test"
            / "suites"
            / "tracer_windows_rust_cli"
            / "tests"
            / "gate_cases.toml"
        )
        if not resolved_cases_config.is_absolute():
            resolved_cases_config = (repo_root / resolved_cases_config).resolve()

        if not cli_bin.is_file():
            print(
                f"Error: CLI binary not found: {cli_bin}\n"
                "Hint: build tracer_windows_rust_cli or rerun the artifact suite first."
            )
            return 2
        if not db_path.is_file():
            print(
                f"Error: database not found: {db_path}\n"
                "Hint: rerun artifact_windows_cli suite or full verify first."
            )
            return 2
        if not export_root.is_dir():
            print(
                f"Error: markdown export root not found: {export_root}\n"
                "Hint: rerun artifact_windows_cli suite or full verify first."
            )
            return 2

        env = self.ctx.setup_env()
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
            str(resolved_cases_config),
            "--strict-text-policy",
        ]
        collect_ret = run_command(collect_cmd, cwd=repo_root, env=env)
        if collect_ret != 0:
            return int(collect_ret)

        if refresh_golden:
            self._sync_and_print(
                current_dir=current_cases_dir,
                golden_dir=golden_dir,
                pattern="*.md",
                scope_name="report_markdown/v1",
            )

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
        render_ret = int(run_command(render_cmd, cwd=repo_root, env=env))
        if render_ret != 0:
            return render_ret

        triplet_specs = (
            ("tex", "latex", "*.tex"),
            ("typ", "typ", "*.typ"),
        )
        for fmt, export_dir_name, pattern in triplet_specs:
            current_dir = quality_gates_root / "report_triplet_cases" / fmt / "current_v1"
            triplet_golden_dir = repo_root / "test" / "golden" / "report_triplet" / fmt / "v1"
            export_dir = result_layout.artifacts_dir / "reports" / export_dir_name
            triplet_audit_output = (
                quality_gates_root / "audits" / f"report-triplet-{fmt}-byte-audit.md"
            )

            collect_triplet_cmd = [
                sys.executable,
                "tools/toolchain/quality_gates/reporting/collect_report_triplet_cases.py",
                "--format",
                fmt,
                "--export-root",
                str(export_dir),
                "--output-dir",
                str(current_dir),
                "--cli-bin",
                str(cli_bin),
                "--db-path",
                str(db_path),
                "--cases-config",
                str(resolved_cases_config),
                "--strict-text-policy",
            ]
            collect_triplet_ret = int(run_command(collect_triplet_cmd, cwd=repo_root, env=env))
            if collect_triplet_ret != 0:
                return collect_triplet_ret

            if refresh_golden:
                self._sync_and_print(
                    current_dir=current_dir,
                    golden_dir=triplet_golden_dir,
                    pattern=pattern,
                    scope_name=f"report_triplet/{fmt}/v1",
                )

            triplet_audit_cmd = [
                sys.executable,
                "tools/toolchain/quality_gates/reporting/report_consistency_audit.py",
                "--left-dir",
                str(triplet_golden_dir),
                "--right-dir",
                str(current_dir),
                "--pattern",
                pattern,
                "--output",
                str(triplet_audit_output),
                "--fail-on-diff",
            ]
            triplet_audit_ret = int(run_command(triplet_audit_cmd, cwd=repo_root, env=env))
            if triplet_audit_ret != 0:
                return triplet_audit_ret

        return 0
