from unittest import TestCase

from tools.toolchain.commands.tidy.fix_strategy import (
    STRATEGY_MANUAL_ONLY,
    STRATEGY_SAFE_REFACTOR,
    resolve_fix_strategy,
)
from tools.toolchain.core.config import TidyFixStrategyConfig


class TestTidyFixStrategy(TestCase):
    def test_agent_owned_refactors_are_not_manual_only(self):
        config = TidyFixStrategyConfig()

        self.assertEqual(
            resolve_fix_strategy("readability-identifier-naming", config),
            STRATEGY_SAFE_REFACTOR,
        )
        self.assertEqual(
            resolve_fix_strategy("bugprone-narrowing-conversions", config),
            STRATEGY_SAFE_REFACTOR,
        )
        self.assertEqual(
            resolve_fix_strategy("clang-analyzer-core.NullDereference", config),
            STRATEGY_MANUAL_ONLY,
        )
