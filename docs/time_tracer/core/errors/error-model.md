# Core Error Model

This document defines the shared core error model, log-writing strategy, and
layered ownership for runtime diagnostics.

## Goals
1. Normalize error output semantics and avoid duplicate multi-layer printing.
2. Keep error-log location and naming stable across hosts.
3. Keep collection and persistence of diagnostics inside core; hosts render.

## Current Contract
1. Core exposes diagnostic ports through `time_tracer::domain::ports`:
   - `IDiagnosticsSink`
   - `IErrorReportWriter`
2. Diagnostic output supports `info`, `warn`, and `error` severities.
3. Error reports are written as:
   - per-run ISO file: `errors-YYYY-MM-DDTHH-MM-SSZ.log`
   - fixed alias: `errors-latest.log`
4. Default error-report directory is `<runtime_data_root>/logs/`.
5. Core C ABI responses include:
   - `error_code`
   - `error_category`
   - `hints`
6. When `ok=true`, those fields default to empty values.
7. When `ok=false`, `error_code` and `error_category` must remain machine-usable.

## Suggested Unified Error Record

`ErrorRecord` is the conceptual model, even if the concrete type name differs:

| Field | Meaning |
| --- | --- |
| `code` | Stable machine-readable error code |
| `severity` | `info` / `warn` / `error` |
| `message` | Short user-facing description |
| `details` | Technical detail, optional |
| `source` | File/line/module locator, optional |
| `timestamp_utc` | UTC timestamp |
| `run_id` | Per-run logical identifier |

## Dedup Guidance
1. Dedup should happen in core, not independently in CLI/UI.
2. Suggested dedup key: `code + source + normalized_message`.
3. Repeated instances may collapse to one body plus an optional repeat count.
4. Summary and detail views must not render the same error block twice.

## Log File Strategy
1. Runtime start should create the per-run ISO log file.
2. Runtime start should reset `errors-latest.log`.
3. Each new error appends to both:
   - the current ISO log
   - `errors-latest.log`
4. Hosts should display the path returned by core, not synthesize it.

## Path Query Contract
1. Core should expose the current run error-log path.
2. Hosts should only display the path returned by core.
3. If file logging is disabled, core should return an explicit disabled state.

## Layer Ownership
1. Core owns:
   - error model
   - error codes
   - dedup
   - log persistence and returned paths
2. Hosts own:
   - text styling / color
   - interactive affordances such as open/share
   - host-local presentation decisions

## Related Shared Docs
1. [error-codes.md](error-codes.md)
2. [../shared/c_abi.md](../shared/c_abi.md)

## Related Code
1. `libs/tracer_core/src/domain/ports/diagnostics.hpp`
2. `libs/tracer_core/src/domain/ports/diagnostics.cpp`
3. `libs/tracer_core/src/infra/logging/file_error_report_writer.hpp`
4. `libs/tracer_core/src/infra/logging/file_error_report_writer.cpp`
5. `libs/tracer_core/src/infra/logging/validation_issue_reporter.module.cpp`
6. `apps/tracer_core_shell/host/bootstrap/android_runtime_factory.cpp`
7. `apps/tracer_core_shell/api/c_api/tracer_core_c_api.cpp`
8. `apps/tracer_core_shell/api/c_api/runtime/tracer_core_c_api_internal.cpp`
