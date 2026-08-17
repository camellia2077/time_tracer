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
  - Native writes encrypted `.zip` bytes directly to that fd.

## Authoring Ownership

Raw TXT and canonical activity-hierarchy TOML changes have one semantic owner:
Core. This applies to create, edit, delete, rename, and conversion operations.

1. Presentation collects user intent and keeps only UI state, document/path
   selection, and presentation feedback.
2. Presentation invokes the owning Core runtime operation through the JNI/C ABI
   boundary; it does not construct an alternative local mutation algorithm.
3. Core validates and determines the resulting TXT/TOML content, replacement
   plan, or atomic write outcome.
4. Core atomic Record and remark operations own their physical TXT write,
   validation, and re-ingest transaction.
5. When a Core content-transform action returns updated content or a
   replacement plan, Android may perform the contract-defined physical write
   and sync transaction, but it persists exactly that Core result and does not
   reinterpret its semantics.

This rule prevents Android-local fallback writers from drifting from the
shared TXT/TOML behavior used by the other hosts.

## C ABI Scope

Current Android JNI integration uses C ABI entrypoints in these categories:

- runtime create/destroy
- ingest/query/insights
- record-atomic pipeline calls (including explicit `time_order_mode` passthrough)
- activity and day remark atomic update calls
- Config runtime calls (`tracer_core_runtime_config_json`) for TXT day-block,
  activity hierarchy operations/raw alias-TOML rewrite, and current-month
  activity-name conversion, and Quick Access TOML content parsing/rendering
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

- JNI request encoding for main ingest/query/insights paths is unified through shared transport helpers.
- Insights now uses `TemporalInsightsQueryRequest` on Kotlin side and
  `tracer_core_runtime_temporal_insights_json` as the only canonical insights
  C ABI entrypoint.
- Android insights JNI keeps `nativeInsightsJson(requestJson)` as the single raw
  insights native method; legacy `nativeInsights(...)` no longer exists.
- Atomic record requests carry explicit `time_order_mode` (`strict_calendar` / `logical_day_0600`) from Kotlin -> JNI -> C ABI.
- Config requests use the shared `tracer_core_runtime_config_json` family and keep
  month-TXT business semantics in core rather than Kotlin UI helpers.
- Android must not directly save a canonical TOML document. Every canonical TOML
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
- `set_parent_color` writes the optional parent presentation color. Android
  sends `operation.color` as a `#RRGGBB` string to set it, or JSON `null` to
  remove it; Core validates the string before rewriting the TOML.
- `merge_leaf_canonical` merges one leaf into another leaf in the same TOML.
  Android sends the source in `operation.target_path` and the destination in
  `operation.destination_path`. Core removes the source leaf, returns canonical
  and alias replacement plans, and rejects groups; Android applies the plan
  through the same TOML/TXT/candidate-database migration transaction.
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
- The `previous_activity_tail` data query is read-only and returns the latest
  persisted activity end boundary at or before the requested logical date. The
  Android caller sends that date through the existing `from_date` field and
  requests `semantic_json`; the semantic payload is
  `{action: "previous_activity_tail", output_mode: "semantic_json", found: bool,
  date?: "YYYY-MM-DD", end_time?: "HH:mm:ss"}`.
- The previous-activity-tail query is a suggestion source only. Android may
  render the returned end time and let the user copy it into an interval
  draft's start time, but the query never changes the draft or persisted data.
- The `latest_activity_record` data query is also read-only. Android sends the
  selected logical date through `from_date` and requests `semantic_json`;
  Core returns the latest persisted record for that exact date with
  `activity`, `record_kind`, `start_time`, `end_time`, and `duration_seconds`.
  The time fields use ISO local time `HH:mm:ss`; only the TXT persistence
  boundary uses compact `HHMMSS`.
- Activity hierarchy responses use Core's presentation-neutral node model:
  each node carries canonical_key, canonical path, kind, aliases, and children.
  Android consumes kind internally and keeps the existing presentation adapter;
  the UI does not add canonical or alias text.
- Quick Access is an alias-only `user/quick_access.toml` document. Android owns
  file existence checks, directory creation, and physical reads/writes through
  `ConfigTomlStorage`; Core only parses existing TOML content or renders new
  TOML content through `tracer_core_runtime_config_json`. A missing file is an
  empty Quick Access list and is not created by a read.
- Kotlin-visible response shape remains `{ok,error_message,content}`.
- JNI native method signatures remain stable.

## Insights Runtime Family

Android insights currently follows this path:

1. feature/app code builds `TemporalInsightsQueryRequest`.
2. `RuntimeInsightsDelegate.insightsMarkdown(request)` encodes that request as the
   temporal JSON payload.
3. `NativeRuntimeBridge` forwards the JSON string to
   `NativeBridge.nativeInsightsJson(...)`.
4. JNI forwards the payload to `tracer_core_runtime_temporal_insights_json`.
5. Core multiplexes `query|structured_query|targets|export` through that single
   temporal insights entrypoint.

Notes:

- Android UI in this refactor does not expose a recent anchor picker.
- The Android contract already supports optional `anchorDate` so future product
  work can send anchored recent requests without another ABI change.
- Markdown insights requests carry the current Android UI language as `locale`;
  Core selects `insights/markdown/<locale>/` and falls back to English.

Structured day insights return `detailed_records` for the Insights tab timeline.
Each record includes `record_kind`, currently `interval` or `end_only`.
Android must use this Core-produced kind instead of inferring semantics from
empty timestamps: an `end_only` record displays a single localized
"as-of" time point and has no duration segment.

When the matching `config/user/activity_hierarchy` parent TOML declares an
optional `color = "#RRGGBB"`, its structured records also include
`parent_color`. Android renders that value only as a narrow decorative card
edge in Records; it does not recolor text, duration, or card surfaces. The
field is absent when the parent has no configured color.

For Week, Month, Year, Range, and Recent, structured insights additionally
return `activity_days[]` in descending date order. Each item keeps the Core
logical day plus that day's `detailed_records`, allowing the Android Activity
section to browse a selected window without parsing the Markdown report.

Period structured insights also return `total_duration` and
`matched_record_count` at `insights` level. These are the selected window's
single Core-owned activity aggregate: total duration and total occurrence
count. Android stores and reuses this aggregate for the Activity overview and
period comparison instead of summing `activity_days[]` again.

Structured insights return `insights.statuses[]` with each configured status's
`id`, `label`, `occurrence_count`, and `total_duration`. Android renders those
statistics directly in the status editor, so the editor and Markdown report
are derived from the same selected period.

Android persists custom status definitions locally and initializes Core with
six independent arrays: `day`, `week`, `month`, `year`, `recent`, and `range`.
Core selects the array matching the temporal display mode; these definitions
never come from CLI `user/insights.toml`.

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
   - structured day-edit parsing and normalized rendering (`resolve_day_edit`,
     `apply_day_edit`), including time/event-kind/remark preservation and
     Core-owned monotonic time bounds for the editor picker
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
- Encrypted ZIP export insights only the package-level overall progress. Android's
  export card consumes that value and renders one progress bar; current-file
  progress remains available for import/other exchange operations.
- Android host intentionally exposes only the Android-supported security levels.

## Open Next

- Stable structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Config/runtime bootstrap:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
