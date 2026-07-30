# Android Runtime Protocol

## Purpose

Describe the stable Android runtime boundary contracts between Kotlin, JNI, and core C ABI calls.

## When To Open

- Open this when the task changes JNI method contracts, runtime payload shapes, or C ABI alignment.

## What This Doc Does Not Cover

- UI routing
- Feature behavior
- File-level ownership

## Boundary Model

Runtime call chain:

1. Kotlin app/runtime code calls `NativeRuntimeBridge`.
2. `NativeRuntimeBridge` forwards to raw JNI methods on `NativeBridge`.
3. JNI calls either core C ABI entrypoints (`tracer_core_*`) or Android host runtime adapters.
4. Host/C ABI forwards into core/runtime implementation.

Important rules:

- Kotlin does not call `tracer_core_*` directly.
- `NativeBridge` is the raw JNI registration surface and should stay thin.
- Android runtime flows should prefer `NativeRuntimeBridge` over calling `NativeBridge.native*` directly.
- Mode decisions (for example record `time_order_mode`) are made in upper Kotlin use cases; JNI and C ABI layers only validate and forward.
- Business payloads between JNI and core remain UTF-8 JSON strings.
- Large binary exchange outputs do not go through JSON.
  - Android tracer exchange export passes a detached output fd into JNI.
  - Native writes encrypted `.tracer` bytes directly to that fd.

## C ABI Scope

Current Android JNI integration uses C ABI entrypoints in these categories:

- runtime create/destroy
- ingest/query/report
- record-atomic pipeline calls (including explicit `time_order_mode` passthrough)
- activity and day remark atomic update calls
- Config runtime calls (`tracer_core_runtime_config_json`) for TXT day-block,
  activity hierarchy operations/raw alias-TOML rewrite, and current-month
  activity-name conversion
- structure/logic validation
- last-error access

Android-specific host adapter scope currently covers:

- tracer exchange export/import/inspect
- crypto progress bridging for Android JNI callbacks

Canonical global rules live in:

- `docs/time_tracer/core/shared/c_abi.md`

TXT runtime day-block contract and ownership live in:

- `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`
- `docs/time_tracer/architecture/libraries/tracer_core.md`

## Stable Response Envelope

Kotlin-visible JNI responses are normalized to:

- `ok`
- `error_message`
- `content`

Some operations may add operation-specific fields, but the envelope shape stays stable.

## Transport Status

Current status:

- JNI request encoding for main ingest/query/report paths is unified through shared transport helpers.
- Reporting now uses `TemporalReportQueryRequest` on Kotlin side and
  `tracer_core_runtime_temporal_report_json` as the only canonical reporting
  C ABI entrypoint.
- Android reporting JNI keeps `nativeReportJson(requestJson)` as the single raw
  reporting native method; legacy `nativeReport(...)` no longer exists.
- Atomic record requests carry explicit `time_order_mode` (`strict_calendar` / `logical_day_0600`) from Kotlin -> JNI -> C ABI.
- Config requests use the shared `tracer_core_runtime_config_json` family and keep
  month-TXT business semantics in core rather than Kotlin UI helpers.
- Android must not directly save an alias TOML document. Every alias TOML
  change must use a Core hierarchy operation or
  `rewrite_activity_hierarchy_document`, then pass the returned TOML and one
  activity-name replacement plan through `RuntimeActivityHierarchyMigrationService`.
  The plan has one shared meaning for every entry (`old token` -> `new token`);
  its canonical and alias namespaces remain separate so each can use the
  matching Core TXT action.
- `rename_parent` is the parent-document rename operation. Android sends the
  current TOML content, `operation.new_name`, and should send
  `operation.old_parent` as a stale-content guard. Core returns the updated
  TOML, all affected canonical replacements, and the hierarchy whose `parent`
  is the new file stem. The runtime protocol carries no filesystem filename
  mutation; Android owns the later `<old_parent>.toml` ->
  `<new_parent>.toml` transaction.
- Cross-TOML activity hierarchy moves use
  `move_activity_hierarchy_node_between_documents`. Android sends the complete
  alias-document set plus source/destination file names; Core returns the
  updated source and destination TOMLs and canonical/alias replacements.
  Android passes those updated documents to one migration request so TXT and
  database rebuilding remain part of the same rollback boundary.
  The move_leaf operation moves one activity name; move_group moves the
  complete group subtree.
- Validation requests still have JNI-local request assembly.
- Android tracer exchange export supports an in-memory payload JSON request plus fd sink output.
- Tree responses are normalized before returning to Kotlin.
- Activity hierarchy responses use Core's presentation-neutral node model:
  each node carries canonical_key, canonical path, kind, aliases, and children.
  Android consumes kind internally and keeps the existing presentation adapter;
  the UI does not add canonical or alias text.
- Kotlin-visible response shape remains `{ok,error_message,content}`.
- JNI native method signatures remain stable.

## Reporting Runtime Family

Android reporting currently follows this path:

1. feature/app code builds `TemporalReportQueryRequest`.
2. `RuntimeReportDelegate.reportMarkdown(request)` encodes that request as the
   temporal JSON payload.
3. `NativeRuntimeBridge` forwards the JSON string to
   `NativeBridge.nativeReportJson(...)`.
4. JNI forwards the payload to `tracer_core_runtime_temporal_report_json`.
5. Core multiplexes `query|structured_query|targets|export` through that single
   temporal reporting entrypoint.

Notes:

- Android UI in this refactor does not expose a recent anchor picker.
- The Android contract already supports optional `anchorDate` so future product
  work can send anchored recent requests without another ABI change.
- Markdown report requests carry the current Android UI language as `locale`;
  Core selects `reports/markdown/<locale>/` and falls back to English.

## TXT Runtime Family

Android uses the TXT runtime family for shared month-TXT day-block semantics
and current-month activity-name representation conversion.

Current Android-facing responsibilities are:

1. Kotlin UI keeps presentation state such as mode, raw marker input, and
   editor visibility.
2. Android runtime services encode TXT actions and forward them through JNI.
3. JNI forwards those JSON payloads to `tracer_core_runtime_config_json`.
4. Core owns:
   - default day marker resolution
   - `MMDD` normalization and validation
   - month-TXT day marker line format (`dMMDD`)
   - day-block extraction and replacement
   - machine-readable fields such as `found`, `can_save`, and
     `day_content_iso_date`
   - activity-name alias/canonical conversion for the full month content
5. In ALL mode, the UI sends only the currently selected month draft to
   `convert_activity_names`; DAY mode does not expose or invoke this action.
6. Android does not re-implement these month-TXT semantics locally.
7. Config hierarchy migration uses `replace_canonical_activity_names` with
   explicit old/new canonical pairs. Runtime stages modified TOML/TXT sources,
   ingests a temporary database, and swaps databases only after candidate
   success; JNI `nativeShutdown()` closes the candidate handle before the swap.

## Crypto Progress Note

- Android crypto progress uses the same snapshot-to-JSON callback path as the core C ABI.
- Android host intentionally exposes only the Android-supported security levels.

## Open Next

- Stable structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Config/runtime bootstrap:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
