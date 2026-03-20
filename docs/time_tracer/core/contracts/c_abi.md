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
4. `tracer_core_get_build_info_json`
5. `tracer_core_get_command_contract_json`
6. `tracer_core_runtime_check_environment_json`
7. `tracer_core_runtime_resolve_cli_context_json`
8. `tracer_core_last_error`
9. `tracer_core_set_log_callback`
10. `tracer_core_set_diagnostics_callback`
11. `tracer_core_set_crypto_progress_callback`
12. `tracer_core_runtime_create`
13. `tracer_core_runtime_destroy`
14. `tracer_core_runtime_ingest_json`
15. `tracer_core_runtime_convert_json`
16. `tracer_core_runtime_import_json`
17. `tracer_core_runtime_validate_structure_json`
18. `tracer_core_runtime_validate_logic_json`
19. `tracer_core_runtime_query_json`
20. `tracer_core_runtime_report_json`
21. `tracer_core_runtime_report_batch_json`
22. `tracer_core_runtime_export_json`
23. `tracer_core_runtime_tree_json`
24. `tracer_core_runtime_crypto_encrypt_json`
25. `tracer_core_runtime_crypto_decrypt_json`
26. `tracer_core_runtime_crypto_inspect_json`

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
   - capability flags include `build_info_json`, `command_contract_json`,
     `runtime_log_callback`, `runtime_diagnostics_callback`,
     `runtime_crypto_progress_callback`
4. `tracer_core_get_build_info_json` returns:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `error_code` (`string`)
   - `error_category` (`string`)
   - `hints` (`string[]`)
   - `core_version` (`string`)
   - `abi_name` (`string`)
   - `abi_version` (`integer`)
   - `build_time_utc` (`string`)
5. `tracer_core_get_command_contract_json` request/response:
   - request: optional JSON object, currently supports `command` filter string
   - response:
     - `ok` (`bool`)
     - `error_message` (`string`)
     - `error_code` (`string`)
     - `error_category` (`string`)
     - `hints` (`string[]`)
     - `contract_version` (`string`)
     - `commands` (`object[]`)
     - command item fields: `id`, `aliases`, `supports`
6. `tracer_core_runtime_query_json` supports action `mapping_names`:
   - Request: `{ "action": "mapping_names" }`
   - Response `content`: `{ "names": ["alias_or_full_name", "..."] }`
7. `tracer_core_runtime_check_environment_json` returns:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `error_code` (`string`)
   - `error_category` (`string`)
   - `hints` (`string[]`)
   - `messages` (`string[]`, optional diagnostic list)
8. `tracer_core_runtime_resolve_cli_context_json` returns:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `error_code` (`string`)
   - `error_category` (`string`)
   - `hints` (`string[]`)
   - `paths` (`object`, present on success)
   - `cli_config` (`object`, present on success)
9. `tracer_core_runtime_tree_json` request/response contract:
   - Request:
     - `list_roots` (`bool`, optional)
     - `root_pattern` (`string`, optional)
     - `max_depth` (`int`, optional)
     - `period` (`string`, optional)
     - `period_argument` (`string`, optional)
     - `root` (`string`, optional)
   - Response:
     - `ok` (`bool`)
     - `found` (`bool`, default `true`)
     - `error_message` (`string`)
     - `roots` (`string[]`)
     - `nodes` (`object[]`)
   - `nodes[]` currently includes:
     - `name` (`string`)
     - `path` (`string`, optional)
     - `duration_seconds` (`integer`, optional)
     - `children` (`object[]`, recursive)
   - Note: internal core DTO may evolve with more fields, but C ABI tree node payload remains additive and backward-compatible.
10. `tracer_core_runtime_crypto_*_json` request/response contracts:
   - Request payloads are UTF-8 JSON object strings.
   - `tracer_core_runtime_crypto_encrypt_json` request fields:
     - `input_path` (`string`, required)
     - `output_path` (`string`, required)
     - `passphrase` (`string`, required)
     - `date_check_mode` (`string`, optional): `none|continuity|full`, default `none`
     - `security_level` (`string`, optional):
       `min|interactive|moderate|high|max`, default `interactive`
       - compatibility alias: `sensitive` -> `high`
     - behavior:
       - input must be a directory; runtime recursively collects `.txt` files
       - every input TXT basename must be `YYYY-MM.txt`, and file name must match `yYYYY + mMM` headers
       - runtime validates every collected TXT structure and logic before export
       - output is a `.tracer` exchange file whose outer carrier is `file_format_v2`
         and whose plaintext payload is `tracer_exchange_package_v3`
   - `tracer_core_runtime_crypto_decrypt_json` request fields:
     - `input_path` (`string`, required)
     - `passphrase` (`string`, required)
     - `output_path` (`string`, optional)
     - behavior:
       - input must be a single `.tracer` file
       - `output_path` is a transaction work root for staging / backup / retained failure artifacts
       - decrypt/import is a transactional full import, not a package unpack operation
       - package converter TOML files are validated and then applied to current active config
       - package months replace matching local months; local months outside the package are preserved
       - decrypt/import validates the full effective TXT set and then rebuilds the database
       - any failure must roll back active config, local TXT, and database state
   - `tracer_core_runtime_crypto_inspect_json` request fields:
     - `input_path` (`string`, required)
     - `passphrase` (`string`, required)
     - behavior:
       - `output_path` is not supported and is rejected when present
       - input must be a single `.tracer` file
       - inspect returns outer `.tracer` header fields plus decrypted package summary
   - Response payloads follow standard text-query envelope:
     - `ok` (`bool`)
     - `content` (`string`)
     - `error_message` (`string`)
     - `error_code` (`string`)
     - `error_category` (`string`)
     - `hints` (`string[]`)
   - Detailed semantics (path resolution, extension rules, success content text):
     - `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`
   - Payload format references:
     - `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
     - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v3.md`
11. callback registration contract:
   - `tracer_core_set_log_callback(callback, user_data)`
   - `tracer_core_set_diagnostics_callback(callback, user_data)`
   - `tracer_core_set_crypto_progress_callback(callback, user_data)`
   - callback message encoding is UTF-8
   - callback severity value mapping:
     - log: `0=info`, `1=warn`, `2=error`
     - diagnostics: `0=info`, `1=warn`, `2=error`
   - crypto progress callback payload:
     - callback signature: `void callback(const char* utf8_progress_json, void* user_data)`
     - payload schema follows `docs/time_tracer/core/contracts/crypto/progress_callback_v1.md`
   - passing `nullptr` callback clears current registration
   - callback failures must not abort core runtime execution

## Response Envelope Contract
1. Standard response envelope fields:
   - `ok` (`bool`)
   - `error_message` (`string`)
   - `error_code` (`string`)
   - `error_category` (`string`)
   - `hints` (`string[]`)
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
2. Transport codec/envelope/field implementation details are maintained in `docs/time_tracer/architecture/libraries/tracer_transport.md`.
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
3. Core runtime no longer assumes direct terminal output; host should register
   callbacks (or inject logger/sink through non-ABI factory path) to receive
   runtime logs/diagnostics.
4. Recommended host output strategy:
   - keep command business payload on stdout (for machine parsing contracts)
   - send callback logs/diagnostics to stderr
   - preserve UTF-8 and avoid ANSI dependency for contract-critical paths
5. Rust CLI baseline (2026-03-02):
   - registers both callbacks before runtime bootstrap
   - registers crypto progress callback when symbol is present and `TRACER_CLI_CRYPTO_PROGRESS=1` (default)
   - supports optional callback timestamp by env: `TRACER_CLI_LOG_TIMESTAMP=1`
   - supports optional runtime timing log by env: `TRACER_CLI_TIMING=1`
   - callback stream can be disabled by env: `TRACER_CLI_CORE_LOG=0`

## Contract Test Guidance
1. Failure-path tests should assert stable machine contract first:
   - `error_code`
   - `error_category`
   - `hints`
2. Human-readable message matching should stay minimal and non-fragile.
3. CLI-side parse/validation failures should still surface contract fields even
   when payload is plain text on stderr.

## Android Status
1. Android keeps JNI bridge entrypoints as runtime-facing API for Kotlin/Compose.
2. JNI internal implementation is migrated to unified `tracer_core_*` ABI.
3. Android still ships as a single bridge `.so`; no standalone core DLL on Android.

## Migration Status
1. Legacy `tt_*` core C ABI symbols are removed from production code.
2. Prefix migration verification baseline:
   - `temp/C-ABI/stepB0_remaining_tt_symbols.log`
   - `temp/C-ABI/stepB1_exports_tracer_core.log`
