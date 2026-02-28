# Infrastructure Tests Layout

This directory hosts infrastructure-level tests for `apps/tracer_core`.
Use this file as the fast navigation index for agents.

## Directory Map (By Responsibility)

- `android_runtime/`
  - Android runtime bootstrap/config/compat/bundle policy tests.
  - Entry: `android_runtime/android_runtime_test_main.cpp`
  - Shared helper: `android_runtime/android_runtime_test_common.hpp/.cpp`

- `data_query/`
  - DataQuery refactor boundary and regression tests.
  - Files:
    - `data_query/data_query_refactor_period_tests.cpp`
    - `data_query/data_query_refactor_tree_tests.cpp`
    - `data_query/data_query_refactor_stats_tests.cpp`
    - `data_query/data_query_refactor_test_internal.hpp`
  - Runner entry: `RunDataQueryRefactorTests(int& failures)` in
    `data_query/data_query_refactor_period_tests.cpp`

- `report_formatter/`
  - Markdown/LaTeX/Typst parity snapshot tests.
  - Files:
    - `report_formatter/report_formatter_parity_md_tests.cpp`
    - `report_formatter/report_formatter_parity_tex_tests.cpp`
    - `report_formatter/report_formatter_parity_typ_tests.cpp`
    - `report_formatter/report_formatter_parity_internal.hpp`

- `file_crypto/`
  - File crypto roundtrip/failure/progress/interop tests.
  - Entry: `file_crypto/file_crypto_test_main.cpp`
  - Group dispatcher: `file_crypto/file_crypto_service_tests.cpp`
  - Shared helper:
    - `file_crypto/file_crypto_service_test_internal.hpp`
    - `file_crypto/file_crypto_service_test_common.cpp`

- Flat files kept at current level:
  - `validation_issue_reporter_tests.cpp`
  - `txt_month_header_tests.cpp`

## CMake Wiring (Authoritative)

Test target source lists are defined in:

- `apps/tracer_core/src/CMakeLists.txt`

When moving/adding test files, always update this file first.

## Placement Rules

1. Put tests into the responsibility folder that owns the behavior.
2. Only place shared helper code in `*_internal.hpp`/`*_common.cpp` when reused by at least two test files in the same responsibility area.
3. Keep `*_test_main.cpp` as dispatcher only; no case logic in `main()`.
4. Keep tests self-cleaning (`RemoveTree(...)`) and avoid cross-test shared mutable state.

## Validation Checklist

1. `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
2. `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise`
3. `rg -n "infrastructure/tests/(android_runtime|data_query|report_formatter|file_crypto)" apps/tracer_core/src`
