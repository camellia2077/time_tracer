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
  - supports two authored shapes in the `Record` tab:
    - point event: `HHMMtoken`
    - interval event: `HHMM-HHMMtoken`
- Raw TXT editing (available from the Files tab)
  - supports month-wide editing (`ALL`) and day-focused editing (`DAY`)
  - uses Android native `EditText`-backed multiline editing instead of the older Compose `OutlinedTextField` path
  - displays the selected TXT content inline in the Files tab, with `Undo`, `Redo`, and `Ingest` actions
- `DAY` editing
  - resolves the current day through shared Core structured-day APIs
  - shows point and interval events as editable cards; structured cards support
    time changes, selecting a canonical activity from the Record tree, and editing
    the day or activity remarks
  - offers a mutually exclusive `Structured` / `Raw TXT` capsule so the same
    selected day can be inspected or edited in its original text form
  - Core renders the normalized day body and merges it into the month TXT when
    users save a structured change; Android then replaces/syncs only that month
    in the database
  - falls back to the raw inline day editor for missing/unresolvable day blocks
- Unsaved draft handling
  - `TXT` editor changes do not write files until explicit `Ingest`
- leaving the Files tab discards the current editing session if `Ingest` has not happened
- leaving the Config page also discards the unsaved month draft that backs file persistence
  - `Config` keeps its own in-memory per-file drafts and is documented separately

## Core Flow

- Feature-record owns TXT editor presentation plus the editor-session reducer/controller/coordinator split.
- Runtime day-block semantics stay in shared TXT runtime calls; Android keeps `MMDD` as the day input/API marker and uses the shared `dMMDD` month-TXT marker line format when seeding missing blocks.
- Runtime record delegates own validation and persistence flow.
- Interval authoring uses the same candidate-TXT save/sync path as TXT editing:
  - Android builds a candidate day-block update
  - shared runtime validation decides whether mixed point/interval timeline semantics are valid
  - Android does not locally enforce overlap/gap business rules
- Android does not require a month to start on day 1 and does not require every day
  to be recorded. Record, interval, TXT save, import, sync, and rebuild flows use
  `DATE_CHECK_NONE`; the shared Core continuity/full checks remain available to CLI
  callers but are not enabled by Android user flows.

## First Code Entry Points

- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorSession.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorRuntimeCoordinator.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
- `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/components/NativeMultilineTextEditor.kt`
