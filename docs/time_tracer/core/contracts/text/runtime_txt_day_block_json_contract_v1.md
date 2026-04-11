# Runtime TXT Day-Block JSON Contract v1

## Scope

1. This document defines the host-facing JSON contract for
   `tracer_core_runtime_txt_json`.
2. The contract covers month-TXT day-block semantics only.
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
3. Day-block extraction never includes the marker line itself in `day_body`.
4. Day-block replacement removes a duplicated leading `MMDD` marker when the
   edited body starts with the same marker.
5. Day-block replacement preserves user-authored trailing blank lines.
6. `day_content_iso_date` is only produced when both `selected_month` and the
   normalized `day_marker` are valid.

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
  "content": "y2025\nm01\n\n0101\n...\n0102\n...\n",
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
  "day_body": "0656w\n0904无氧训练 #cherry\n2207minecraft\n",
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
  "content": "y2025\nm01\n\n0101\n...\n0102\n...\n",
  "day_marker": "0102",
  "edited_day_body": "0102\n0656w\n0904无氧训练 #cherry\n"
}
```

Response:

```json
{
  "ok": true,
  "normalized_day_marker": "0102",
  "found": true,
  "is_marker_valid": true,
  "updated_content": "y2025\nm01\n\n0101\n...\n0102\n0656w\n0904无氧训练 #cherry\n",
  "error_message": ""
}
```

Rules:

1. `edited_day_body` replaces the day-block body only; the runtime keeps the
   marker line in the full month content.
2. A duplicated leading marker line equal to the normalized `day_marker` is
   removed before merge.
3. `updated_content` is omitted when `found=false` or `is_marker_valid=false`.
4. The runtime does not create a new block when the requested block is missing.

## Cross-Layer Call Chains

### Android DAY mode

1. Compose screen keeps UI state such as mode, raw marker input, and visibility.
2. Android runtime service encodes a TXT action request and forwards it through
   JNI.
3. JNI/native bridge calls `tracer_core_runtime_txt_json`.
4. Shell C ABI decodes the action and forwards it into `tracer_core` pipeline
   TXT day-block APIs.
5. Core resolves or replaces the block and returns JSON for the Android UI to
   render.

### Windows CLI `txt view-day`

1. CLI parses arguments and reads the TXT file locally.
2. CLI infers `selected_month` from `YYYY-MM.txt` when possible; otherwise it
   sends an empty value.
3. CLI calls `tracer_core_runtime_txt_json` with the full file content.
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
4. Windows CLI black-box suite:
   - `tools/suites/tracer_windows_rust_cli/tests/commands_txt_view_day.toml`
   - stage/log group: `txt-view-day`
