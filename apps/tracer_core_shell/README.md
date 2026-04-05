# tracer_core_shell

Internal shell host for `tracer_core` app identity migration.

Current phase:
- Preferred semantic app id: `tracer_core_shell`.
- Keep external compatibility app id as `tracer_core`.
- Shell source root now uses top-level folders:
  - `api/`
  - `host/`
  - `tests/`
- CMake configure root now also lives directly at `apps/tracer_core_shell`.

## Build And Verify

- Use `python tools/run.py -h` and `python tools/run.py <subcommand> -h` before trying new flag combinations.
- Preferred daily validation:
  - `python tools/run.py verify --app tracer_core_shell --build-dir build_fast --concise`
- Milestone / batch validation:
  - `python tools/run.py verify --app tracer_core_shell --profile fast_ci_no_pch --build-dir build_fast --concise`
- Result files:
  - `out/test/artifact_windows_cli/result.json`
  - `out/test/artifact_windows_cli/logs/output.log`
- CMake baseline for this host is `3.28` or newer.

## Runtime Boundary

- `tracer_core_shell` host assembly must not treat runtime bootstrap as permission to create a database.
- For ingest flows, database creation and schema initialization belong only to the post-validation persistence phase.
- If ingest validation fails and the database did not exist before the run, the run must not leave new SQLite artifacts behind.
- Prefer documenting and enforcing this rule in core/runtime design docs instead of relying on CLI-side cleanup.

## Shell Family Rules

- New shell source files must land under an existing family:
  - `api/c_api/capabilities/**`
  - `api/c_api/runtime/**`
  - `host/bootstrap/**`
  - `host/exchange/**`
- Do not add new flat `api/c_api/*.cpp` or `host/*.cpp` entrypoints.
- The only stable root-level C ABI exception remains:
  - `api/c_api/tracer_core_c_api.cpp`
  - `api/c_api/tracer_core_c_api.h`
- Existing retained root files are compatibility exceptions, not templates for new code.
- Every new shell file must be reflected in:
  - `tools/toolchain/commands/cmd_quality/verify_internal/verify_profile_inference.py`
  - the matching verify inference / CI tests under `tools/tests/verify/**`
