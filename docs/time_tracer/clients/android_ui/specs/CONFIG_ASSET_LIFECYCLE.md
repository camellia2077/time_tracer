# Config/Asset Lifecycle (Android Runtime)

This document defines the Android-side lifecycle from source config to runtime consumption.

## 1. Source of Truth

- Core config source: `apps/tracer_core/config`
- Android bundle sync entry:
  - `scripts/platform_config/run.py --target android --apply`
- Gradle integration:
  - `apps/tracer_android/runtime/build.gradle.kts`
  - task: `syncTracerCoreConfig` (hooked into `preBuild`)

## 2. Runtime Asset Layout

- Synced output root:
  - `apps/tracer_android/runtime/src/main/assets/tracer_core/config`
- Bundle metadata:
  - `apps/tracer_android/runtime/src/main/assets/tracer_core/config/meta/bundle.toml`
- `bundle.toml` carries:
  - `schema_version`
  - `profile` (must be `android`)
  - `file_list.required` (startup required files)

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
- Import/export flow in app module must follow `AndroidConfigPathPolicy`.

## 5. Diagnostics and Support

- Runtime writes structured diagnostics JSONL:
  - `<filesDir>/tracer_core/output/logs/diagnostics.jsonl`
- `RuntimeGateway` exposes:
  - `listRecentDiagnostics(limit)`
  - `buildDiagnosticsPayload(maxEntries)`
- Config tab "Copy Diagnostics Payload" uses the payload for support triage.
