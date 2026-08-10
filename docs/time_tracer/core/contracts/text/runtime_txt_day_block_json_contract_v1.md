# Runtime TXT JSON Contract v1

## Scope

1. This document defines the host-facing JSON contract for
   `tracer_core_runtime_config_json`.
2. The contract covers month-TXT day-block semantics and activity-name
   representation conversion.
3. The runtime accepts raw month TXT content as input and does not read or
   write files directly.

## Ownership

1. `tracer_core` `pipeline` owns the business semantics described here.
2. `tracer_transport` owns JSON envelope encoding/decoding, not the meaning of
   these fields.
3. Hosts such as Android and Windows CLI own UI, terminal rendering, path
   handling, and local input-state policy.

## Shared Semantic Rules

1. `day_marker` is normalized by keeping digits only and truncating to 4
   characters.
2. A valid `day_marker` must be a legal `MMDD` value.
3. Month TXT content stores day marker lines as `dMMDD`.
4. Day-block extraction never includes the marker line itself in `day_body`.
5. Day-block replacement removes a duplicated leading `dMMDD` marker when the
   edited body starts with the same marker line.
6. Day-block replacement preserves user-authored trailing blank lines.
7. `day_content_iso_date` is only produced when both `selected_month` and the
   normalized `day_marker` are valid.
8. Remarks use the `//` syntax. A `//` line before the first event belongs to
   the day remark; a `//` line after an event belongs to the most recent event
   within the same day block. Multiple physical lines are joined with actual
   LF characters in memory and in SQLite `TEXT`. The two characters `\\n` in
   TXT remain literal text and are not decoded.

## Standard Envelope

1. Responses use the standard runtime JSON envelope:
   - `ok`
   - `error_message`
   - `error_code`
   - `error_category`
   - `hints`
2. Successful TXT actions may add action-specific fields beside the standard
   envelope fields.

## Actions

### `default_day_marker`

Request:

```json
{
  "action": "default_day_marker",
  "selected_month": "2025-02",
  "target_date_iso": "2025-02-28"
}
```

Response:

```json
{
  "ok": true,
  "normalized_day_marker": "0228",
  "error_message": ""
}
```

Rules:

1. `selected_month` should be `YYYY-MM` when present.
2. `target_date_iso` should be `YYYY-MM-DD`.
3. When `selected_month` is valid, the returned `MMDD` is clipped to that
   month's max day.
4. When `selected_month` is missing or invalid, the runtime falls back to the
   month/day from `target_date_iso`.

### `resolve_day_block`

Request:

```json
{
  "action": "resolve_day_block",
  "content": "y2025\nm01\n\nd0101\n...\nd0102\n...\n",
  "day_marker": "0102",
  "selected_month": "2025-01"
}
```

Response:

```json
{
  "ok": true,
  "normalized_day_marker": "0102",
  "found": true,
  "is_marker_valid": true,
  "can_save": true,
  "day_body": "0656w\n0904无氧训练 // cherry\n2207minecraft\n",
  "day_content_iso_date": "2025-01-02",
  "error_message": ""
}
```

Rules:

1. `content` is the full month TXT content, not a file path.
2. `found=false` means the marker is valid but no day block was present in the
   supplied content.
3. `is_marker_valid=false` means the normalized marker is not a legal `MMDD`.
4. `can_save=true` only when the marker is valid and the day block exists.
5. `day_body` is empty when the block is missing or the marker is invalid.
6. `day_content_iso_date` is optional and omitted when it cannot be derived.

### `replace_day_block`

Request:

```json
{
  "action": "replace_day_block",
  "content": "y2025\nm01\n\nd0101\n...\nd0102\n...\n",
  "day_marker": "0102",
  "edited_day_body": "d0102\n0656w\n0904无氧训练 // cherry\n"
}
```

Response:

```json
{
  "ok": true,
  "normalized_day_marker": "0102",
  "found": true,
  "is_marker_valid": true,
  "updated_content": "y2025\nm01\n\nd0101\n...\nd0102\n0656w\n0904无氧训练 // cherry\n",
  "error_message": ""
}
```

Rules:

1. `edited_day_body` replaces the day-block body only; the runtime keeps the
   marker line in the full month content.
2. A duplicated leading marker line equal to `d` plus the normalized
   `day_marker` is removed before merge.
3. `updated_content` is omitted when `found=false` or `is_marker_valid=false`.
4. The runtime does not create a new block when the requested block is missing.

### `convert_activity_names`

This action converts activity names in the supplied full month TXT content. It
does not read or write a file and does not apply to an individual DAY draft.

Request:

```json
{
  "action": "convert_activity_names",
  "content": "y2026\nm01\n\n0830英语单词 // keep remark\n",
  "direction": "alias_to_canonical"
}
```

Response:

```json
{
  "ok": true,
  "converted_content": "y2026\nm01\n\n0830study_english_words // keep remark\n",
  "error_message": ""
}
```

Rules:

1. `direction` is either `alias_to_canonical` or `canonical_to_alias`.
2. Names already in the requested representation remain unchanged.
3. Headers, times, remarks, blank lines, and wake keywords are preserved.
4. Canonical-to-alias selection is deterministic when a canonical name has
   multiple aliases.
5. `content` is the current full month draft supplied by the host; persistence
   remains an explicit host-side ingest/save operation.

### `replace_canonical_activity_names`

This action is for configuration hierarchy migrations. It replaces only the
exact canonical activity tokens specified by the host; it does not convert
aliases or remarks.

Request:

```json
{
  "action": "replace_canonical_activity_names",
  "content": "y2026\nm01\n\n0830exercise_walk // remark\n",
  "replacements": [
    {
      "old_canonical": "exercise_walk",
      "new_canonical": "exercise_cardio_walk"
    }
  ]
}
```

Response:

```json
{
  "ok": true,
  "updated_content": "y2026\nm01\n\n0830exercise_cardio_walk // remark\n",
  "error_message": ""
}
```

Rules:

1. Each replacement source and target must be non-empty; a source appears at
   most once in a request.
2. Only parsed event-line activity tokens are eligible. Headers, times,
   whitespace, remarks, aliases, and unrelated canonical paths are preserved.
3. The runtime remains file-system agnostic; the host owns source-file writes,
   candidate database construction, replacement, and rollback.

### `replace_alias_activity_names`

This action is for alias-key migrations. It replaces only exact authored alias
tokens specified by the host; canonical activity names, remarks, headers, and
other TXT structure remain unchanged.

Request:

```json
{
  "action": "replace_alias_activity_names",
  "content": "y2026\nm01\n\n0830有氧 // remark\n",
  "replacements": [
    {
      "old_alias": "有氧",
      "new_alias": "有氧aa"
    }
  ]
}
```

The response uses the same `updated_content` result shape as
`replace_canonical_activity_names`. Persistence and database re-ingest remain
host-side migration responsibilities.

## Activity hierarchy TOML

An alias group may itself be recordable. Its `group_aliases` string array maps
each alias directly to the canonical path represented by the current group;
child string entries append their canonical leaf as usual.

```toml
parent = "recreation"

[canonical.online]
group_aliases = ["上网"]
"bilibili" = ["哔哩哔哩"]
"douyin" = ["抖音"]
```

The resulting canonical paths are `recreation_online`,
`recreation_online_bilibili`, and `recreation_online_douyin`. The
`group_aliases` key is reserved inside group tables and is not an activity leaf.

### Android alias-key hierarchy migration

1. Android sends a hierarchy operation or raw TOML rewrite request to Core and
   receives the updated canonical TOML together with `replacements[]` and
   `alias_replacements[]`.
2. Android passes both replacement lists to the Core TXT actions for every
   managed TXT file through `RuntimeActivityHierarchyMigrationService`.
3. The service writes candidate TOML and rewritten TXT files, rebuilds an
   isolated database by ingesting the candidate data, and swaps it in only
   after successful ingestion.
4. On failure the service restores the source files and the previous database.

## Cross-Layer Call Chains

### Android DAY mode

1. Compose screen keeps UI state such as mode, raw marker input, and visibility.
2. Android runtime service encodes a TXT action request and forwards it through
   JNI.
3. JNI/native bridge calls `tracer_core_runtime_config_json`.
4. Shell C ABI decodes the action and forwards it into `tracer_core` pipeline
   TXT day-block APIs.
5. Core resolves or replaces the block and returns JSON for the Android UI to
   render.

### Android ALL-mode activity-name conversion

1. The TXT editor shows mutually exclusive Alias/Canonical controls only in ALL
   mode.
2. Android sends the current selected month draft through
   `convert_activity_names`.
3. Android replaces only the ALL month draft with `converted_content`; it does
   not alter DAY draft state or save automatically.
4. The existing Ingest action persists the converted month TXT.

### Android canonical hierarchy migration

1. Android sends the move operation to Core; Core computes the updated TOML and
   all affected canonical replacements (including descendants when a hierarchy
   path changes).
2. `RuntimeActivityHierarchyMigrationService` sends those replacements to
   `replace_canonical_activity_names`, writes the candidate TOML and TXT files,
   and builds an isolated temporary database by full ingest.
3. On any failure the service restores source files and retains/restores the
   prior database before reinitializing the active runtime.

### Windows CLI `txt view-day`

1. CLI parses arguments and reads the TXT file locally.
2. CLI infers `selected_month` from `YYYY-MM.txt` when possible; otherwise it
   sends an empty value.
3. CLI calls `tracer_core_runtime_config_json` with the full file content.
4. Core resolves the target day block and returns JSON.
5. CLI prints `day_body` on success or a host-formatted error on failure.

## Related Tests

1. Core semantic tests:
   - `libs/tracer_core/tests/application/tests/modules/txt_day_block_tests.cpp`
2. Shell / C ABI tests:
   - `apps/tracer_core_shell/tests/integration/tracer_core_c_api_pipeline_tests.cpp`
   - `apps/tracer_core_shell/tests/integration/tracer_core_c_api_error_tests.cpp`
   - `apps/tracer_core_shell/tests/integration/tracer_core_c_api_smoke_tests.cpp`
3. Android runtime/client tests:
   - `apps/android/runtime/src/test/java/com/example/tracer/NativeTxtRuntimeCodecTest.kt`
   - `apps/android/runtime/src/test/java/com/example/tracer/RuntimeTxtActivityNameServiceTest.kt`
4. Windows CLI black-box suite:
   - `tools/suites/tracer_windows_rust_cli/tests/commands_txt_view_day.toml`
   - stage/log group: `txt-view-day`
