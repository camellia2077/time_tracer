# tracer_android Architecture

Android host app for `apps/time_tracer` (`Jetpack Compose + JNI`).

## Module Layers

- `app`: composition root + main UI host
- `contract`: stable `RuntimeGateway` + DTO contracts
- `feature-data`: data screen
- `feature-report`: report screen
- `feature-record`: record/txt screens
- `runtime`: JNI + runtime implementation

## Dependency Direction

- `app -> contract + runtime + feature-*`
- `feature-* -> contract`
- `runtime -> contract`

## Runtime Data Layout

Bundled assets location:

- `runtime/src/main/assets/time_tracer/**`

On app startup, assets are copied to:

- `${filesDir}/time_tracer/**`

Then runtime initializes via:

- `nativeInit(dbPath, outputRoot, converterConfigTomlPath)`

`converterConfigTomlPath`:

- `${filesDir}/time_tracer/config/converter/interval_processor_config.toml`

## Core C ABI Alignment Status

- Core side ABI naming is standardized to `tracer_core_*`.
- Android runtime is still exposed to Kotlin through JNI bridge APIs
  (`nativeInit/nativeIngest/nativeQuery/nativeReport`).
- JNI internal call path is migrated to unified `tracer_core_*` C ABI.
- JNI-to-core business payload protocol remains UTF-8 JSON (`const char*`).
- Android delivery model remains single bridge `.so` (`time_tracker_android_bridge`);
  no separate Android-side core DLL packaging.

## Runtime Protocol

For full protocol details (JNI contract, C ABI mapping, JSON fields):

- `docs/time_tracer/clients/android_ui/runtime-protocol.md`
- `docs/time_tracer/core/contracts/c_abi.md`

## Deep-Dive

For full module tree and responsibilities, see:

- `apps/tracer_android/STRUCTURE.md`
