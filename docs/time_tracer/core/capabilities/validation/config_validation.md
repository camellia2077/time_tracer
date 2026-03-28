# Config Validation

## Scope

Config validation covers runtime-managed converter `TOML` files before their
values become trusted inputs to TXT validation or ingest workflow execution.

## What It Checks

1. file can be read and decoded as canonical text
2. `TOML` parses successfully
3. required sections and fields exist
4. field types are correct
5. rule constraints are satisfied
6. cross-field combinations remain valid

## What It Is Not

Config validation is not just regex matching.

It is parser/AST plus field/type/rule validation. Once config is accepted, its
values become inputs to downstream TXT validation and pipeline behavior.

## Why It Gates TXT Validation

TXT validation depends on config values such as:
1. remark prefix
2. wake keywords
3. alias-derived valid activity keywords

So a TXT file cannot be semantically validated against an invalid or partially
parsed config snapshot.

## Exchange Import Rule

When importing a `.tracer` package:
1. packaged converter `TOML` files are validated first
2. valid imported config is applied to the effective validation context
3. only then is the effective TXT set validated

This keeps TXT logic validation aligned with the imported alias/config state.

## Code Entry References
1. `libs/tracer_core/src/infra/config/loader/converter_config_loader.cpp`
2. `libs/tracer_core/src/infra/config/validator/converter/rules/converter_rules.cpp`
