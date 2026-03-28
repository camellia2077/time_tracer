# Error Codes (Core Shared)

This document records the shared core error-code vocabulary and the current
mapping guidance used across hosts.

Validation-focused interpretation now lives in:
1. [../capabilities/validation/diagnostics.md](../capabilities/validation/diagnostics.md)

## Goals
1. Provide stable, machine-readable identifiers.
2. Reduce semantic drift between Windows and Android.
3. Support logs, search, aggregation, and automation.

## Naming Rules
1. Use lower-case dot style: `domain.category.detail`
2. Keep codes stable even if human-facing messages change
3. Add a new code for a new semantic failure; do not recycle a near match

Examples:
1. `validation.source.unrecognized_activity`
2. `validation.time.discontinuity`
3. `runtime.io.path_not_found`

## Suggested Domain Groups
1. `validation.*`: source, structure, and logic validation
2. `pipeline.*`: workflow-stage failures
3. `import.*`: write/import stage failures
4. `export.*`: export stage failures
5. `runtime.*`: runtime, path, and dependency failures
6. `io.*`: file-system read/write failures
7. `config.*`: config load and parse failures

## Mapping Guidance For Existing Validator Errors

Source:
`libs/tracer_core/src/domain/logic/validator/common/validator_utils.hpp`

| `validator::ErrorType` | Suggested error code |
| --- | --- |
| `kFileAccess` | `validation.io.file_access` |
| `kStructural` | `validation.structure.invalid` |
| `kLineFormat` | `validation.line.invalid` |
| `kTimeDiscontinuity` | `validation.time.discontinuity` |
| `kMissingSleepNight` | `validation.sleep.missing_night` |
| `kLogical` | `validation.logic.invalid` |
| `kDateContinuity` | `validation.date.continuity_missing` |
| `kIncorrectDayCountForMonth` | `validation.date.incorrect_day_count` |
| `kSourceRemarkAfterEvent` | `validation.source.remark_after_event` |
| `kSourceNoDateAtStart` | `validation.source.no_date_at_start` |
| `kUnrecognizedActivity` | `validation.source.unrecognized_activity` |
| `kSourceInvalidLineFormat` | `validation.source.invalid_line_format` |
| `kSourceMissingYearHeader` | `validation.source.missing_year_header` |
| `kJsonTooFewActivities` | `validation.json.too_few_activities` |
| `kZeroDurationActivity` | `validation.activity.zero_duration` |
| `kActivityDurationTooLong` | `validation.activity.duration_too_long` |

## Diagnostic Compatibility Guidance

Source:
`libs/tracer_core/src/domain/logic/validator/common/diagnostic.hpp`

Current `Diagnostic.code` already carries business-facing codes such as
`activity.duration.zero`.

Guidance:
1. Preserve existing codes as compatibility inputs when needed.
2. Map them to normalized shared codes at stable output boundaries, or replace
   them fully once the downstream surface is ready.
3. Keep explicit old-to-new mapping documented; do not leave long-term hidden
   dual-track semantics.

## Runtime And Pipeline Starter Set

| Scenario | Suggested error code |
| --- | --- |
| Config load failed | `config.load.failed` |
| Config field invalid | `config.field.invalid` |
| Workflow step failed | `pipeline.step.failed` |
| Import transaction failed | `import.transaction.failed` |
| Export write failed | `export.write.failed` |
| Path not found | `runtime.io.path_not_found` |
| Runtime dependency missing | `runtime.dependency.missing` |

## C ABI Codes Already In Use (2026-03)

| Scenario | Error code | Category |
| --- | --- | --- |
| Generic runtime exception | `runtime.generic_error` | `runtime` |
| Build info interface failed | `runtime.build_info_error` | `runtime` |
| Resolve CLI context failed | `config.resolve_failed` | `config` |
| Invalid contract request | `contract.invalid_request` | `contract` |
| Internal contract exception | `contract.internal_error` | `contract` |

## Maintenance Rules
1. Update this file whenever a new shared error code is introduced.
2. Review compatibility impact before renaming an existing code.
3. Add tests for critical mapping paths.

## Related Code
1. `libs/tracer_core/src/domain/logic/validator/common/validator_utils.hpp`
2. `libs/tracer_core/src/domain/logic/validator/common/diagnostic.hpp`
3. `libs/tracer_core/src/domain/errors/error_codes.hpp`
4. `libs/tracer_core/src/infra/logging/validation_issue_reporter.module.cpp`
5. `libs/tracer_core/src/domain/ports/diagnostics.hpp`
