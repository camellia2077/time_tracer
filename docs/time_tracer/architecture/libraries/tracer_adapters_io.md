# tracer_adapters_io

## Purpose

Detailed navigation for the IO adapter library used around the core runtime.

## When To Open

- Open this when the task changes ingest input collection, filesystem helpers, or processed-data persistence adapters.
- Open this after `library_dependency_map.md` once IO adapter ownership is confirmed.

## What This Doc Does Not Cover

- Core business-rule routing
- C ABI / JNI glue behavior
- Full implementation walkthroughs

## Ownership

1. `tracer_adapters_io` is an adapter layer around core-owned ports.
2. It owns filesystem-facing helpers, ingest input collection, and processed-data persistence helpers.
3. It does not own core business rules, runtime contracts, or host lifecycle glue.

## TXT Runtime Family Index

1. `tracer_adapters_io` may read and write month TXT files, but it does not own
   day-block extraction, replacement, or default `MMDD` semantics.
2. TXT runtime action meaning remains in `tracer_core`.
3. Hosts may combine IO adapter file reads with TXT runtime actions, but that
   composition does not move business ownership into this library.

## Allowed Dependencies

1. `libs/tracer_core`
2. `libs/tracer_transport`
3. Filesystem and JSON implementation dependencies

## Forbidden Dependencies

1. Do not move domain or application ownership here.
2. Do not place C ABI / JNI runtime glue here.
3. Do not treat this library as the owner of runtime request/response contracts.

## Public Surfaces

1. Canonical module surfaces under `src/modules`
2. Runtime adapter declarations under `src/infra/io/internal/runtime_adapter_types.hpp`
   remain implementation-owned, not consumer-facing contracts

## Change Routing

1. Change raw file enumeration, file read/write behavior, or filesystem edge cases:
   - start in `src/infra/io/core`
   - then inspect `src/infra/io/utils/file_utils.*`
2. Change TXT ingest source collection:
   - start in `src/infra/io/txt_ingest_input_provider.cpp`
   - then inspect `src/infra/io/internal/runtime_adapter_types.hpp`
3. Change processed-data persistence or validation:
   - start in `src/infra/io/processed_data_io.cpp`
   - then inspect `src/infra/io/processed_json_validation.*`
   - pair with core serialization if format behavior changes
4. Change module exports or runtime bridge assembly:
   - inspect `src/modules`
   - then inspect downstream shell/runtime assembly
5. If the task is about viewing or editing a month TXT by `MMDD`:
   - keep file read/write concerns here only if filesystem behavior changes
   - move day-block parsing or replacement changes to `tracer_core.md`

## Tests / Validate Entry Points

1. Module smoke coverage lives in `tests/adapters_io_modules_smoke_tests.cpp`.
2. TXT host-flow coverage that passes through this library is validated
   downstream in:
   - `tools/suites/tracer_windows_rust_cli/tests/commands_txt_view_day.toml`
   - `apps/tracer_core_shell/tests/integration/tracer_core_c_api_pipeline_tests.cpp`
3. Focused validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_adapters_io apps/tracer_core_shell
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [tracer_core](tracer_core.md)
3. [Core JSON Boundary Design](../../core/architecture/core_json_boundary_design.md)
4. [TXT Runtime JSON Contract](../../core/contracts/text/runtime_txt_day_block_json_contract_v1.md)
