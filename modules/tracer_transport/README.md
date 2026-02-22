# tracer_transport module

## Purpose

`tracer_transport` provides shared transport utilities for runtime JSON boundaries.
It is used by core C ABI adapters and host adapters to avoid duplicated codec and envelope logic.

## Layer Position

1. Contract layer (what fields mean) is documented in:
   - `docs/time_tracer/core/c_abi.md`
   - `docs/time_tracer/android_ui/runtime-protocol.md`
2. This module is implementation layer (how to parse/serialize/normalize).
3. This module does not execute business use cases, does not access database, and does not contain domain rules.

## Public API Surface

1. Envelope utilities: `include/tracer/transport/envelope.hpp`
   - `BuildResponseEnvelope`
   - `SerializeResponseEnvelope`
   - `ParseResponseEnvelope`
2. Field readers: `include/tracer/transport/fields.hpp`
   - `RequireStringField`
   - `TryReadStringField`
   - `TryReadBoolField`
   - `TryReadIntField`
   - `TryReadIntListField`
3. Runtime payload DTOs:
   - `include/tracer/transport/runtime_requests.hpp`
   - `include/tracer/transport/runtime_responses.hpp`
4. Runtime codec functions: `include/tracer/transport/runtime_codec.hpp`
   - decode/encode request payloads for ingest/query/report/report_batch/export/tree
   - encode response payloads for ingest/query/report/report_batch/export/tree

## Current Coverage

1. Shared request codec is available for:
   - ingest/query/report/report_batch/export/tree
2. Shared response envelope parser is available for all runtime operations through `ParseResponseEnvelope`.
3. `convert/import/validate_*` currently have no dedicated request codec in this module.
   - callers may assemble request JSON locally
   - response parsing should still reuse envelope parser

## Compatibility Rules

1. Envelope keys are stable: `ok`, `error_message`, `content`.
2. Evolve request/response JSON with additive optional fields where possible.
3. Keep parser error messages explicit to help CLI/JNI surface concrete failures.
4. Do not introduce ABI-level behavior changes here without updating contract docs first.

## Tests

1. `modules/tracer_transport/tests/transport_envelope_tests.cpp`
2. `modules/tracer_transport/tests/transport_fields_tests.cpp`
3. `modules/tracer_transport/tests/transport_runtime_codec_tests.cpp`

## Main Sources

1. `modules/tracer_transport/src/envelope.cpp`
2. `modules/tracer_transport/src/fields.cpp`
3. `modules/tracer_transport/src/runtime_codec_ingest.cpp`
4. `modules/tracer_transport/src/runtime_codec_query.cpp`
5. `modules/tracer_transport/src/runtime_codec_report.cpp`
6. `modules/tracer_transport/src/runtime_codec_export.cpp`
7. `modules/tracer_transport/src/runtime_codec_tree.cpp`
