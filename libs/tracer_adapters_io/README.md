# tracer_adapters_io

## Purpose

Local entrypoint for agents touching the IO adapter library.

## When To Open

- Open this when the task touches `libs/tracer_adapters_io`.
- Use it to jump into the detailed adapter doc and the cross-library map.

## What This Doc Does Not Cover

- Internal adapter-family routing
- Detailed module ownership notes
- Full validation inventory

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_adapters_io.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [Detailed `tracer_core` doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)

## Validation Boundary

- `tracer_adapters_io` does not own TXT/TOML business validation.
- For converter-config validation and TXT structure/logic validation, open [libs/tracer_core/README.md](../tracer_core/README.md) and follow its `Validation Docs` section.

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_adapters_io apps/tracer_core_shell
```
