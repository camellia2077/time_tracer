# tracer_adapters_io

## Purpose

`tracer_adapters_io` provides file-system and processed-data adapter
implementations used around the core runtime.
It hosts file enumeration, file read/write helpers, ingest input collection,
and processed-data persistence helpers.

## Layer Position

1. This library is an infrastructure adapter layer.
2. It follows contracts owned by `tracer_core`; it does not define core business rules.
3. It may depend on `tracer_transport` for shared wire/runtime helpers when needed.

## Allowed Dependencies

1. `libs/tracer_core`
   - application ports and shared/core serialization helpers
2. `libs/tracer_transport`
   - shared transport/wire helpers where required by adapter behavior
3. Filesystem and JSON implementation dependencies

## Forbidden Dependencies

1. Do not move domain or application ownership into this library.
2. Do not place C ABI / JNI runtime glue here.
3. Do not treat this library as the owner of runtime request/response contracts.

## Public Surfaces

1. Canonical module surfaces:
   - `tracer.adapters.io`
   - `tracer.adapters.io.core.reader`
   - `tracer.adapters.io.core.writer`
   - `tracer.adapters.io.core.fs`
   - `tracer.adapters.io.runtime`
   - `tracer.adapters.io.utils.file_utils`
2. Concrete runtime adapter declarations now live under
   `src/infrastructure/io/internal/runtime_adapter_types.hpp` and are
   implementation-owned, not consumer-facing contract.

## Physical Layout

1. `src/infrastructure/io`
2. `src/infrastructure/io/core`
3. `src/infrastructure/io/internal`
4. `src/infrastructure/io/utils`
5. `src/modules`
6. `tests`

## Hotspot Families

| Family | Start Here | Also Check |
| --- | --- | --- |
| TXT ingest input collection | `src/infrastructure/io/txt_ingest_input_provider.cpp` | `src/infrastructure/io/core/file_reader.*` |
| Processed-data persistence | `src/infrastructure/io/processed_data_io.cpp` | `src/infrastructure/io/processed_json_validation.*`, `../../../libs/tracer_core/src/infrastructure/serialization` |
| Runtime adapter concrete declarations | `src/infrastructure/io/internal/runtime_adapter_types.hpp` | `src/infrastructure/io/processed_data_io.cpp`, `src/infrastructure/io/txt_ingest_input_provider.cpp` |
| Low-level file read/write/fs helpers | `src/infrastructure/io/core` | `src/infrastructure/io/utils` |
| File/path utility helpers | `src/infrastructure/io/utils/file_utils.*` | module bridge `tracer.adapters.io.utils.file_utils` |
| Runtime port bridge | `src/modules/tracer.adapters.io.runtime.cppm` | `apps/tracer_core_shell/host/android_runtime_factory.cpp` |
| Module coverage | `src/modules` | `tests/adapters_io_modules_smoke_tests.cpp` |

## Change Routing

1. Change raw file enumeration, read/write behavior, or filesystem edge cases:
   - start in `src/infrastructure/io/core`
   - then inspect `src/infrastructure/io/utils/file_utils.*`
2. Change TXT ingest source collection:
   - start in `src/infrastructure/io/txt_ingest_input_provider.cpp`
   - then inspect `src/infrastructure/io/internal/runtime_adapter_types.hpp`
   - then inspect `src/infrastructure/io/core/file_reader.*`
3. Change processed-data persistence or validation:
   - start in `src/infrastructure/io/processed_data_io.cpp`
   - then inspect `src/infrastructure/io/internal/runtime_adapter_types.hpp`
   - then inspect `src/infrastructure/io/processed_json_validation.*`
   - pair with `../../../libs/tracer_core/src/infrastructure/serialization`
     if serialization format changes
4. Change runtime assembly bridge or other module exports:
   - inspect `src/modules`
   - inspect module smoke tests

## Tests / Validate Entry Points

1. Module smoke test:
   - `tests/adapters_io_modules_smoke_tests.cpp`
2. Focused repo validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_adapters_io apps/tracer_core_shell
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [tracer_core](tracer_core.md)
3. [Core JSON Boundary Design](../../core/architecture/core_json_boundary_design.md)
