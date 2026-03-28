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
  - native import decrypts the package, validates packaged converter TOML,
    builds an effective canonical TXT view, runs TXT structure validation,
    then runs TXT logic validation with the imported converter config before
    replacing managed files and rebuilding the database
- `Export Complete Exchange Package`
  - selects a destination tree
  - collects managed TXT payloads in memory
  - requests a passphrase
  - exports one complete `.tracer` package through a native fd sink

## Core Data Flow

- App route helpers own picker flow, SAF target resolution, status updates, and transfer skeleton behavior.
- Runtime owns exchange import/export execution, payload validation, package assembly, and native output writing.
- Record-side UI state owns crypto progress presentation.

## First Code Entry Points

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTxtImport.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTracerImport.kt`

If the change is shell-flow related, then also open:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTransferCoordinator.kt`
