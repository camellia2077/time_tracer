# tracer_android Architecture

Android host app for `apps/tracer_core_shell` (`Jetpack Compose + JNI`).

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

- `runtime/src/main/assets/tracer_core/**`

On app startup, assets are copied to:

- `${filesDir}/tracer_core/**`

Then runtime initializes via:

- `nativeInit(dbPath, outputRoot, converterConfigTomlPath)`

`converterConfigTomlPath`:

- `${filesDir}/tracer_core/config/converter/interval_processor_config.toml`

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

- `docs/time_tracer/clients/android_ui/specs/STRUCTURE.md`

## Recent Responsibility Split (2026-03-02)

To reduce file-level coupling and improve agent navigation, three hotspots were split by role:

1. Runtime controller boundary (`runtime`)
   - orchestrator: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
   - JNI bridge calls: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeBridge.kt`
   - runtime lifecycle/session cache: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/RuntimeSession.kt`
   - runtime error formatting/mapping: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/RuntimeErrorMapper.kt`

2. Tracer screen composition root (`app`)
   - route/composition root: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/TracerScreen.kt` (`TracerScreen`)
   - action wiring: same file (`TracerScreenActions`, `rememberTracerScreenActions`)
   - presentational content: same file (`TracerScreenContent`)

3. Record state flow (`feature-record`)
   - ViewModel shell: `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
   - intent handling: `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordIntentHandler.kt`
   - state reducer: `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordStateReducer.kt`
   - use case call adapter: `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordUseCaseCaller.kt`
