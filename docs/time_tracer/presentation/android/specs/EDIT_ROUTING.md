# Android Edit Routing

This document is the detailed file-routing map for Android changes.

Use it after:

1. `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
2. `docs/time_tracer/presentation/android/specs/STRUCTURE.md`

## Common Change Routing

1. Add or modify a Data tab button or progress area:
   - `apps/android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`
   - `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
2. Change import/export behavior:
   - `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
   - `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenImports.kt`
3. Change report chart visualization behavior:
   - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSection.kt`
   - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualMode.kt`
   - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationControls.kt`
   - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationHeatmapControls.kt`
   - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSummary.kt`
4. Change record-side shared UI state:
   - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
   - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordIntentHandler.kt`
   - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordStateReducer.kt`
   - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordUseCaseCaller.kt`
5. Add or extend runtime service behavior:
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/*.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeBridge.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/RuntimeSession.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/RuntimeErrorMapper.kt`
6. Change runtime record write or TXT storage behavior:
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawTxtFileStore.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordNormalization.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordParsing.kt`
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordPersistence.kt`
7. Add or extend JNI callback/event bridge:
   - Android side: `apps/android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt`
   - Core side: `apps/tracer_core_shell/api/android_jni/native_bridge_calls.cpp`
8. Add or extend shared contract fields:
   - `apps/android/contract/src/main/java/com/example/tracer/*.kt`
   - Then sync runtime parsing and UI usage.

## Example Chain: File Crypto Progress

1. Core emits progress snapshot:
   - `apps/tracer_core_shell/api/android_jni/native_bridge_calls.cpp`
2. JNI bridge forwards JSON:
   - `apps/android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt`
3. Runtime parses and dispatches:
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeCryptoService.kt`
4. Contract receives typed event:
   - `apps/android/contract/src/main/java/com/example/tracer/FileCryptoProgress.kt`
5. App flow passes callback:
   - `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
   - `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenImports.kt`
6. ViewModel updates UI state:
   - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
7. Data tab renders progress:
   - `apps/android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`

## Troubleshooting First Checks

1. Build fails after contract change:
   - Check `contract -> runtime parser -> UI` sync completeness.
2. Progress is shown but never updates:
   - Check callback registration and cleanup in `RuntimeCryptoService`.
   - Check `NativeBridge.onCryptoProgressJson(...)`.
3. UI changed but text is missing:
   - Check string resources across English, Chinese, and Japanese locales.
4. Runtime call path is unclear after refactor:
   - Start from `NativeRuntimeController`, then follow to `NativeRuntimeBridge`, `RuntimeSession`, and `RuntimeErrorMapper`.
5. Record UI state ownership is unclear:
   - Reducer logic stays in `RecordStateReducer`.
   - Async use case wiring stays in `RecordIntentHandler` and `RecordUseCaseCaller`.
   - `RecordViewModel` stays as the orchestration shell.
