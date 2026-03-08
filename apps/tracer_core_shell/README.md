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
  - `python tools/run.py post-change --app tracer_core_shell --run-tests always --build-dir build_fast --concise`
- Milestone / batch validation:
  - `python tools/run.py verify --app tracer_core_shell --quick --scope batch --concise`
- Result files:
  - `out/build/tracer_core_shell/build_fast/post_change_last.json`
  - `out/test/artifact_windows_cli/result.json`
  - `out/test/artifact_windows_cli/logs/output.log`
- CMake baseline for this host is `3.28` or newer.

## Runtime Boundary

- `tracer_core_shell` host assembly must not treat runtime bootstrap as permission to create a database.
- For ingest flows, database creation and schema initialization belong only to the post-validation persistence phase.
- If ingest validation fails and the database did not exist before the run, the run must not leave new SQLite artifacts behind.
- Prefer documenting and enforcing this rule in core/runtime design docs instead of relying on CLI-side cleanup.
