# Android Config / Asset Lifecycle

## Purpose

Describe the path from shared config source to Android runtime consumption.

## When To Open

- Open this when the task touches generated config assets, runtime bootstrap,
  or diagnostics/config access.

## What This Doc Does Not Cover

- Full runtime protocol
- UI behavior details
- Historical refactor notes

## Source of Truth

- Shared program-resource source:
  - `config/program`
- Distribution activity-hierarchy seed:
  - `assets/tracer_core/defaults/activity_hierarchy`
- Test activity-hierarchy source:
  - `test/data/activity_hierarchy` (not packaged into APK)
- Android generated runtime assets:
  - `apps/android/runtime/build/generated/tracer/assets/config/program`

Boundary rules:

- The selected shared program-resource directory is canonical for a build.
- Android builds generate program TOML from `config/program`;
  no build-time profile selects test data for Android.
- The generated assets are consumed by Android builds and are not checked in.
- Fix the canonical source config, then rerun the Gradle generation task.
- Date continuity/fullness is not a shared program-config setting. Android uses
  `DATE_CHECK_NONE` for user data flows; CLI callers select `none`, `continuity`,
  or `full` through command arguments.

## Runtime Consumption Path

1. `NativeRuntimeController.initializeRuntime()` delegates to `RuntimeInitService`.
2. `RuntimeInitService` calls `RuntimeCoreAdapter.initializeRuntimeInternal()`.
3. `RuntimeEnvironment.prepareRuntimePaths()` copies assets into app-private files:
   - `<filesDir>/config/program` is overwritten on every process
     start. It is an APK-owned read-only bundle, so an APK update refreshes it.
   - `<filesDir>/config/user` is seeded without overwriting existing
     files, preserving user-managed configuration.
   - `<filesDir>/config/user/activity_hierarchy` is seeded from
     `tracer_core/defaults/activity_hierarchy` when the private files are absent.
4. `RuntimeEnvironment` validates `meta/bundle.toml`.
5. Successful validation proceeds to `nativeInit(...)`.

## Runtime Access Paths

- Native init config TOML:
  - `<filesDir>/tracer_core/config/user/behavior.toml`
- Config editor reads and writes under:
  - `<filesDir>/tracer_core/config`
- Only `<filesDir>/tracer_core/config/user/*.toml` and
  `<filesDir>/tracer_core/config/user/activity_hierarchy/*.toml` are user-editable.
  Android-managed preferences are the exception: `charts.toml`, `heatmap.toml`,
  and `insights.toml` are neither packaged nor read from this directory.
  `config.toml`, `charts/**`, `meta/**`, and `insights/**` are program resources
  and are read-only in the Android UI.
- The Settings tab exposes structured editing plus an advanced raw TOML mode for
  the `alias` category. Alias files use the structured canonical-to-alias-list
  editor; the advanced mode still serializes and validates the same strict TOML
  shape. Chart and insights preferences live in Android `DataStore`, not under
  the runtime config snapshot.

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
- Debug fixtures are injected with
  `tools/scripts/devtools/android/push_test_data.py`; they are not APK assets.
