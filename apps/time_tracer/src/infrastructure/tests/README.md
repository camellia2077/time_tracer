# Infrastructure Tests Layout

This folder hosts integration-style infrastructure tests for `apps/time_tracer`.

Android runtime tests are grouped under:

- `apps/time_tracer/src/infrastructure/tests/android_runtime/`

## Android Runtime Test Files

- `android_runtime/android_runtime_test_common.hpp/.cpp`
  - Shared fixtures/helpers only.
  - Keep path/build helpers and common assertions here.
  - Do not put case-specific test logic here.

- `android_runtime/android_runtime_smoke_tests.cpp`
  - Runtime smoke behavior.
  - Query/report baseline checks.

- `android_runtime/android_runtime_core_config_tests.cpp`
  - Runtime config and report config validation checks.

- `android_runtime/android_runtime_business_regression_tests.cpp`
  - Business regression checks.

- `android_runtime/android_runtime_bundle_policy_tests.cpp`
  - Android `meta/bundle.toml` policy validation.
  - Any bundle policy restriction should be added here.

- `android_runtime/android_runtime_compat_tests.cpp`
  - Compatibility/fallback behavior (for example legacy path fallback).

- `android_runtime/android_runtime_test_main.cpp`
  - Test entrypoint only.
  - Dispatches grouped runners and reports total failures.
  - Keep `main()` free of case logic.

## Placement Rules For New Tests

1. Add shared utility code to `android_runtime/android_runtime_test_common.*` only when it is reused by at least two groups.
2. Put new test cases into the group file that matches behavior ownership:
   - Runtime/query/report baseline => `android_runtime/android_runtime_smoke_tests.cpp`
   - Config validation => `android_runtime/android_runtime_core_config_tests.cpp`
   - Business regression => `android_runtime/android_runtime_business_regression_tests.cpp`
   - Bundle constraints => `android_runtime/android_runtime_bundle_policy_tests.cpp`
   - Backward compatibility/fallback => `android_runtime/android_runtime_compat_tests.cpp`
3. If a new behavior does not fit existing groups, add a new `*_tests.cpp` group file and wire it via:
   - a new `Run...Tests(int& failures)` function declaration in `android_runtime/android_runtime_test_common.hpp`
   - a call from `android_runtime/android_runtime_test_main.cpp`
   - `apps/time_tracer/src/CMakeLists.txt` source list
4. Keep each test self-cleaning (`RemoveTree(...)`) and avoid cross-test shared mutable state.

## Validation Checklist

1. `python scripts/run.py build --app tracer_windows_cli`
2. `apps/tracer_windows_cli/build_fast/bin/time_tracker_android_runtime_tests.exe`
3. `rg -n "TestAndroidRuntime" apps/time_tracer/src/infrastructure/tests`
