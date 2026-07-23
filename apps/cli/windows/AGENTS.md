# Windows CLI Local Contract

## Scope

Applies to `apps/cli/windows/**`. The only active CLI implementation is the Rust
client under `apps/cli/windows/rust/**`, hosted by `tracer_core.dll`.

## Required Read Set

1. `docs/time_tracer/clients/windows_cli/README.md`
2. `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`

Read `docs/time_tracer/core/shared/c_abi.md` when changing runtime bindings,
symbol loading, or ABI payload consumption.

## Ownership And Routing

- CLI/argument model: `apps/cli/windows/rust/src/cli/**`.
- Command dispatch: `apps/cli/windows/rust/src/commands/**`.
- Runtime session and capability clients:
  `apps/cli/windows/rust/src/core/runtime.rs` and `src/core/runtime/**`.
- Error model and user-facing diagnostics:
  `apps/cli/windows/rust/src/error/**`.
- Black-box suite root: `tools/suites/tracer_windows_rust_cli/tests.toml`.
- The CLI owns parsing, presentation, filesystem-facing command UX, and runtime
  invocation. It does not own core business semantics or maintain independent
  semantic caches.

## Local Invariants

- CLI runtime config under `apps/cli/windows/rust/runtime/config/**` is a
  generated/synchronized snapshot, not an independently authored config.
- Do not reintroduce dependencies on archived frontend implementations.
- Ingest persistence is atomic with respect to validation: a failed ingest must
  not create a new database or leave SQLite sidecars when the database did not
  exist before the run.
- If native/core code that enters `tracer_core.dll` changed, rebuild core before
  rebuilding the Rust CLI. A CLI-only build may otherwise copy a stale DLL.

## Validation

Required for Rust CLI code, config, suite, or behavior changes:

```powershell
python tools/run.py verify --app tracer_core --concise
```

When explicit release-bundle confirmation is required, preserve this order:

```powershell
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

Evidence:

- `out/test/artifact_windows_cli/result.json`
- `out/test/artifact_windows_cli/logs/output.log`

## Tests And Assets

- Use `test/data/**` for canonical cross-client TXT input.
- Use `test/fixtures/text/**` and `test/fixtures/config/**` for focused CLI
  black-box cases.
- Use `test/golden/**` only for stable final-output reconciliation.
- Runtime output belongs under `out/test/artifact_windows_cli/**`.

## Local Completion Bar

- CLI behavior remains a thin mapping to the canonical runtime contract.
- Changed command behavior has a focused suite case in the owning command
  family and passes standard Windows core/CLI verification.
- If native/core code changed, validation uses a freshly rebuilt DLL in the
  required core-then-CLI order.
- If ingest changed, failure-path coverage proves the no-database/no-sidecar
  boundary.
