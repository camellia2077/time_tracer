from __future__ import annotations

from . import task_auto_fix_report
from .autofix import run_task_auto_fix_orchestrator
from .autofix.analyzers import detect_task_refactors
from .task_auto_fix_apply import (
    allowed_roots,
    analysis_compile_db_dir,
    apply_explicit_constructor_actions,
    apply_redundant_casts,
    apply_runtime_int_actions,
    apply_using_namespace_actions,
    rename_action,
)
from .task_auto_fix_plan import (
    plan_explicit_constructor_actions,
    plan_redundant_cast_actions,
    plan_runtime_int_actions,
    plan_using_namespace_actions,
    rename_candidates,
    suggest_const_name,
)
from .task_auto_fix_types import AutoFixAction, TaskAutoFixResult


def run_task_auto_fix(
    ctx,
    *,
    task_log_path: str,
    dry_run: bool = False,
    report_suffix: str = "fix",
) -> TaskAutoFixResult:
    return run_task_auto_fix_orchestrator(
        ctx,
        task_log_path=task_log_path,
        dry_run=dry_run,
        report_suffix=report_suffix,
    )


def suggest_task_refactors(parsed) -> list[dict]:
    return task_auto_fix_report.suggest_task_refactors(
        parsed,
        detect_task_refactors_fn=detect_task_refactors,
    )


def write_task_suggestion_report(**kwargs):
    return task_auto_fix_report.write_task_suggestion_report(**kwargs)


__all__ = [
    "AutoFixAction",
    "TaskAutoFixResult",
    "apply_explicit_constructor_actions",
    "apply_redundant_casts",
    "apply_runtime_int_actions",
    "apply_using_namespace_actions",
    "analysis_compile_db_dir",
    "allowed_roots",
    "plan_using_namespace_actions",
    "plan_redundant_cast_actions",
    "suggest_const_name",
    "plan_explicit_constructor_actions",
    "plan_runtime_int_actions",
    "rename_action",
    "rename_candidates",
    "run_task_auto_fix",
    "suggest_task_refactors",
    "write_task_suggestion_report",
    "detect_task_refactors",
]
