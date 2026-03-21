# tracer_core_bridge_common

## Purpose

Local entrypoint for agents touching the shared bridge-helper layer.

## When To Open

- Open this when the task touches `libs/tracer_core_bridge_common`.
- Use it to jump into the detailed bridge-helper doc and shell-facing contracts.

## What This Doc Does Not Cover

- Detailed bridge ownership notes
- Downstream shell/runtime routing
- Full validation and regression paths

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core_bridge_common.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core_bridge_common apps/tracer_core_shell libs/tracer_transport
python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise
```
