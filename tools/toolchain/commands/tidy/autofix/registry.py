from __future__ import annotations

from .models import AutoFixRule, FixContext, FixIntent
from .rules import (
    ConcisePreprocessorDirectivesRule,
    ExplicitConstructorRule,
    IdentifierNamingRule,
    RedundantCastRule,
    RuntimeIntRule,
    UsingNamespaceRule,
)


class RuleRegistry:
    def __init__(self, rules: list[AutoFixRule]):
        self._rules = list(rules)
        self._rules_by_check: dict[str, list[AutoFixRule]] = {}
        for rule in self._rules:
            for check in rule.supported_checks:
                self._rules_by_check.setdefault(check, []).append(rule)

    def rules_for_check(self, check: str) -> list[AutoFixRule]:
        return list(self._rules_by_check.get(check, ()))

    def plan(self, context: FixContext) -> list[FixIntent]:
        planned: list[FixIntent] = []
        for diagnostic in context.parsed.diagnostics:
            for rule in self.rules_for_check(diagnostic.check):
                planned.extend(rule.plan(context, diagnostic))
        return planned


def build_default_registry() -> RuleRegistry:
    return RuleRegistry(
        [
            IdentifierNamingRule(),
            RedundantCastRule(),
            RuntimeIntRule(),
            ExplicitConstructorRule(),
            UsingNamespaceRule(),
            ConcisePreprocessorDirectivesRule(),
        ]
    )
