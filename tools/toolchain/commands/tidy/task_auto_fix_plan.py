from __future__ import annotations

from pathlib import Path

from .autofix.models import FixContext, FixIntent
from .autofix.rules import (
    ExplicitConstructorRule,
    IdentifierNamingRule,
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
        kind=str(intent.payload.get("action_kind", intent.rule_id)),
        file_path=intent.file_path,
        line=intent.line,
        col=intent.col,
        check=intent.check,
        old_name=str(intent.payload.get("old_name", "")),
        new_name=str(intent.payload.get("new_name", "")),
        replacement=str(intent.payload.get("replacement", "")),
    )


__all__ = [
    "rename_candidates",
    "suggest_const_name",
    "supported_rename_candidate",
    "plan_redundant_cast_actions",
    "plan_runtime_int_actions",
    "plan_explicit_constructor_actions",
    "plan_using_namespace_actions",
]
