from __future__ import annotations

from ..models import RuleMetadata

IDENTIFIER_NAMING_METADATA = RuleMetadata(
    rule_id="identifier_naming",
    action_kind="rename",
    engine_id="clangd",
    supported_checks=("readability-identifier-naming",),
    preview_only=False,
    risk_level="medium",
)

REDUNDANT_CAST_METADATA = RuleMetadata(
    rule_id="redundant_cast",
    action_kind="redundant_cast",
    engine_id="text",
    supported_checks=("readability-redundant-casting",),
    preview_only=False,
    risk_level="low",
)

RUNTIME_INT_METADATA = RuleMetadata(
    rule_id="runtime_int",
    action_kind="runtime_int",
    engine_id="text",
    supported_checks=("google-runtime-int",),
    preview_only=False,
    risk_level="low",
)

EXPLICIT_CONSTRUCTOR_METADATA = RuleMetadata(
    rule_id="explicit_constructor",
    action_kind="explicit_constructor",
    engine_id="text",
    supported_checks=("google-explicit-constructor",),
    preview_only=False,
    risk_level="low",
)

USING_NAMESPACE_METADATA = RuleMetadata(
    rule_id="using_namespace",
    action_kind="using_namespace",
    engine_id="text",
    supported_checks=("google-build-using-namespace",),
    preview_only=True,
    risk_level="medium",
)

CONCISE_PREPROCESSOR_METADATA = RuleMetadata(
    rule_id="concise_preprocessor_directives",
    action_kind="concise_preprocessor_directive",
    engine_id="text",
    supported_checks=("readability-use-concise-preprocessor-directives",),
    preview_only=False,
    risk_level="low",
)

