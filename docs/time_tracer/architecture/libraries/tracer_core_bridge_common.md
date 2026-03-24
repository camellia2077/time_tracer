# tracer_core_bridge_common

## Purpose

Detailed navigation for the shared bridge-helper library.

## When To Open

- Open this when the task changes shared JNI/C API helper logic that still lives in `libs/tracer_core_bridge_common`.
- Open this after `library_dependency_map.md` once bridge-helper ownership is confirmed.

## What This Doc Does Not Cover

- Business-rule routing
- Runtime orchestration ownership
- Full downstream shell walkthroughs

## Ownership

1. This library hosts the remaining shared bridge-helper logic.
2. It is a helper layer beside host adapters, not a contract-owner or business-rule layer.
3. App-owned shell bridges own C API parse helpers and crypto-progress projection paths.

## Allowed Dependencies

1. `libs/tracer_transport` envelope helpers
2. `nlohmann_json` where bridge-side normalization needs it

## Forbidden Dependencies

1. Do not place business rules or use case orchestration here.
2. Do not move runtime request/response schema ownership here.
3. Do not move app lifecycle or context management here.

## Public Surfaces

1. `src/jni/bridge_utils.hpp`
2. `src/jni/bridge_utils.cpp`

## Change Routing

1. Change shared JNI response-envelope parsing:
   - start in `src/jni/bridge_utils.*`
   - pair with `tracer_transport.md`
2. Change C API parse helper ownership or string-to-core mapping:
   - start in `apps/tracer_core_shell/api/c_api/c_api_parse_bridge.*`
3. Change JNI int-code to wire-value translation:
   - start in `apps/tracer_core_shell/api/android_jni/jni_runtime_code_bridge.*`
4. Change crypto progress callback payload projection:
   - start in `apps/tracer_core_shell/host/crypto_progress_bridge.*`
5. If the change touches business logic or runtime orchestration:
   - leave this library and start in `tracer_core.md`

## Tests / Validate Entry Points

1. There are no dedicated library-only tests under `tests/` right now.
2. Validate through downstream shell/runtime paths:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core_bridge_common apps/tracer_core_shell libs/tracer_transport
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [tracer_transport](tracer_transport.md)
3. [C ABI Contract](../../core/contracts/c_abi.md)
4. [Android Runtime Protocol](../../presentation/android/runtime-protocol.md)
