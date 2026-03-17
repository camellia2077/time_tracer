# tracer_adapters_io

`tracer_adapters_io` is the file-system and processed-data adapter library
used around the core runtime. It owns file IO helpers, ingest input collection,
and processed-data persistence adapters, but it does not own core business
rules or runtime contract semantics. Runtime-facing consumers should prefer
the canonical module surface, especially `tracer.adapters.io.runtime`, instead
of including retained adapter headers directly. Concrete adapter declarations
now live under `src/infrastructure/io/internal/` as implementation detail.

## Start Here
1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_adapters_io.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_adapters_io apps/tracer_core_shell
```

## Read-First Docs
1. [Detailed `tracer_core` doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)
2. [Core JSON boundary design](../../docs/time_tracer/core/architecture/core_json_boundary_design.md)
