# Config/Asset Lifecycle (Android Runtime)

This document defines the Android-side lifecycle from source config to runtime consumption.

## 1. Source of Truth

- Core config source: `assets/tracer_core/config`
- Android checked-in snapshot:
  - `apps/android/runtime/src/main/assets/tracer_core/config`
- Explicit refresh entry:
  - `tools/platform_config/run.py --target android --apply`
- Explicit Gradle wrappers:
  - `:runtime:syncTracerCoreConfigSnapshot`
  - `:runtime:verifyTracerCoreConfigSnapshot`

## 1.1 Boundary Rules

- `assets/tracer_core/config` is the only shared config source.
- `apps/android/runtime/src/main/assets/tracer_core/config` is the checked-in Android runtime snapshot consumed by IDE/Gradle builds.
- Android-side fixes to shared config must be applied back at `assets/tracer_core/config`, then refreshed into the snapshot explicitly.
- Android runtime Gradle no longer supports overriding config roots or debug asset roots via custom path properties; the repository paths above are canonical.
- Design-reference SVG / branding exploration files are outside this lifecycle; their long-term home is `design/branding/**`, not the Android runtime asset tree.

## 2. Runtime Asset Layout

- Snapshot root consumed by Android Gradle builds:
  - `apps/android/runtime/src/main/assets/tracer_core/config`
- Bundle metadata:
  - `apps/android/runtime/src/main/assets/tracer_core/config/meta/bundle.toml`
- `bundle.toml` carries:
  - `schema_version`
  - `profile` (must be `android`)
  - `file_list.required` (startup required files)

## 2.1 Debug Seed TXT Assets

- Canonical source:
  - `test/data/**/*.txt`
- Debug generated assets root:
  - `apps/android/runtime/build/generated/tracer/runtime/debug/assets/tracer_core/input/full`
- Sync mode:
  - pure Gradle sync, attached only to debug asset generation
- Release builds do not consume or package these seed TXT assets.

## 3. App Startup Consumption Chain

1. `NativeRuntimeController.initializeRuntimeInternal()` requests runtime paths.
2. `RuntimeEnvironment.prepareRuntimePaths()` copies assets to app files dir:
   - `<filesDir>/tracer_core/config`
3. `RuntimeEnvironment` validates `meta/bundle.toml` via `validateRuntimeConfigBundle(...)`:
   - schema/version check
   - profile check (`android`)
   - required file existence check
4. Validation failure throws and stops runtime init.
5. Validation success proceeds to `nativeInit(...)`.

## 4. Runtime Access Paths

- Native init config path:
  - `<filesDir>/tracer_core/config/converter/interval_processor_config.toml`
- Config editor gateway (`ConfigTomlStorage`) reads/writes under:
  - `<filesDir>/tracer_core/config`
- Config tab edits the local runtime snapshot in place; Android no longer exposes a config bundle import/export exchange flow.

## 5. Diagnostics and Support

- Runtime writes structured diagnostics JSONL:
  - `<filesDir>/tracer_core/output/logs/diagnostics.jsonl`
- `RuntimeGateway` exposes:
  - `listRecentDiagnostics(limit)`
  - `buildDiagnosticsPayload(maxEntries)`
- Config tab "Copy Diagnostics Payload" uses the payload for support triage.
