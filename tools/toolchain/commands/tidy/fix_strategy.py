from __future__ import annotations

from ...core.config import TidyFixStrategyConfig

FixStrategyName = str

STRATEGY_AUTO_FIX = "auto_fix"
STRATEGY_SAFE_REFACTOR = "safe_refactor"
STRATEGY_NOLINT_ALLOWED = "nolint_allowed"
STRATEGY_MANUAL_ONLY = "manual_only"

ALL_STRATEGIES = (
    STRATEGY_AUTO_FIX,
    STRATEGY_SAFE_REFACTOR,
    STRATEGY_NOLINT_ALLOWED,
    STRATEGY_MANUAL_ONLY,
)

_PRIMARY_PRIORITY = {
    STRATEGY_MANUAL_ONLY: 4,
    STRATEGY_SAFE_REFACTOR: 3,
    STRATEGY_AUTO_FIX: 2,
    STRATEGY_NOLINT_ALLOWED: 1,
}


def _match_pattern(check_name: str, pattern: str) -> bool:
    normalized_check = check_name.strip()
    normalized_pattern = pattern.strip()
    if not normalized_check or not normalized_pattern:
        return False
    if normalized_pattern.endswith("*"):
        return normalized_check.startswith(normalized_pattern[:-1])
    return normalized_check == normalized_pattern


def _matches_any(check_name: str, patterns: list[str]) -> bool:
    for pattern in patterns:
        if _match_pattern(check_name, pattern):
            return True
    return False


def resolve_fix_strategy(check_name: str, strategy_cfg: TidyFixStrategyConfig) -> FixStrategyName:
    if _matches_any(check_name, strategy_cfg.manual_only):
        return STRATEGY_MANUAL_ONLY
    if _matches_any(check_name, strategy_cfg.auto_fix):
        return STRATEGY_AUTO_FIX
    if _matches_any(check_name, strategy_cfg.safe_refactor):
        return STRATEGY_SAFE_REFACTOR
    if _matches_any(check_name, strategy_cfg.nolint_allowed):
        return STRATEGY_NOLINT_ALLOWED
    # Conservative fallback: unknown checks are treated as manual.
    return STRATEGY_MANUAL_ONLY


def resolve_primary_strategy(
    checks: list[str], strategy_cfg: TidyFixStrategyConfig
) -> FixStrategyName:
    if not checks:
        return STRATEGY_MANUAL_ONLY
    strategies = {resolve_fix_strategy(check, strategy_cfg) for check in checks}
    return max(strategies, key=lambda item: _PRIMARY_PRIORITY.get(item, 0))
