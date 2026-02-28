# tracer_android Structure

This document defines current tab ownership, entry boundaries, and coordinator responsibilities for `tracer_android`.

## Tab Ownership

- `DATA` tab
  - entry: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - UI implementation: `apps/tracer_android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`
  - ViewModel: `DataViewModel` (`feature-data`)

- `REPORT` tab
  - entry: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - feature entry: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
  - result dispatcher: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
  - chart entry: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`
  - chart parameter section: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartParameterSection.kt`
  - chart visualization section (split):
    - orchestrator: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSection.kt`
    - mode contracts: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualMode.kt`
    - mode/hint controls: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationControls.kt`
    - heatmap controls: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationHeatmapControls.kt`
    - summary/footer: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSummary.kt`
  - markdown renderer/parser:
    - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownRenderer.kt`
    - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownParser.kt`
  - chart query use case split:
    - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartPipeline.kt`
    - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartParamResolver.kt`
    - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartMappers.kt`
    - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartModels.kt`
  - ViewModel: `QueryReportViewModel` (`feature-report`)

- `RECORD` tab
  - entry: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - feature entry: `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt`
  - ViewModel: `RecordViewModel` (`feature-record`)

- `TXT` tab
  - entry: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - UI implementation: `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
  - ViewModel: `RecordViewModel` (`feature-record`)

- `CONFIG` tab
  - entry: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - UI implementation: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/ConfigScreen.kt`
  - ViewModel: `ConfigViewModel` (`app`)
  - config import/export use case: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigBundleTransferUseCase.kt`
  - config import/export models: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigBundleTransferModels.kt`

## Coordinator and Shell Boundaries

- Composition root and coordinator: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/TracerScreen.kt`
  - owns selected tab state
  - wires viewmodels and shared dependencies
  - dispatches lifecycle/tab transition hooks through `TracerTabRegistry`
  - dispatches typed coordinator events

- Navigation shell: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/TracerScreenShell.kt`
  - owns bottom navigation rendering only
  - does not contain tab-specific business logic

- Tab registry and tab contracts: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
  - single source for tab order/meta/content
  - tab hooks: `onEnter`, `onLeave`, `statusText`, `statusEvent`

## Runtime Query Layer Index

- Service entry
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeQueryService.kt`
  - provides query-facing methods for `RuntimeGateway` and delegates execution.
- Delegate layer
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeQueryDelegate.kt`
  - coordinates validation, runtime invocation, fallback strategy, and domain result mapping.
- Runtime bridge execution
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
  - owns `executeNativeDataQuery(...)` and `executeNativeTreeQuery(...)` transport calls.
- Query ops split (pure helpers)
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryContracts.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryValidation.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryParsing.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryMessages.kt`
- Translator layer
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/translators/NativeQueryTranslator.kt`
  - translates native/raw call results into typed query domain results.

## Runtime Record Store Index

- Record facade entry
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
  - owns record-write orchestration and delegates IO/parsing/normalization concerns.
- TXT file IO store
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawTxtFileStore.kt`
  - owns `list/read/write` TXT file access and path-boundary checks.
- Normalization rules
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordNormalization.kt`
  - normalizes activity/remark text and comparison keys.
- Parsing rules
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordParsing.kt`
  - detects duplicate events, parses event lines, validates HHmm ordering.
- Persistence rules
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordPersistence.kt`
  - creates month files and persists day-block insertion semantics.

## Event Flow

- Tab UI event type: `TracerTabUiEvent`
  - file: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenEvents.kt`

- Coordinator event type: `TracerCoordinatorEvent`
  - file: `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenEvents.kt`

- Status event bridge:
  - `StatusSnackbarEffect` consumes `TracerTabRegistry.statusEvent(...)`
  - snackbar action emits coordinator event (for example: `SelectTab(TXT)`)
  - `TracerScreen` handles coordinator events centrally

## Persistence Flow (Record/Report UI Preferences)

- UI preference writes are injected as callbacks from `TracerScreen` into tab route args.
- Feature tab entries call injected callbacks; they do not directly depend on app repository implementation.
