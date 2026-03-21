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
  - supports month-wide editing and day-focused editing
  - keeps edits as draft until explicit save
- Unsaved draft handling
  - discards drafts on tab leave or app stop
  - restores last saved content on re-entry

## Core Flow

- Feature-record owns UI state, intent handling, reducers, and save/sync actions.
- Runtime record delegates own validation and persistence flow.

## First Code Entry Points

- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
- `apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
