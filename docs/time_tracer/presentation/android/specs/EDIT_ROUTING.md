# Android Edit Routing

## Purpose

Map common Android task types to the first code entrypoints worth opening.

## When To Open

- Open this after `STRUCTURE.md` when you know the task category but not the first file.

## What This Doc Does Not Cover

- Exhaustive related-file lists
- Full implementation walkthroughs
- Historical refactor rationale

## Task Routing

### Cross-feature screen wiring

Start here:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/TracerScreen.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenActions.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`

Route wiring rule:

- prefer the smallest gateway contract needed by the screen or helper
- do not default app-layer wiring to `RuntimeGateway`

### Import or export behavior

Start here:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTxtImport.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTracerImport.kt`

If the change is flow-shell related, then open:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTransferCoordinator.kt`

### Record or TXT editor behavior

Start here:

- `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorSession.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorRuntimeCoordinator.kt`

If the task is specifically about multiline TXT input behavior, then also open:

- `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/components/NativeMultilineTextEditor.kt`

### Report or chart presentation

Start here:

- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`

### Config, diagnostics, or app appearance

Start here:

- `apps/android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigViewModel.kt`
- `apps/android/app/src/main/java/com/example/tracer/data/ReportHeatmapTomlLoader.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigScreen.kt`

If the task is specifically about raw TOML editing behavior, then also open:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigEditorCard.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigAliasEditorCard.kt`
- `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/components/NativeMultilineTextEditor.kt`

### Runtime init, query, diagnostics, or JNI-backed execution

Start here:

- `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
- `apps/android/runtime/src/main/java/com/example/tracer/runtime/coreadapter/RuntimeCoreAdapter.kt`
- `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/*.kt`

### Runtime record or TXT storage behavior

Start here:

- `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
- `apps/android/runtime/src/main/java/com/example/tracer/runtime/LiveRawTxtFileStore.kt`
- `apps/android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeRecordDelegate.kt`

### Contract or model shape changes

Start here:

- `apps/android/contract/src/main/java/com/example/tracer/*.kt`

Then sync both runtime and UI consumers.

## High-Signal Troubleshooting

- UI text changed but one locale is missing:
  - check `values`, `values-zh`, and `values-ja` in the touched module
- Runtime call path is unclear:
  - start at `NativeRuntimeController`, then follow `RuntimeCoreAdapter` or the matching runtime service
- Unsure whether a change belongs in `app` or `feature-*`:
  - if it coordinates multiple tabs or repositories, start in `app`
  - if it changes only one tab’s presentation/state behavior, start in the feature module
