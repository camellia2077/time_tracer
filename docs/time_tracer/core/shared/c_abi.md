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
22. `tracer_core_runtime_temporal_insights_json`
23. `tracer_core_runtime_insights_batch_json`
25. `tracer_core_runtime_crypto_encrypt_json`
26. `tracer_core_runtime_crypto_decrypt_json`
27. `tracer_core_runtime_crypto_inspect_json`
28. `tracer_core_runtime_config_json`
29. `tracer_core_runtime_update_activity_remark_atomically_json`
30. `tracer_core_runtime_update_day_remark_atomically_json`

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
     - `runtime_temporal_insights_json`
     - `runtime_insights_batch_json`
     - `runtime_record_activity_atomically_json`
     - `runtime_update_activity_remark_atomically_json`
     - `runtime_update_day_remark_atomically_json`
     - `runtime_config_json`
     - `processed_json_io`
     - `insights_markdown`
     - `insights_latex`
     - `insights_typst`
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
11. `tracer_core_runtime_temporal_insights_json` is the single canonical
    insights ABI surface:
   - request fields:
     - `operation_kind` (`query|structured_query|targets|export`)
     - `display_mode` (`day|week|month|year|range|recent`)
     - optional `selection_kind` (`single_day|date_range|recent_days`)
     - optional `date`
     - optional `start_date`
     - optional `end_date`
     - optional `days`
     - optional `anchor_date` (`recent_days` only)
     - optional `format`
     - optional `locale` (`en|zh|ja`; unknown values fall back to English for
       Markdown text)
     - optional `export_scope` (`single|all_matching|batch_recent_list`)
     - optional `recent_days_list`
   - `query` response fields:
     - standard text envelope fields
     - optional `insights_window_metadata`-derived fields for recent/range text
       responses
     - optional `insights_hash_sha256`
   - `structured_query` response fields:
     - `ok`
     - `display_mode`
     - `selection_kind`
     - `insights_kind`
     - `insights`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
   - `targets` response fields:
     - `ok`
     - `type`
     - `items`
     - `error_message`
     - `error_code`
     - `error_category`
     - `hints`
   - `export` response follows the standard ack-style envelope
12. `tracer_core_runtime_insights_batch_json` remains a separate helper for
    multi-days recent text rendering:
   - request fields:
     - `days_list`
     - optional `format`
   - response fields:
     - standard text envelope fields
     - optional `insights_hash_sha256`
13. `tracer_core_runtime_crypto_*_json` contracts:
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
14. `tracer_core_runtime_config_json` contract:
   - request/response payloads are UTF-8 JSON objects
   - `action` currently supports:
     - `default_day_marker`
     - `resolve_day_block`
     - `replace_day_block`
   - `convert_activity_names`
   - `replace_canonical_activity_names`
     - replaces exact canonical activity tokens for a configuration migration
   - `replace_alias_activity_names`
     - replaces exact authored alias tokens for a configuration migration
     - request `replacements[]` entries contain `old_alias` and `new_alias`
   - `apply_activity_hierarchy_operation`
     - request: `toml_content` and `operation`
     - `operation.kind` is one of `add_group`, `delete_group`, `add_leaf`, `set_leaf_aliases`, `delete_leaf`, `promote_leaf`, `move_leaf`, `move_group`, `merge_leaf_canonical`, `set_group_aliases`, `rename_parent`, `rename_group_canonical`, `rename_leaf_canonical`, `append_leaf_alias`, `append_group_alias`, or `rename_group_alias`
     - optional operation fields are `target_path`, `destination_path`, `canonical_key`, `new_name`, `old_parent`, `target_alias`, `old_alias`, and `aliases`; their required combination is determined by `kind`
     - paths are dot-separated canonical keys relative to `[canonical]`; `root` represents the root parent for add operations
     - response: `updated_toml_content`, canonical `replacements[]`, alias-key `alias_replacements[]`, and `hierarchy` (the core-validated parent/node snapshot); the caller persists TOML and performs TXT/database migration when replacements are returned
     - `rename_parent` changes the TOML `parent` and every document canonical path below it. `new_name` is the new parent; `old_parent`, when supplied, must match the TOML `parent`. A same-name rename or a path-shaped/whitespace-containing parent is rejected.
     - `rename_parent` does not accept a filename and never performs filesystem IO. Hosts must treat the TOML stem and `parent` as one value and commit the corresponding filename change together with TOML/TXT/database migration.
     - `merge_leaf_canonical` requires `target_path` and `destination_path` to identify two leaf nodes in the same document. It removes the source leaf, keeps the destination aliases unchanged, returns source-canonical -> destination-canonical, and maps every source alias to the destination leaf's first alias. Group merge is rejected.
   - `move_activity_hierarchy_leaf_between_documents`
     - request: `documents[]` containing the complete canonical TOML document set,
       `source_name`, `destination_name`, and an operation object with
       `kind=move_leaf`, `target_path` or `target_alias`, and
       `destination_path` (`root` or an existing destination group)
     - response: `updated_documents[]` for the source and destination TOMLs,
       canonical `replacements[]`, and alias-key `alias_replacements[]`
     - compatibility action for leaf-only callers; the caller owns
       TXT/database migration and atomic persistence.
   - `move_activity_hierarchy_node_between_documents`
     - request shape matches the leaf action; `operation.kind` is `move_leaf`
       or `move_group`
     - `move_group` moves the complete target group subtree, preserving group
       aliases, nested groups, and leaf aliases
     - response contains replacements for the target group and every descendant
       whose canonical path changes
   - `rewrite_activity_hierarchy_document`
     - request: `original_toml_content` and `updated_toml_content`
     - response: Core-serialized `updated_toml_content`, canonical `replacements[]`, alias-key `alias_replacements[]`, and `hierarchy`
     - callers must pass this result through host-side TXT/database migration before persisting the canonical TOML
   - `describe_activity_hierarchy`
     - request: `toml_content`
     - response: `hierarchy`, the core-validated parent/node snapshot without applying an edit
   - `validate_activity_hierarchy_documents`
     - request: `documents[]`, where every item has diagnostic `source_name` and in-memory `toml_content`
     - validates every document and rejects aliases duplicated across the supplied set; it never reads or writes files
   - `render_activity_hierarchy_text`
     - request: `toml_content`, optional `show_aliases` boolean
     - response: plaintext `content` rendered by core from the validated hierarchy
   - detailed DTO fields and semantics live in
     `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`

The describe_activity_hierarchy model is presentation-neutral. Each selectable node
contains canonical_key, canonical path relative to [canonical], kind (leaf or
group), aliases, and recursive children. The legacy is_group field remains in
the JSON response for compatibility; new consumers should use kind.

## Response Envelope Contract
1. Standard envelope fields:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
   - optional `content`
   - optional `insights_hash_sha256`
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
2. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v6.md`
4. `docs/time_tracer/presentation/android/runtime-protocol.md`
5. `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`

## Related Implementation Paths
1. `apps/tracer_core_shell/api/c_api/tracer_core_c_api.cpp`
2. `apps/tracer_core_shell/api/c_api/capabilities/**`
3. `apps/tracer_core_shell/api/c_api/runtime/*.cpp`
