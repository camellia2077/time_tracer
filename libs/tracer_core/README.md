# tracer_core

## Purpose

Local entrypoint for agents touching the core business-logic library.

## When To Open

- Open this when the task touches `libs/tracer_core`.
- Use it to jump into the detailed library doc and the cross-library map.

## What This Doc Does Not Cover

- Internal layer routing inside the library
- Detailed boundary rules
- Full validation strategy

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [Core agent onboarding](../../docs/time_tracer/core/specs/AGENT_ONBOARDING.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core apps/tracer_core_shell
python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise
```
