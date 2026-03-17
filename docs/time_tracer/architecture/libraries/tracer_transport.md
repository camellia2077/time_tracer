# tracer_transport

## Purpose

`tracer_transport` provides shared transport utilities for runtime JSON
boundaries. It centralizes response envelopes, field readers, runtime DTOs,
and request/response codec logic so host adapters do not duplicate wire-layer
behavior.

## Layer Position

1. Contract meaning lives in docs, not in this library.
2. This library is the implementation layer for runtime payload parsing and serialization.
3. This library does not own business rules, database behavior, or host runtime lifecycle.

## Allowed Dependencies

1. Standard library
2. `nlohmann_json`

## Forbidden Dependencies

1. Do not place `tracer_core` business logic here.
2. Do not place JNI/C API app lifecycle behavior here.
3. Do not make this library the source of truth for ABI semantics without updating contract docs.

## Public Surfaces

1. Envelope helpers:
   - `include/tracer/transport/envelope.hpp`
2. Field readers:
   - `include/tracer/transport/fields.hpp`
3. Runtime DTOs:
   - `include/tracer/transport/runtime_requests.hpp`
   - `include/tracer/transport/runtime_responses.hpp`
4. Runtime codec API:
   - `include/tracer/transport/runtime_codec.hpp`
5. Module bridges:
   - `tracer.transport`
   - `tracer.transport.envelope`
   - `tracer.transport.fields`
   - `tracer.transport.errors`

## Physical Layout

1. `include/tracer/transport`
2. `src`
3. `src/modules`
4. `tests`

## Hotspot Families

| Family | Start Here | Also Check |
| --- | --- | --- |
| Response envelope behavior | `include/tracer/transport/envelope.hpp`, `src/envelope.cpp` | `tests/transport_envelope_tests.cpp` |
| Field parsing helpers | `include/tracer/transport/fields.hpp`, `src/fields.cpp` | `tests/transport_fields_tests.cpp` |
| Runtime request/response DTO surfaces | `include/tracer/transport/runtime_requests.hpp`, `include/tracer/transport/runtime_responses.hpp` | contract docs |
| Operation-specific codec logic | `src/runtime_codec_*.cpp` | `tests/transport_runtime_codec_*.cpp` |
| Module surface coverage | `src/modules` | `tests/transport_modules_smoke_tests.cpp` |

## Change Routing

1. Change envelope keys or response wrapper behavior:
   - start in `include/tracer/transport/envelope.hpp` and `src/envelope.cpp`
   - update contract docs first if public behavior changes
2. Change field parsing or normalization rules:
   - start in `include/tracer/transport/fields.hpp` and `src/fields.cpp`
3. Change runtime request/response payload shape:
   - start in `include/tracer/transport/runtime_requests.hpp`,
     `include/tracer/transport/runtime_responses.hpp`, and the matching
     `src/runtime_codec_*.cpp`
   - read ABI/runtime protocol docs first
4. Change operation-specific codec behavior:
   - inspect the matching source:
     - `runtime_codec_ingest.cpp`
     - `runtime_codec_runtime.cpp`
     - `runtime_codec_capabilities.cpp`
     - `runtime_codec_workflow.cpp`
     - `runtime_codec_query.cpp`
     - `runtime_codec_report.cpp`
     - `runtime_codec_export.cpp`
     - `runtime_codec_tree.cpp`
5. Change module surface coverage:
   - inspect `src/modules`
   - inspect `tests/transport_modules_smoke_tests.cpp`

## Tests / Validate Entry Points

1. `tests/transport_envelope_tests.cpp`
2. `tests/transport_fields_tests.cpp`
3. `tests/transport_runtime_codec_decode_request_tests.cpp`
4. `tests/transport_runtime_codec_decode_response_tests.cpp`
5. `tests/transport_runtime_codec_encode_tests.cpp`
6. `tests/transport_runtime_codec_tests.cpp`
7. `tests/transport_modules_smoke_tests.cpp`
8. Focused repo validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_transport apps/tracer_core_shell
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [C ABI Contract](../../core/contracts/c_abi.md)
3. [Android Runtime Protocol](../../clients/android_ui/runtime-protocol.md)
