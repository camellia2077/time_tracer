from unittest import TestCase

from tools.toolchain.commands.tidy.autofix.rules import (
    CONCISE_PREPROCESSOR_METADATA,
    EXPLICIT_CONSTRUCTOR_METADATA,
    IDENTIFIER_NAMING_METADATA,
    REDUNDANT_CAST_METADATA,
    RUNTIME_INT_METADATA,
    USING_NAMESPACE_METADATA,
    ConcisePreprocessorDirectivesRule,
    ExplicitConstructorRule,
    IdentifierNamingRule,
    RedundantCastRule,
    RuntimeIntRule,
    UsingNamespaceRule,
)


class RuleContractAsserts(TestCase):
    def assert_rule_contract(self, rule, metadata) -> None:
        self.assertEqual(rule.rule_id, metadata.rule_id)
        self.assertEqual(rule.action_kind, metadata.action_kind)
        self.assertEqual(rule.engine_id, metadata.engine_id)
        self.assertEqual(rule.supported_checks, metadata.supported_checks)
        self.assertEqual(rule.preview_only, metadata.preview_only)
        self.assertEqual(rule.risk_level, metadata.risk_level)


class TestTidyTaskAutoFixRuleContract(RuleContractAsserts):
    def test_identifier_naming_contract(self):
        self.assert_rule_contract(IdentifierNamingRule(), IDENTIFIER_NAMING_METADATA)

    def test_redundant_cast_contract(self):
        self.assert_rule_contract(RedundantCastRule(), REDUNDANT_CAST_METADATA)

    def test_runtime_int_contract(self):
        self.assert_rule_contract(RuntimeIntRule(), RUNTIME_INT_METADATA)

    def test_explicit_constructor_contract(self):
        self.assert_rule_contract(ExplicitConstructorRule(), EXPLICIT_CONSTRUCTOR_METADATA)

    def test_using_namespace_contract(self):
        self.assert_rule_contract(UsingNamespaceRule(), USING_NAMESPACE_METADATA)

    def test_concise_preprocessor_contract(self):
        self.assert_rule_contract(ConcisePreprocessorDirectivesRule(), CONCISE_PREPROCESSOR_METADATA)

