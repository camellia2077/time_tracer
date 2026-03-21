# Android Reference: Data Import and Export

## Purpose

Capture the user-visible behavior and core data flow for Data-tab import and export actions.

## When To Open

- Open this when the task changes TXT import, TRACER import, or complete exchange package export behavior.

## What This Doc Does Not Cover

- Full runtime protocol
- Stable architecture ownership
- Historical design evolution

## Behavior Summary

- `Import Single TXT`
  - selects one TXT file
  - stages it in app cache
  - replaces the matching managed month through ingest
- `Import Single TRACER`
  - selects one `.tracer` file
  - stages it in app cache
  - requests a passphrase
  - imports through the TRACER exchange runtime path
- `Export Complete Exchange Package`
  - selects a destination tree
  - collects managed TXT and config payloads
  - requests a passphrase
  - exports one complete `.tracer` package

## Core Data Flow

- App route helpers own picker flow, staging, status updates, and transfer skeleton behavior.
- Runtime owns exchange import/export execution and payload validation.
- Record-side UI state owns crypto progress presentation.

## First Code Entry Points

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTxtImport.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTracerImport.kt`

If the change is shell-flow related, then also open:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTransferCoordinator.kt`
