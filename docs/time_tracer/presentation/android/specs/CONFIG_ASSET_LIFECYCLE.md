# Android Config / Asset Lifecycle

## Purpose

Describe the path from shared config source to Android runtime consumption.

## When To Open

- Open this when the task touches config snapshots, runtime bootstrap, or diagnostics/config access.

## What This Doc Does Not Cover

- Full runtime protocol
- UI behavior details
- Historical refactor notes

## Source of Truth

- Shared config source:
  - `assets/tracer_core/config_distribution` (new-user distribution)
  - `assets/tracer_core/config_test` (test/report data)
- Android checked-in runtime snapshot:
  - `apps/android/runtime/src/main/assets/tracer_core/config`

Boundary rules:

- The selected shared config directory is canonical for a build.
- Android build selection defaults to `test`; pass
  `--config-profile distribution` to build the distribution variant.
- The Android runtime snapshot is generated and consumed by Android builds.
- Fix the selected shared config source, then refresh the Android snapshot.

## Runtime Consumption Path

1. `NativeRuntimeController.initializeRuntime()` delegates to `RuntimeInitService`.
2. `RuntimeInitService` calls `RuntimeCoreAdapter.initializeRuntimeInternal()`.
3. `RuntimeEnvironment.prepareRuntimePaths()` copies assets into app-private files:
   - `<filesDir>/tracer_core/config`
4. `RuntimeEnvironment` validates `meta/bundle.toml`.
5. Successful validation proceeds to `nativeInit(...)`.

## Runtime Access Paths

- Native init config TOML:
  - `<filesDir>/tracer_core/config/aliases/_system.toml`
- Config editor reads and writes under:
  - `<filesDir>/tracer_core/config`
- The Config tab exposes structured editing plus an advanced raw TOML mode for
  the user-facing categories: `alias`, `charts`, and `reports`. Alias
  files use the structured canonical-to-alias-list editor; the advanced mode
  still serializes and validates the same strict TOML shape.

## Diagnostics and Support

- Runtime diagnostics log path:
  - `<filesDir>/tracer_core/output/logs/diagnostics.jsonl`
- Config-related diagnostics entrypoints:
  - `ConfigGateway.listRecentDiagnostics(limit)`
  - `ConfigGateway.buildDiagnosticsPayload(maxEntries)`
- Runtime-side diagnostics assembly:
  - `RuntimeDiagnosticsService`

Runtime TXT layout:

- Canonical runtime input root:
  - `<filesDir>/tracer_core/input`
- Managed month files:
  - `<filesDir>/tracer_core/input/YYYY/YYYY-MM.txt`
- Temporary validation/staging files:
  - `<filesDir>/tracer_core/cache`
- APK assets do not package runtime TXT input files.
- Runtime TXT content should come only from user-managed import, edit, record,
  or explicit device-side copy flows.
