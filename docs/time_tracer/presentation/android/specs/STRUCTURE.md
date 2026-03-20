# tracer_android Structure

This document defines current tab ownership, entry boundaries, and coordinator responsibilities for `tracer_android`.

## Version Metadata Ownership

- Android app version source of truth:
  - `apps/android/meta/version.properties`
- Core version source of truth:
  - `libs/tracer_core/src/shared/types/version.hpp`
- Gradle ownership:
  - `apps/android/app/build.gradle.kts` reads both sources and injects:
    - Android app `versionCode` / `versionName`
    - `BuildConfig.TRACER_CORE_VERSION`
- About page display:
  - App version uses `BuildConfig.VERSION_NAME`
  - Core version uses `BuildConfig.TRACER_CORE_VERSION`
- Do not reintroduce hardcoded version strings in UI layer files such as:
  - `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigScreen.kt`

## Tab Ownership

- `DATA` tab
  - entry: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - UI implementation: `apps/android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`
  - ViewModel: `DataViewModel` (`feature-data`)

- `REPORT` tab
  - entry: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - feature entry: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
  - result dispatcher: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
  - chart entry: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`
  - chart parameter section: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartParameterSection.kt`
  - chart visualization section (split):
    - orchestrator: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSection.kt`
    - mode contracts: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualMode.kt`
    - mode/hint controls: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationControls.kt`
    - heatmap controls: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationHeatmapControls.kt`
    - summary/footer: `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSummary.kt`
  - markdown renderer/parser:
    - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownRenderer.kt`
    - `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownParser.kt`
  - chart query use case split:
    - `apps/android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartPipeline.kt`
    - `apps/android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartParamResolver.kt`
    - `apps/android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartMappers.kt`
    - `apps/android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartModels.kt`
  - ViewModel: `QueryReportViewModel` (`feature-report`)

- `RECORD` tab
  - entry: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - feature entry: `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt`
  - ViewModel: `RecordViewModel` (`feature-record`)

- `TXT` tab
  - entry: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - UI implementation: `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
  - ViewModel: `RecordViewModel` (`feature-record`)

- `CONFIG` tab
  - entry: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - UI implementation: `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigScreen.kt`
  - ViewModel: `ConfigViewModel` (`app`)
  - scope: local config browsing/editing and diagnostics copy only

## Coordinator and Shell Boundaries

- Composition root and coordinator: `apps/android/app/src/main/java/com/example/tracer/ui/screen/TracerScreen.kt`
  - route role (`TracerScreen`):
    - owns selected tab state
    - wires viewmodels and shared dependencies
    - dispatches lifecycle/tab transition hooks through `TracerTabRegistry`
    - dispatches typed coordinator events
  - actions role (`TracerScreenActions`, `rememberTracerScreenActions`):
    - owns event dispatchers and side-effect callbacks
    - owns preference persistence callback wiring
    - owns diagnostics payload copy action
  - content role (`TracerScreenContent`):
    - pure UI render function for shell + current tab content
    - receives all state/actions by parameters, no coordinator ownership

- Navigation shell: `apps/android/app/src/main/java/com/example/tracer/ui/screen/TracerScreenShell.kt`
  - owns bottom navigation rendering only
  - does not contain tab-specific business logic

- Tab registry and tab contracts: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - single source for tab order/meta/content
  - tab hooks: `onEnter`, `onLeave`, `statusText`, `statusEvent`

## Runtime Query Layer Index

- Service entry
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeQueryService.kt`
  - provides query-facing methods for `RuntimeGateway` and delegates execution.
- Delegate layer
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeQueryDelegate.kt`
  - coordinates validation, runtime invocation, fallback strategy, and domain result mapping.
- Runtime bridge execution
  - orchestrator: `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
  - bridge adapter: `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeBridge.kt`
  - session cache owner: `apps/android/runtime/src/main/java/com/example/tracer/runtime/RuntimeSession.kt`
  - error mapping owner: `apps/android/runtime/src/main/java/com/example/tracer/runtime/RuntimeErrorMapper.kt`
  - query transport dispatch still owned by controller:
    - `executeNativeDataQuery(...)`
    - `executeNativeTreeQuery(...)`
- Query ops split (pure helpers)
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryContracts.kt`
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryValidation.kt`
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryParsing.kt`
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryMessages.kt`
- Translator layer
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/translators/NativeQueryTranslator.kt`
  - translates native/raw call results into typed query domain results.

## Runtime Record Store Index

- Record facade entry
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
  - owns record-write orchestration and delegates IO/parsing/normalization concerns.
- TXT file IO store
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawTxtFileStore.kt`
  - owns `list/read/write` TXT file access and path-boundary checks.
- Normalization rules
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordNormalization.kt`
  - normalizes activity/remark text and comparison keys.
- Parsing rules
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordParsing.kt`
  - detects duplicate events, parses event lines, validates HHmm ordering.
- Persistence rules
  - `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordPersistence.kt`
  - creates month files and persists day-block insertion semantics.

## Event Flow

- Tab UI event type: `TracerTabUiEvent`
  - file: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenEvents.kt`

- Coordinator event type: `TracerCoordinatorEvent`
  - file: `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenEvents.kt`

- Status event bridge:
  - `StatusSnackbarEffect` consumes `TracerTabRegistry.statusEvent(...)`
  - snackbar action emits coordinator event (for example: `SelectTab(TXT)`)
  - `TracerScreen` handles coordinator events centrally

## Persistence Flow (Record/Report UI Preferences)

- UI preference writes are injected as callbacks from `TracerScreen` into tab route args.
- Feature tab entries call injected callbacks; they do not directly depend on app repository implementation.

## Record ViewModel Responsibility Split

- ViewModel shell
  - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
  - keeps UI state holder + coroutine scope boundaries only.
- Intent handling
  - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordIntentHandler.kt`
  - maps UI intents to reducer transitions/use case calls.
- State reducer
  - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordStateReducer.kt`
  - pure state transitions and crypto progress presentation formatting.
- Use case adapter
  - `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordUseCaseCaller.kt`
  - encapsulates `RecordUseCases` invocations.
