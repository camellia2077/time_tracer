# Validation Diagnostics

## Purpose

Validation diagnostics must be stable enough for:
1. CLI and Android error presentation
2. log search and regression assertions
3. contract-level machine handling

## Shared Error-Code Vocabulary

The shared cross-capability error-code registry now lives in:
1. [../../shared/error-codes.md](../../shared/error-codes.md)

This page focuses on how validation uses that shared vocabulary.

## Main Validation Families

1. `validation.io.*`
2. `validation.structure.*`
3. `validation.line.*`
4. `validation.logic.*`
5. `validation.date.*`
6. `validation.source.*`
7. `validation.activity.*`

## Output Guidance

1. Keep machine-readable error codes stable.
2. Allow user-facing text to improve without changing the semantic code.
3. Preserve the concrete normalized-text failure when canonical text handling is
   the root cause.
4. Avoid hiding a validation failure behind a generic runtime/import/export
   wrapper if a more specific validation code is available.

## Related Code
1. `libs/tracer_core/src/domain/errors/error_codes.hpp`
2. `libs/tracer_core/src/infra/logging/validation_issue_reporter.module.cpp`
3. `libs/tracer_core/src/domain/logic/validator/common/diagnostic.hpp`
4. `libs/tracer_core/src/domain/logic/validator/common/validator_utils.hpp`
