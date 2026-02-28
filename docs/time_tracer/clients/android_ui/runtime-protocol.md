# tracer_android Runtime Protocol

## Scope

This document defines Android runtime boundary contracts:

1. Android UI/Kotlin to JNI method boundary.
2. JNI internal calls to core C ABI (`tracer_core_*`).
3. Business payload protocol used between JNI and core (JSON string).

## Boundary Model

Runtime call chain:

1. Kotlin/Compose -> JNI native methods (`nativeInit/nativeIngest/nativeQuery/nativeTree/nativeReport`)
2. JNI -> core C ABI (`tracer_core_*`)
3. C ABI -> core/application/infrastructure implementation

Important:

1. Android app does not call `tracer_core_*` directly from Kotlin.
2. Kotlin-facing API remains JNI methods.
3. JNI-to-core business payload is UTF-8 JSON (`const char*`).

## C ABI Contract

Android JNI currently uses these C ABI entrypoints:

1. `tracer_core_runtime_create`
2. `tracer_core_runtime_destroy`
3. `tracer_core_runtime_ingest_json`
4. `tracer_core_runtime_query_json`
5. `tracer_core_runtime_tree_json`
6. `tracer_core_runtime_report_json`
7. `tracer_core_runtime_report_batch_json`
8. `tracer_core_runtime_validate_structure_json`
9. `tracer_core_runtime_validate_logic_json`
10. `tracer_core_last_error`

Canonical C ABI naming and global rules:

1. `docs/time_tracer/core/contracts/c_abi.md`

## Transport Status (2026-02-19)

1. JNI request encoding has been unified through `tracer_transport` for:
   - `nativeIngest` -> `EncodeIngestRequest`
   - `nativeQuery` -> `EncodeQueryRequest`
   - `nativeTree` -> `EncodeTreeRequest`
   - `nativeReport` (single) -> `EncodeReportRequest`
   - `nativeReport` (batch) -> `EncodeReportBatchRequest`
2. `nativeValidateStructure` and `nativeValidateLogic` request JSON is still assembled locally in JNI.
3. `nativeTree` response uses `DecodeTreeResponse` after envelope parse, then JNI returns normalized envelope (`ok/error_message/content`) where `content` carries tree payload JSON.
4. Core responses are parsed through `ParseResponseEnvelope`, then JNI returns a normalized JSON string via `SerializeResponseEnvelope`.
5. Kotlin-visible response shape remains stable: `{ok,error_message,content}`.
6. JNI native method signatures are unchanged.

## JSON Payload Contract (JNI -> core)

### Common response envelope

All JNI responses are normalized to:

1. `ok` (`bool`)
2. `error_message` (`string`)
3. `content` (`string`, optional/operation-specific)

### `nativeInit`

1. JNI receives typed parameters: `dbPath`, `outputRoot`, `converterConfigTomlPath`.
2. JNI calls `tracer_core_runtime_create(...)`.
3. No JSON request is required for create; errors are read from `tracer_core_last_error()`.

### `nativeIngest`

JNI builds request via transport codec (`EncodeIngestRequest`) with fields:

1. `input_path` (`string`)
2. `date_check_mode` (`"none" | "continuity" | "full"`)
3. `save_processed_output` (`bool`)

Then calls `tracer_core_runtime_ingest_json(...)`.

### `nativeQuery`

JNI builds request via transport codec (`EncodeQueryRequest`) with `action` and optional query fields.
Typical fields include:

1. `action` (`years|months|days|days_duration|days_stats|search|activity_suggest|tree`)
2. date/period fields such as `year`, `month`, `from_date`, `to_date`
3. filter flags such as `overnight`, `reverse`, `activity_score_by_duration`

Then calls `tracer_core_runtime_query_json(...)`.

### `nativeTree`

JNI builds request via `EncodeTreeRequest` with optional fields:

1. `list_roots` (`bool`)
2. `root_pattern` (`string`)
3. `max_depth` (`int`)
4. `period` (`string`)
5. `period_argument` (`string`)
6. `root` (`string`)

Then JNI calls `tracer_core_runtime_tree_json(...)`, decodes tree payload (`DecodeTreeResponse`), and returns normalized envelope:

1. `ok`
2. `error_message`
3. `content` (tree payload JSON string with `found/roots/nodes`)

### `nativeReport`

Single report request is built via `EncodeReportRequest`:

1. `type` (`day|month|recent|week|year|range`)
2. `argument` (`string`)
3. `format` (`markdown|latex|typst`)

Batch report request is built via `EncodeReportBatchRequest`:

1. `days_list` (`int[]`)
2. `format` (`markdown|latex|typst`)

Then calls:

1. `tracer_core_runtime_report_json(...)` (single)
2. `tracer_core_runtime_report_batch_json(...)` (batch)

### `nativeValidateStructure` / `nativeValidateLogic`

1. JNI currently assembles request JSON locally.
2. Core response parsing and return envelope normalization still use `ParseResponseEnvelope` + `SerializeResponseEnvelope`.

## Compatibility Rules

1. Keep JNI method signatures stable unless Android UI contract must change.
2. Prefer additive JSON fields; avoid breaking existing required field semantics.
3. Keep response envelope (`ok/error_message/content`) stable for ViewModel parsing.
4. Android Tree 主链应优先消费结构化节点；`query data tree` 文本路径仅作兼容 fallback。
5. Any C ABI symbol change must follow `docs/time_tracer/core/contracts/c_abi.md`.

## Report Output Fidelity Rules

1. `nativeReport` 成功响应中的 `content` 必须原样透传到 Kotlin `ReportCallResult.outputText`。
2. JNI/runtime/translators 不允许对报告正文做 `trim()/replace()/normalize`。
3. Android 仅支持 `markdown` 报告格式（轻量化策略）；`latex/typst` 能力由 core/windows_cli 保持。
4. UI 渲染层可做展示解析，但不得回写或覆盖导出正文。
5. 规范来源：`docs/time_tracer/core/contracts/reporting/report_output_text_contract_v1.md`。

## References

1. `apps/tracer_core/src/api/android/native_bridge_calls.cpp`
2. `apps/tracer_core/src/api/android/native_bridge_registration.cpp`
3. `apps/tracer_core/src/api/core_c/time_tracer_core_c_api.h`
4. `modules/tracer_transport/include/tracer/transport/runtime_codec.hpp`
5. `modules/tracer_transport/include/tracer/transport/envelope.hpp`
6. `docs/time_tracer/clients/android_ui/architecture.md`
7. `docs/time_tracer/core/contracts/c_abi.md`
