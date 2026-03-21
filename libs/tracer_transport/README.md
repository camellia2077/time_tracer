# tracer_transport

## Purpose

Local entrypoint for agents touching the shared transport implementation layer.

## When To Open

- Open this when the task touches `libs/tracer_transport`.
- Use it to jump into the detailed library doc and ABI/runtime contract docs.

## What This Doc Does Not Cover

- Codec-family routing details
- Transport boundary rules
- Full test inventory

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_transport.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_transport apps/tracer_core_shell
```
