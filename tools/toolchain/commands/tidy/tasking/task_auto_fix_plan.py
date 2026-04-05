from __future__ import annotations

from pathlib import Path

from ..autofix.models import (
    FixContext,
    FixIntent,
    operation_new_name,
    operation_old_name,
    operation_replacement,
)
from ..autofix.rules import (
    ConcisePreprocessorDirectivesRule,
    ExplicitConstructorRule,
    RedundantCastRule,
    RuntimeIntRule,
    UsingNamespaceRule,
    extract_rename_candidates,
    suggest_const_name,
    supported_rename_candidate,
)
from .task_auto_fix_types import AutoFixAction


def rename_candidates(parsed) -> list[dict]:
    return extract_rename_candidates(parsed)


def plan_redundant_cast_actions(parsed) -> list[AutoFixAction]:
    return _plan_rule_actions(parsed, RedundantCastRule())


def plan_runtime_int_actions(parsed) -> list[AutoFixAction]:
    return _plan_rule_actions(parsed, RuntimeIntRule())


def plan_explicit_constructor_actions(parsed) -> list[AutoFixAction]:
    return _plan_rule_actions(parsed, ExplicitConstructorRule())


def plan_using_namespace_actions(parsed) -> list[AutoFixAction]:
    return _plan_rule_actions(parsed, UsingNamespaceRule())


def plan_concise_preprocessor_actions(parsed) -> list[AutoFixAction]:
    return _plan_rule_actions(parsed, ConcisePreprocessorDirectivesRule())


def _plan_rule_actions(parsed, rule) -> list[AutoFixAction]:
    context = FixContext(
        ctx=None,
        app_name="",
        workspace=None,
        parsed=parsed,
        build_tidy_dir=Path("."),
        dry_run=True,
    )
    intents: list[FixIntent] = []
    for diagnostic in parsed.diagnostics:
        if diagnostic.check not in rule.supported_checks:
            continue
        intents.extend(rule.plan(context, diagnostic))
    return [_intent_to_action(intent) for intent in intents]


def _intent_to_action(intent: FixIntent) -> AutoFixAction:
    return AutoFixAction(
        action_id=intent.intent_id,
        kind=intent.action_kind,
        file_path=intent.file_path,
        line=intent.line,
        col=intent.col,
        check=intent.check,
        rule_id=intent.rule_id,
        risk_level=intent.risk_level,
        preview_only=intent.preview_only,
        old_name=operation_old_name(intent.operation),
        new_name=operation_new_name(intent.operation),
        replacement=operation_replacement(intent.operation),
    )


__all__ = [
    "rename_candidates",
    "suggest_const_name",
    "supported_rename_candidate",
    "plan_redundant_cast_actions",
    "plan_runtime_int_actions",
    "plan_explicit_constructor_actions",
    "plan_using_namespace_actions",
    "plan_concise_preprocessor_actions",
]
