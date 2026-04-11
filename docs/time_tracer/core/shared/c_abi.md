# Core C ABI (`tracer_core_*`)

## Scope
1. This document defines the stable C ABI exported by `time_tracer_core.dll`.
2. The ABI is intended for host-side presentation layers and Android JNI
   internal bridging.
3. This document is the single source of truth for core C ABI naming and JSON
   payload contract.

## Naming Rules
1. All core C ABI symbols use the `tracer_core_` prefix.
2. New symbols must not use the legacy `tt_*` prefix.
3. Names are snake_case and include the operation domain when applicable.

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
15. `tracer_core_runtime_ingest_sync_status_json`
16. `tracer_core_runtime_clear_ingest_sync_status_json`
17. `tracer_core_runtime_convert_json`
18. `tracer_core_runtime_import_json`
19. `tracer_core_runtime_validate_structure_json`
20. `tracer_core_runtime_validate_logic_json`
21. `tracer_core_runtime_query_json`
22. `tracer_core_runtime_report_json`
23. `tracer_core_runtime_report_targets_json`
24. `tracer_core_runtime_report_batch_json`
25. `tracer_core_runtime_export_json`
26. `tracer_core_runtime_tree_json`
27. `tracer_core_runtime_crypto_encrypt_json`
28. `tracer_core_runtime_crypto_decrypt_json`
29. `tracer_core_runtime_crypto_inspect_json`
30. `tracer_core_runtime_txt_json`

## JSON Boundary Policy
1. JSON-returning runtime operations keep UTF-8 JSON object responses as the ABI
   boundary.
2. Most runtime request payloads also use `const char*` UTF-8 JSON objects.
3. The current exception is
   `tracer_core_runtime_clear_ingest_sync_status_json`, which takes no
   `request_json` argument and returns the standard JSON ack envelope.
4. Request parsing may be implemented by shared codec or local parser, but the
   external contract remains JSON-object based where a request body exists.
5. JSON evolution is additive unless a dedicated compatibility review approves a
   new struct-based ABI surface.

## Payload Contract
1. Request payloads are UTF-8 JSON object strings.
2. Response payloads are UTF-8 JSON object strings.
3. `tracer_core_get_capabilities_json` returns:
   - `abi` object (`name`, `version`)
   - `features` object with additive boolean feature flags
   - currently documented feature flags include:
     - `build_info_json`
     - `command_contract_json`
     - `runtime_log_callback`
     - `runtime_diagnostics_callback`
     - `runtime_crypto_progress_callback`
     - `runtime_ingest_json`
     - `runtime_ingest_sync_status_json`
     - `runtime_convert_json`
     - `runtime_import_json`
     - `runtime_validate_structure_json`
     - `runtime_validate_logic_json`
     - `runtime_query_json`
     - `runtime_report_json`
     - `runtime_report_batch_json`
     - `runtime_report_targets_json`
     - `runtime_export_json`
     - `runtime_tree_json`
     - `runtime_txt_json`
     - `processed_json_io`
     - `report_markdown`
     - `report_latex`
     - `report_typst`
4. `tracer_core_get_build_info_json` returns:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
   - `core_version`
   - `abi_name`
   - `abi_version`
   - `build_time_utc`
5. `tracer_core_get_command_contract_json` returns:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
   - `contract_version`
   - `commands`
6. `tracer_core_runtime_query_json` supports action `mapping_names`:
   - request: `{ "action": "mapping_names" }`
   - response `content`: `{ "names": ["alias_or_full_name", "..."] }`
7. `tracer_core_runtime_check_environment_json` returns:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
   - optional `messages`
8. `tracer_core_runtime_resolve_cli_context_json` returns:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
   - `paths`
   - `cli_config`
9. `tracer_core_runtime_ingest_sync_status_json` request/response contract:
   - request fields:
     - optional `months` (`string[]`)
   - response fields:
     - `ok`
     - `items`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
   - `items[]` currently includes:
     - `month_key`
     - `txt_relative_path`
     - `txt_content_hash_sha256`
     - `ingested_at_unix_ms`
10. `tracer_core_runtime_clear_ingest_sync_status_json` response contract:
   - no request body
   - response follows the standard ack-style envelope:
     - `ok`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
11. `tracer_core_runtime_tree_json` request/response contract:
   - request fields: `list_roots`, `root_pattern`, `max_depth`, `period`,
     `period_argument`, `root`
   - response fields: `ok`, `found`, `error_message`, `roots`, `nodes`
12. `tracer_core_runtime_report_json` and
    `tracer_core_runtime_report_batch_json` response contract:
   - standard text envelope fields:
     - `ok`
     - `content`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
   - may additionally include:
     - `report_hash_sha256`
13. `tracer_core_runtime_report_targets_json` request/response contract:
   - request fields:
     - `type` (`day|month|week|year`)
   - response fields:
     - `ok`
     - `type`
     - `items`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
14. `tracer_core_runtime_crypto_*_json` contracts:
   - request/response payloads are UTF-8 JSON objects
   - encrypt request fields:
     - `input_path`
     - `output_path`
     - `passphrase`
     - `date_check_mode`
     - `security_level`
   - decrypt request fields:
     - `input_path`
     - `passphrase`
     - optional `output_path`
   - inspect request fields:
     - `input_path`
     - `passphrase`
   - response envelope fields:
     - `ok`
     - `content`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
15. `tracer_core_runtime_txt_json` contract:
   - request/response payloads are UTF-8 JSON objects
   - `action` currently supports:
     - `default_day_marker`
     - `resolve_day_block`
     - `replace_day_block`
   - detailed DTO fields and semantics live in
     `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`

## Response Envelope Contract
1. Standard envelope fields:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
   - optional `content`
   - optional `report_hash_sha256`
2. `ok` and `error_message` are required semantic fields.
3. Operations may add fields without breaking the envelope.
4. Envelope normalization is implemented in `tracer_transport`.

## Error Contract
1. Functions returning `const char*` use thread-local response buffers.
2. `tracer_core_last_error` exposes thread-local last-error text.
3. Empty last-error text means "no last error".

## Layered Ownership
1. This file defines the stable external ABI contract only.
2. Transport codec/envelope implementation details live in `tracer_transport`.
3. Android JNI runtime-boundary behavior lives in the Android client docs.

## Host Integration Notes
1. Windows CLI dynamic loading binds `tracer_core_*` symbols only.
2. Missing runtime dependency must fail fast before command execution.
3. Core does not assume direct terminal output; hosts should register
   callbacks or equivalent sinks.
4. Recommended host strategy:
   - stdout for business payload
   - stderr for callback logs and diagnostics
   - preserve UTF-8 and avoid ANSI dependency on contract-critical paths

## Related Shared Docs
1. [../errors/error-model.md](../errors/error-model.md)
2. [../errors/error-codes.md](../errors/error-codes.md)

## Detailed Related Contracts
1. `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`
2. `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
3. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v3.md`
4. `docs/time_tracer/clients/android_ui/runtime-protocol.md`
5. `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`

## Related Implementation Paths
1. `apps/tracer_core_shell/api/c_api/tracer_core_c_api.cpp`
2. `apps/tracer_core_shell/api/c_api/capabilities/**`
3. `apps/tracer_core_shell/api/c_api/runtime/*.cpp`
