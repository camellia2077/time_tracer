# TXT Structure Validation

## Scope

TXT structure validation checks whether source text can be accepted as a valid
time-tracer text document before semantic rules are evaluated.

## Important Clarification

This phase is not just regex validation.

The implementation is line-classification plus parser/state-machine logic.
It also is not fully config-free, because some line interpretation depends on
validated config.

## Typical Responsibilities

1. recognize required headers such as `yYYYY` and `mMM`
2. classify line shapes and reject malformed line formats
3. enforce structural ordering constraints
4. distinguish semantic event lines from remarks and metadata
5. ensure the parsed output is coherent enough for logic validation

## Config Coupling

Structure validation already depends on config-backed semantics such as:
1. `remark_prefix`
2. `wake_keywords`
3. alias-derived allowed event names

That is why config validation must already have passed before this phase runs.

## Typical Failure Shapes

1. missing or malformed year/month headers
2. invalid line format
3. remark placement that violates source structure rules
4. unrecognized source line shape

## Code Entry References
1. `libs/tracer_core/src/domain/logic/validator/structure/structure_validator.cpp`
2. `libs/tracer_core/src/domain/logic/validator/txt/facade/text_validator.cpp`
