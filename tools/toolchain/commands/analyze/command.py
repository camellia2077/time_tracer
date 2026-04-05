from __future__ import annotations

import subprocess

from ...core.context import Context
from .orchestrator import execute_analyze_command, split_only_analyze_command
from .report_service import build_summary, merge_sarif_reports
from .unit_runner import (
    build_analyzer_command,
    format_progress_file,
    is_excluded_analyze_file,
    parse_compile_command,
    resolve_entry_file,
    strip_build_only_flags,
    is_under_any_root,
)


# Backward-compatible helper names imported by existing tests/callers.

def _format_progress_file(repo_root, file_path):
    return format_progress_file(repo_root, file_path)


def _parse_compile_command(entry: dict):
    return parse_compile_command(entry)


def _resolve_entry_file(entry: dict):
    return resolve_entry_file(entry)


def _is_under_any_root(file_path, roots):
    return is_under_any_root(file_path, roots)


def _is_excluded_analyze_file(file_path, repo_root):
    return is_excluded_analyze_file(file_path, repo_root)


def _strip_build_only_flags(args: list[str]):
    return strip_build_only_flags(args)


class AnalyzeCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        *,
        jobs: int | None = None,
        source_scope: str | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> int:
        return execute_analyze_command(
            ctx=self.ctx,
            app_name=app_name,
            jobs=jobs,
            source_scope=source_scope,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            run_subprocess_fn=subprocess.run,
        )

    def split_only(
        self,
        app_name: str,
        *,
        source_scope: str | None = None,
        build_dir_name: str | None = None,
        batch_size: int | None = None,
    ) -> int:
        return split_only_analyze_command(
            ctx=self.ctx,
            app_name=app_name,
            source_scope=source_scope,
            build_dir_name=build_dir_name,
            batch_size=batch_size,
        )
