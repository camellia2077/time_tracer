# tracer_core_bridge_common

`tracer_core_bridge_common` is the shared helper library for the remaining
runtime bridge helpers. After `Goal 2 / Phase 3 / Wave 3`, it mainly keeps the
JNI response-envelope helper path; C API parse helpers and crypto-progress JSON
projection now live in shell-owned bridge files.

## Start Here
1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core_bridge_common.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core_bridge_common apps/tracer_core_shell libs/tracer_transport
python tools/run.py verify --app tracer_core_shell --quick --scope batch --concise
```

## Read-First Docs
1. [Detailed `tracer_transport` doc](../../docs/time_tracer/architecture/libraries/tracer_transport.md)
2. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)
3. [Android runtime protocol](../../docs/time_tracer/clients/android_ui/runtime-protocol.md)
