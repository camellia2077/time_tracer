# Core C ABI (`tracer_core_*`)

## Scope
1. This document defines the stable C ABI contract exported by `time_tracer_core.dll`.
2. The ABI is intended for host-side presentation layers and Android JNI internal bridging.
3. This document is the single source of truth for core C ABI naming and payload contract.

## Naming Rules
1. All core C ABI symbols must use `tracer_core_` prefix.
2. New core C ABI symbols must not use legacy `tt_*` prefix.
3. ABI names are snake_case and include operation domain (for example: `runtime_query_json`).

## Exported Symbol Set
1. `tracer_core_get_version`
2. `tracer_core_ping`
3. `tracer_core_get_capabilities_json`
4. `tracer_core_runtime_check_environment_json`
5. `tracer_core_runtime_resolve_cli_context_json`
6. `tracer_core_last_error`
7. `tracer_core_runtime_create`
8. `tracer_core_runtime_destroy`
9. `tracer_core_runtime_ingest_json`
10. `tracer_core_runtime_convert_json`
11. `tracer_core_runtime_import_json`
12. `tracer_core_runtime_validate_structure_json`
13. `tracer_core_runtime_validate_logic_json`
14. `tracer_core_runtime_query_json`
15. `tracer_core_runtime_report_json`
16. `tracer_core_runtime_report_batch_json`
17. `tracer_core_runtime_export_json`
18. `tracer_core_runtime_tree_json`

## JSON Boundary Policy
1. Runtime operation payloads keep `const char*` UTF-8 JSON as the ABI boundary.
2. Request parsing can be implemented by shared codec or local parser, but external contract remains JSON object based.
3. Current strategy is additive JSON evolution; introducing operation-specific C struct ABI requires separate compatibility review.

## Payload Contract
1. Request payloads are UTF-8 JSON object strings.
2. Response payloads are UTF-8 JSON object strings.
3. `tracer_core_get_capabilities_json` returns UTF-8 JSON object with:
   - `abi` object (`name`, `version`)
   - `features` object (boolean feature flags, additive extension allowed)
4. `tracer_core_runtime_query_json` supports action `mapping_names`:
   - Request: `{ "action": "mapping_names" }`
   - Response `content`: `{ "names": ["alias_or_full_name", "..."] }`
5. `tracer_core_runtime_check_environment_json` returns:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `messages` (`string[]`, optional diagnostic list)
6. `tracer_core_runtime_resolve_cli_context_json` returns:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `paths` (`object`, present on success)
   - `cli_config` (`object`, present on success)

## Response Envelope Contract
1. Standard response envelope fields:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `content` (`string`, when the operation returns text output)
2. `ok` and `error_message` are required semantic fields for all runtime responses.
3. Operations may add operation-specific fields without breaking envelope semantics (for example tree query fields `found`, `roots`, `nodes`).
4. Envelope normalization is implemented in `tracer_transport` (`ParseResponseEnvelope` / `SerializeResponseEnvelope`).

## Error Contract
1. Functions returning `const char*` always return thread-local response buffers.
2. `tracer_core_last_error` exposes thread-local last error text.
3. Callers must treat empty error string as "no last error".

## Layered Ownership
1. This file defines stable external contract only.
2. Transport codec/envelope/field implementation details are maintained in `modules/tracer_transport/README.md`.
3. Android JNI runtime boundary behavior is maintained in `docs/time_tracer/clients/android_ui/runtime-protocol.md`.

## Implementation Notes (Non-Contract)
1. Core C ABI implementation is split by operation domain to reduce single-TU coupling:
   - `time_tracer_core_c_api.cpp`: base/runtime lifecycle and capability entry
   - `time_tracer_core_c_api_workflow.cpp`: ingest/convert/import/validate
   - `time_tracer_core_c_api_reporting.cpp`: query/report/export/tree
   - `time_tracer_core_c_api_internal.cpp`: shared parsing/error/response helpers
2. This split is internal and does not change exported ABI names.

## Host Integration Notes
1. Windows CLI dynamic loader must bind `tracer_core_*` symbols only.
2. Missing runtime dependency must fail fast before command execution.

## Android Status
1. Android keeps JNI bridge entrypoints as runtime-facing API for Kotlin/Compose.
2. JNI internal implementation is migrated to unified `tracer_core_*` ABI.
3. Android still ships as a single bridge `.so`; no standalone core DLL on Android.

## Migration Status
1. Legacy `tt_*` core C ABI symbols are removed from production code.
2. Prefix migration verification baseline:
   - `temp/C-ABI/stepB0_remaining_tt_symbols.log`
   - `temp/C-ABI/stepB1_exports_tracer_core.log`
