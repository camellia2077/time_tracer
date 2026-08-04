from pathlib import Path

from tools.toolchain.commands.tidy.queue.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSnippet,
    TaskSummary,
    TaskSummaryEntry,
)

REPO_ROOT = Path(__file__).resolve().parents[4]


def _make_task_record(
    *,
    task_id: str = "001",
    cluster_id: str = "cluster_fixture",
    source_file: str,
    diagnostics: tuple[TaskDiagnostic, ...],
    snippets: tuple[TaskSnippet, ...] = (),
    checks: tuple[str, ...] = ("clang-diagnostic-error",),
) -> TaskRecord:
    summary = TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=any(item.severity == "error" for item in diagnostics),
        files=(TaskSummaryEntry(name=source_file, count=max(1, len(diagnostics))),),
        checks=tuple(TaskSummaryEntry(name=check, count=1) for check in checks),
    )
    return TaskRecord(
        version=4,
        task_id=task_id,
        cluster_id=cluster_id,
        scan_id="scan_fixture",
        queue_generation=None,
        source_file=source_file,
        source_fingerprint=None,
        workspace="build_tidy_core_family",
        source_scope="core_family",
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=snippets,
        raw_lines=tuple(line for diagnostic in diagnostics for line in diagnostic.raw_lines),
    )
