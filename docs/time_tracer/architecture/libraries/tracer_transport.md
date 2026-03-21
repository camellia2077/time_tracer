# tracer_transport

## Purpose

Detailed navigation for the transport implementation library.

## When To Open

- Open this when the task changes runtime envelopes, field readers, runtime DTO helpers, or transport codec behavior.
- Open this after `library_dependency_map.md` once `tracer_transport` is the confirmed owner.

## What This Doc Does Not Cover

- ABI semantic ownership
- Business-rule changes
- Host runtime lifecycle behavior

## Ownership

1. `tracer_transport` owns transport implementation, not contract meaning.
2. It centralizes envelopes, field readers, runtime DTO helpers, and codec logic.
3. External contract meaning still lives in docs and host-facing boundaries.

## Allowed Dependencies

1. Standard library
2. `nlohmann_json`

## Forbidden Dependencies

1. Do not place `tracer_core` business rules here.
2. Do not place JNI/C API host lifecycle glue here.
3. Do not make this library the sole source of truth for ABI semantics.

## Public Surfaces

1. `include/tracer/transport/envelope.hpp`
2. `include/tracer/transport/fields.hpp`
3. `include/tracer/transport/runtime_requests.hpp`
4. `include/tracer/transport/runtime_responses.hpp`
5. `include/tracer/transport/runtime_codec.hpp`
6. Canonical modules under `src/modules`

## Change Routing

1. Change envelope wrapper behavior:
   - start in `include/tracer/transport/envelope.hpp` and `src/envelope.cpp`
2. Change field parsing or normalization:
   - start in `include/tracer/transport/fields.hpp` and `src/fields.cpp`
3. Change runtime request/response DTO helpers:
   - start in `include/tracer/transport/runtime_requests.hpp` and `include/tracer/transport/runtime_responses.hpp`
4. Change operation codec behavior:
   - start in the matching `src/runtime_codec_*.cpp` family
   - treat these as operation codec entrypoints, not as a complete file checklist
5. Change module surfaces:
   - inspect `src/modules` and module smoke tests

## Tests / Validate Entry Points

1. Envelope, field, runtime codec, and module smoke tests live under `tests/`.
2. Focused validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_transport apps/tracer_core_shell
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [C ABI Contract](../../core/contracts/c_abi.md)
3. [Android Runtime Protocol](../../presentation/android/runtime-protocol.md)
