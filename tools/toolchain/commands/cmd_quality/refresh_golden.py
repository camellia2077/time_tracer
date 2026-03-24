from __future__ import annotations

import hashlib
import os
import re
from dataclasses import dataclass
from pathlib import Path

from ...core.context import Context
from ...core.generated_paths import resolve_build_layout, resolve_test_result_layout
from ...core.executor import run_command
from ...services.suite_registry import resolve_result_output_name
from .verify import VerifyCommand


@dataclass
class SyncStats:
    added: list[str]
    updated: list[str]
    removed: list[str]


class RefreshGoldenCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    @staticmethod
    def _sha256(path: Path) -> str:
        return hashlib.sha256(path.read_bytes()).hexdigest()

    def _sync_snapshot_dir(self, src: Path, dst: Path, pattern: str) -> SyncStats:
        dst.mkdir(parents=True, exist_ok=True)
        src_map = {
            p.name: p
            for p in src.glob(pattern)
            if p.is_file()
        }
        dst_map = {
            p.name: p
            for p in dst.glob(pattern)
            if p.is_file()
        }

        added: list[str] = []
        updated: list[str] = []
        removed: list[str] = []

        for name, src_path in src_map.items():
            dst_path = dst / name
            if not dst_path.exists():
                dst_path.write_bytes(src_path.read_bytes())
                added.append(name)
                continue
            if self._sha256(src_path) != self._sha256(dst_path):
                dst_path.write_bytes(src_path.read_bytes())
                updated.append(name)

        for name, dst_path in dst_map.items():
            if name in src_map:
                continue
            dst_path.unlink()
            removed.append(name)

        return SyncStats(added=added, updated=updated, removed=removed)

    def _run_collect_and_audit(
        self,
        *,
        cli_bin: Path,
        db_path: Path,
        cases_config_path: Path,
        output_name: str,
    ) -> tuple[int, list[tuple[str, SyncStats]]]:
        repo_root = self.ctx.repo_root
        env = self.ctx.setup_env()

        result_layout = resolve_test_result_layout(repo_root, output_name)
        quality_gates_root = result_layout.quality_gates_dir
        markdown_export_root = result_layout.artifacts_dir / "reports" / "markdown"
        markdown_current = quality_gates_root / "report_markdown_cases" / "current_v1"
        markdown_golden = repo_root / "test" / "golden" / "report_markdown" / "v1"
        markdown_audit_output = quality_gates_root / "audits" / "report-md-golden-byte-audit.md"

        collect_markdown_cmd = [
            os.sys.executable,
            "tools/toolchain/quality_gates/reporting/collect_report_markdown_cases.py",
            "--export-root",
            str(markdown_export_root),
            "--output-dir",
            str(markdown_current),
            "--cli-bin",
            str(cli_bin),
            "--db-path",
            str(db_path),
            "--cases-config",
            str(cases_config_path),
            "--strict-text-policy",
        ]
        if run_command(collect_markdown_cmd, cwd=repo_root, env=env) != 0:
            return 1, []

        changes: list[tuple[str, SyncStats]] = []
        changes.append(
            ("report_markdown/v1", self._sync_snapshot_dir(markdown_current, markdown_golden, "*.md"))
        )

        normalize_ext = tuple(self.ctx.config.quality.gate_audit.normalize_ext)
        audit_markdown_cmd = [
            os.sys.executable,
            "tools/toolchain/quality_gates/reporting/report_consistency_audit.py",
            "--left-dir",
            str(markdown_golden),
            "--right-dir",
            str(markdown_current),
            "--pattern",
            "*.md",
            "--output",
            str(markdown_audit_output),
            "--fail-on-diff",
        ]
        if normalize_ext:
            audit_markdown_cmd.extend(["--normalize-ext", ",".join(normalize_ext)])
        if run_command(audit_markdown_cmd, cwd=repo_root, env=env) != 0:
            return 1, changes

        triplet_specs = (
            ("md", "markdown", "*.md"),
            ("tex", "latex", "*.tex"),
            ("typ", "typ", "*.typ"),
        )
        for fmt, export_dir_name, pattern in triplet_specs:
            current_dir = quality_gates_root / "report_triplet_cases" / fmt / "current_v1"
            golden_dir = repo_root / "test" / "golden" / "report_triplet" / fmt / "v1"
            export_root = result_layout.artifacts_dir / "reports" / export_dir_name
            audit_output = quality_gates_root / "audits" / f"report-triplet-{fmt}-byte-audit.md"

            collect_cmd = [
                os.sys.executable,
                "tools/toolchain/quality_gates/reporting/collect_report_triplet_cases.py",
                "--format",
                fmt,
                "--export-root",
                str(export_root),
                "--output-dir",
                str(current_dir),
                "--cli-bin",
                str(cli_bin),
                "--db-path",
                str(db_path),
                "--cases-config",
                str(cases_config_path),
                "--strict-text-policy",
            ]
            if run_command(collect_cmd, cwd=repo_root, env=env) != 0:
                return 1, changes

            changes.append(
                (
                    f"report_triplet/{fmt}/v1",
                    self._sync_snapshot_dir(current_dir, golden_dir, pattern),
                )
            )

            audit_cmd = [
                os.sys.executable,
                "tools/toolchain/quality_gates/reporting/report_consistency_audit.py",
                "--left-dir",
                str(golden_dir),
                "--right-dir",
                str(current_dir),
                "--pattern",
                pattern,
                "--output",
                str(audit_output),
                "--fail-on-diff",
            ]
            if fmt == "md" and normalize_ext:
                audit_cmd.extend(["--normalize-ext", ",".join(normalize_ext)])
            if run_command(audit_cmd, cwd=repo_root, env=env) != 0:
                return 1, changes

        return 0, changes

    def _write_summary(
        self,
        changes: list[tuple[str, SyncStats]],
        output_name: str,
    ) -> Path:
        summary_path = (
            resolve_test_result_layout(self.ctx.repo_root, output_name).quality_gates_dir
            / "report-golden-refresh-summary.md"
        )
        lines: list[str] = ["# Golden Refresh Summary", ""]
        for scope, stats in changes:
            lines.append(f"## {scope}")
            lines.append(f"- added: {len(stats.added)}")
            lines.append(f"- updated: {len(stats.updated)}")
            lines.append(f"- removed: {len(stats.removed)}")
            if stats.added:
                lines.append("")
                lines.append("added files:")
                lines.extend([f"- {name}" for name in stats.added])
            if stats.updated:
                lines.append("")
                lines.append("updated files:")
                lines.extend([f"- {name}" for name in stats.updated])
            if stats.removed:
                lines.append("")
                lines.append("removed files:")
                lines.extend([f"- {name}" for name in stats.removed])
            lines.append("")
        summary_path.parent.mkdir(parents=True, exist_ok=True)
        summary_path.write_text("\n".join(lines), encoding="utf-8")
        return summary_path

    @staticmethod
    def _replace_line(text: str, key: str, value: str) -> str:
        pattern = re.compile(rf"^{re.escape(key)}\s*=\s*\".*\"$", flags=re.MULTILINE)
        replacement = f'{key} = "{value}"'
        if pattern.search(text):
            return pattern.sub(replacement, text, count=1)
        suffix = "" if text.endswith("\n") else "\n"
        return text + suffix + replacement + "\n"

    def _update_gate_cases(
        self,
        *,
        cases_config_path: Path,
        recent_range: str | None,
        range_argument: str | None,
    ) -> None:
        if not recent_range and not range_argument:
            return
        text = cases_config_path.read_text(encoding="utf-8")
        if range_argument:
            text = self._replace_line(text, "range_argument", range_argument)
            if "|" in range_argument:
                start, end = [part.strip() for part in range_argument.split("|", 1)]
                if start and end:
                    text = self._replace_line(
                        text,
                        "range_case_name",
                        f"range_{start}_{end}.md",
                    )
        if recent_range:
            text = self._replace_line(text, "recent_range_argument", recent_range)
            if "|" in recent_range:
                start, end = [part.strip() for part in recent_range.split("|", 1)]
                if start and end:
                    text = self._replace_line(
                        text,
                        "recent_case_name",
                        f"recent_fixed_window_{start}_{end}.md",
                    )
        cases_config_path.write_text(text, encoding="utf-8")
        print(f"--- refresh-golden: updated gate cases config: {cases_config_path}")

    def execute(
        self,
        *,
        app_name: str,
        build_dir_name: str | None,
        profile_name: str | None,
        concise: bool,
        skip_verify: bool,
        recent_range: str | None,
        range_argument: str | None,
    ) -> int:
        if app_name not in {"tracer_core", "tracer_core_shell"}:
            print(
                "Error: refresh-golden currently supports "
                "--app tracer_core / --app tracer_core_shell only."
            )
            return 2

        resolved_build_dir = (build_dir_name or "").strip() or "build_fast"
        if not skip_verify:
            verify_ret = VerifyCommand(self.ctx).execute(
                app_name=app_name,
                build_dir_name=resolved_build_dir,
                profile_name=profile_name,
                concise=concise,
            )
            if verify_ret != 0:
                return verify_ret

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
        db_path = (
            resolve_test_result_layout(repo_root, output_name).workspace_dir
            / "output"
            / "db"
            / "time_data.sqlite3"
        )
        cases_config_path = (
            repo_root / "test" / "suites" / "tracer_windows_rust_cli" / "tests" / "gate_cases.toml"
        )
        self._update_gate_cases(
            cases_config_path=cases_config_path,
            recent_range=recent_range,
            range_argument=range_argument,
        )

        ret, changes = self._run_collect_and_audit(
            cli_bin=cli_bin,
            db_path=db_path,
            cases_config_path=cases_config_path,
            output_name=output_name,
        )
        if ret != 0:
            return ret

        summary_path = self._write_summary(changes, output_name)
        print(f"--- refresh-golden: summary written to {summary_path}")
        return 0
