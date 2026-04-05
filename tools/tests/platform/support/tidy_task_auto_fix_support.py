from __future__ import annotations

import json
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from types import SimpleNamespace

from tools.toolchain.commands.tidy.tasking.task_log import parse_task_log
from tools.toolchain.commands.tidy.tasking.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSummary,
    TaskSummaryEntry,
    task_record_to_dict,
)
from tools.toolchain.commands.tidy.workspace import ResolvedTidyWorkspace
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


@dataclass(frozen=True, slots=True)
class DiagnosticEntry:
    line: int
    col: int
    check: str
    message: str
    severity: str = "warning"
    raw_lines: tuple[str, ...] = ()
    notes: tuple[str, ...] = ()


def build_test_context(
    *,
    build_tidy_dir: Path,
    tasks_dir: Path,
    automation_dir: Path,
) -> Context:
    ctx = Context(REPO_ROOT)
    ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
        root=build_tidy_dir,
        tasks_dir=tasks_dir,
        automation_dir=automation_dir,
        tasks_done_dir=build_tidy_dir / "tasks_done",
        batch_state_path=build_tidy_dir / "batch_state.json",
    )
    return ctx


def build_test_workspace(build_dir_name: str = "build_tidy_test") -> ResolvedTidyWorkspace:
    return ResolvedTidyWorkspace(
        source_scope="core_family",
        build_dir_name=build_dir_name,
        source_roots=[],
        prebuild_targets=[],
    )


class AutoFixFixtureBuilder:
    def __init__(
        self,
        root: Path,
        *,
        app_name: str = "tracer_core_shell",
        build_dir_name: str = "build_tidy_test",
        source_scope: str = "core_family",
    ) -> None:
        self.root = Path(root)
        self.app_name = app_name
        self.build_dir_name = build_dir_name
        self.source_scope = source_scope
        self.build_tidy_dir = (
            self.root / "out" / "tidy" / self.app_name / self.build_dir_name
        )
        self.tasks_dir = self.build_tidy_dir / "tasks"
        self.automation_dir = self.build_tidy_dir / "automation"
        self.tasks_dir.mkdir(parents=True, exist_ok=True)
        self.automation_dir.mkdir(parents=True, exist_ok=True)

    def context(self) -> Context:
        return build_test_context(
            build_tidy_dir=self.build_tidy_dir,
            tasks_dir=self.tasks_dir,
            automation_dir=self.automation_dir,
        )

    def workspace(self) -> ResolvedTidyWorkspace:
        return build_test_workspace(self.build_dir_name)

    def write_source(self, relative_path: str, lines: list[str]) -> Path:
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("\n".join(lines) + "\n", encoding="utf-8")
        return path

    def write_text(self, relative_path: str, lines: list[str]) -> Path:
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("\n".join(lines), encoding="utf-8")
        return path

    def write_legacy_task_log(
        self,
        *,
        relative_path: str,
        source_file: Path,
        diagnostics: list[DiagnosticEntry],
    ) -> Path:
        entries: list[str] = [
            f"File: {source_file}",
            "============================================================",
        ]
        for item in diagnostics:
            entries.append(
                f"{source_file}:{item.line}:{item.col}: {item.severity}: {item.message} [{item.check}]"
            )
        return self.write_text(relative_path, entries)

    def write_toon_task(
        self,
        *,
        relative_path: str,
        task_id: str,
        batch_id: str,
        source_file: Path,
        diagnostics: list[DiagnosticEntry],
    ) -> Path:
        check_counts = Counter(item.check for item in diagnostics)
        lines = [
            "task:",
            f"  id: {task_id}",
            f"  batch: {batch_id}",
            f"  source: {source_file}",
            "summary:",
            f"  diagnostics: {len(diagnostics)}",
            "  compiler_errors: false",
            f"checks[{len(check_counts)}]{{name,count}}:",
        ]
        for check_name, count in check_counts.items():
            lines.append(f"  {check_name},{count}")
        lines.append(
            f"diagnostics[{len(diagnostics)}]{{index,line,col,severity,check,message}}:"
        )
        for index, item in enumerate(diagnostics, start=1):
            lines.append(
                f"  {index},{item.line},{item.col},{item.severity},{item.check},{item.message}"
            )
        return self.write_text(relative_path, lines)

    def write_batch_toon_task(
        self,
        *,
        batch_id: str,
        task_id: str,
        source_file: Path,
        diagnostics: list[DiagnosticEntry],
    ) -> Path:
        return self.write_toon_task(
            relative_path=(
                f"out/tidy/{self.app_name}/{self.build_dir_name}/tasks/{batch_id}/task_{task_id}.toon"
            ),
            task_id=task_id,
            batch_id=batch_id,
            source_file=source_file,
            diagnostics=diagnostics,
        )

    def write_batch_task_artifacts(
        self,
        *,
        batch_id: str,
        task_id: str,
        source_file: Path,
        diagnostics: list[DiagnosticEntry],
    ) -> tuple[Path, Path]:
        task_path = self.write_batch_toon_task(
            batch_id=batch_id,
            task_id=task_id,
            source_file=source_file,
            diagnostics=diagnostics,
        )
        json_path = self.write_task_json(
            task_path=task_path,
            task_id=task_id,
            batch_id=batch_id,
            source_file=source_file,
            diagnostics=diagnostics,
        )
        return task_path, json_path

    def write_task_json(
        self,
        *,
        task_path: Path,
        task_id: str,
        batch_id: str,
        source_file: Path,
        diagnostics: list[DiagnosticEntry],
    ) -> Path:
        files_summary = Counter(str(source_file) for _ in diagnostics)
        checks_summary = Counter(item.check for item in diagnostics)

        record = TaskRecord(
            version=3,
            task_id=task_id,
            batch_id=batch_id,
            queue_generation=None,
            source_file=str(source_file),
            source_fingerprint=None,
            workspace=self.build_dir_name,
            source_scope=self.source_scope,
            checks=tuple(checks_summary.keys()),
            summary=TaskSummary(
                diagnostic_count=len(diagnostics),
                compiler_errors=False,
                files=tuple(
                    TaskSummaryEntry(name=name, count=count)
                    for name, count in files_summary.items()
                ),
                checks=tuple(
                    TaskSummaryEntry(name=name, count=count)
                    for name, count in checks_summary.items()
                ),
            ),
            diagnostics=tuple(
                TaskDiagnostic(
                    file=str(source_file),
                    line=item.line,
                    col=item.col,
                    severity=item.severity,
                    check=item.check,
                    message=item.message,
                    raw_lines=(
                        item.raw_lines
                        if item.raw_lines
                        else (
                            f"{source_file}:{item.line}:{item.col}: {item.severity}: {item.message} [{item.check}]",
                        )
                    ),
                    notes=item.notes,
                )
                for item in diagnostics
            ),
            snippets=(),
            raw_lines=(),
        )
        json_path = task_path.with_suffix(".json")
        json_path.parent.mkdir(parents=True, exist_ok=True)
        json_path.write_text(
            json.dumps(task_record_to_dict(record), indent=2),
            encoding="utf-8",
        )
        return json_path

    @staticmethod
    def parse(task_path: Path):
        return parse_task_log(task_path)
