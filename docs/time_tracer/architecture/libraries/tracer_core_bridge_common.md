# tracer_core_bridge_common

## Purpose

`tracer_core_bridge_common` hosts the remaining shared helper logic for runtime
bridge code. After `Goal 2 / Phase 3 / Wave 3`, it is intentionally narrowed to
transport-envelope helpers used by JNI-facing bridge code, not app-owned parse
or crypto-progress boundaries.

## Layer Position

1. This library sits beside host adapters, not inside the business core.
2. It may depend on `tracer_transport` for envelope utilities.
3. App-owned shell bridges now own C API parse helper and crypto-progress JSON
   projection boundaries.
4. It is a helper layer, not a contract-owner layer.

## Allowed Dependencies

1. `libs/tracer_transport` envelope helpers
2. `nlohmann_json` where a bridge helper needs envelope normalization

## Forbidden Dependencies

1. Do not place business rules or use case orchestration here.
2. Do not make this library the owner of runtime request/response schema.
3. Do not move app lifecycle/context management here.

## Public Surfaces

1. `src/jni/bridge_utils.hpp`
   - shared JNI response helpers and response-envelope parsing

## Physical Layout

1. `src/jni`
2. `tests`

## Hotspot Families

| Family | Start Here | Notes |
| --- | --- | --- |
| JNI bridge helpers | `src/jni/bridge_utils.*` | Response envelope parsing and envelope normalization |

## Change Routing

1. Change response-envelope parsing or bridge-side JSON wrapper behavior:
   - start in `src/jni/bridge_utils.*`
   - pair with `tracer_transport.md`
2. Change C API parse helper ownership or string-to-core mapping:
   - start in `apps/tracer_core_shell/api/c_api/c_api_parse_bridge.*`
   - if external wire values change, also update the relevant contract docs
3. Change JNI int-code to wire-value translation:
   - start in `apps/tracer_core_shell/api/android_jni/jni_runtime_code_bridge.*`
4. Change crypto progress callback payload projection:
   - start in `apps/tracer_core_shell/host/crypto_progress_bridge.*`
   - pair with core crypto contract/runtime docs if callback payload meaning changes
5. If the change touches business rules, app context, or runtime orchestration:
   - leave this library and start in `tracer_core.md` or
     `../../core/specs/AGENT_ONBOARDING.md`

## Tests / Validate Entry Points

1. There are currently no dedicated library-only tests under `tests/`.
2. Validate through downstream shell/runtime paths:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core_bridge_common apps/tracer_core_shell libs/tracer_transport
```

3. Shell/runtime integration shortcut:

```powershell
python tools/run.py verify --app tracer_core_shell --quick --scope batch --concise
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [Core Agent Onboarding](../../core/specs/AGENT_ONBOARDING.md)
3. [tracer_transport](tracer_transport.md)
4. [C ABI Contract](../../core/contracts/c_abi.md)
5. [Android Runtime Protocol](../../clients/android_ui/runtime-protocol.md)
