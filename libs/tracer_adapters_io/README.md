# tracer_adapters_io module

## Purpose

`tracer_adapters_io` provides IO adapter implementations used by `tracer_core` infrastructure.
It hosts file-system based ingest input collection and processed-data persistence helpers.

## Layer Position

1. This module belongs to infrastructure adapters.
2. Core (`domain + application`) must not depend on this implementation directory directly.
3. Core-facing contracts remain in `apps/tracer_core/src/application/ports/*`.

## Public Surfaces

1. Legacy headers (stable for existing call sites):
   - `infrastructure/io/file_io_service.hpp`
   - `infrastructure/io/txt_ingest_input_provider.hpp`
   - `infrastructure/io/processed_data_io.hpp`
2. C++20 module bridges (when `TT_ENABLE_CPP20_MODULES=ON` and effective):
   - `tracer.adapters.io`
   - `tracer.adapters.io.core.reader`
   - `tracer.adapters.io.core.writer`
   - `tracer.adapters.io.core.fs`
   - `tracer.adapters.io.utils.file_utils`

## Compatibility Rules

1. Keep legacy headers usable while introducing module bridge exports.
2. Do not force downstream call-site migration in the same phase.
3. Keep high-coupling JSON details in implementation units; avoid over-exporting them in module interfaces.

## Tests

1. Modules smoke test:
   - `libs/tracer_adapters_io/tests/adapters_io_modules_smoke_tests.cpp`
2. Legacy compatibility test (headers + modules coexistence):
   - `libs/tracer_adapters_io/tests/adapters_io_legacy_headers_compat_tests.cpp`
