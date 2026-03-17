# tracer_transport

`tracer_transport` is the transport implementation library for runtime JSON
boundaries. It owns envelopes, field readers, runtime DTO helpers, and codec
implementation details, but it does not own core business rules or host
runtime lifecycle.

## Start Here
1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_transport.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_transport apps/tracer_core_shell
```

## Read-First Docs
1. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)
2. [Android runtime protocol](../../docs/time_tracer/clients/android_ui/runtime-protocol.md)
