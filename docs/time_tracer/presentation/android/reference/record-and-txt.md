# Android Reference: Record and TXT

## Purpose

Describe the user-visible behavior of record creation and TXT editing flows.

## When To Open

- Open this when the task changes activity recording, TXT editing, draft handling, or record-side save/sync behavior.

## What This Doc Does Not Cover

- Full record parser implementation
- File-level runtime storage internals
- Broad architecture routing

## Behavior Summary

- `Record Activity`
  - is append-oriented
  - should not be used as a historical insertion tool
- TXT editing
  - supports month-wide editing (`ALL`) and day-focused editing (`DAY`)
  - uses Android native `EditText`-backed multiline editing instead of the older Compose `OutlinedTextField` path
  - exposes editor actions through the top toolbar (`Undo`, `Redo`, `Close`, `Ingest`)
- `DAY` editing
  - resolves the current day block from the month TXT through shared runtime day-block APIs
  - edits a local day draft inside the editor session
  - merges the edited day body back into the month TXT only when users tap `Ingest`
- Unsaved draft handling
  - `TXT` editor changes do not write files until explicit `Ingest`
  - closing the TXT editor sheet discards the current editing session if `Ingest` has not happened
  - leaving the `TXT` tab also discards the unsaved month draft that backs file persistence
  - `Config` keeps its own in-memory per-file drafts and is documented separately

## Core Flow

- Feature-record owns TXT editor presentation plus the editor-session reducer/controller/coordinator split.
- Runtime day-block semantics stay in shared TXT runtime calls; Android does not re-implement month/day parsing locally.
- Runtime record delegates own validation and persistence flow.

## First Code Entry Points

- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorSession.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorRuntimeCoordinator.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
- `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/components/NativeMultilineTextEditor.kt`
