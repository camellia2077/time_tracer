# Android Reference: Data Import and Export

## Purpose

Capture the user-visible behavior and core data flow for Config > Data Management import and export actions.

## When To Open

- Open this when the task changes TXT import, TOML import, TRACER import, or exchange package export behavior.

## What This Doc Does Not Cover

- Full runtime protocol
- Stable architecture ownership
- Historical design evolution

## Behavior Summary

- `Import TXT Folder` (from Config > Data Management)
  - selects one folder
  - recursively imports `.txt` files
  - stages each TXT in app cache
  - replaces matching managed months through ingest
- `Import TOML Folder` (from Config > Data Management)
  - selects one folder
  - recursively imports `.toml` files
  - preserves selected-folder-relative paths under the imported `config/` root
  - sends `config/user/activity_hierarchy/*.toml` through Core activity-hierarchy
    validation/migration
  - imports mutable TOML under `config/user/`; `config/program/` is not required
    in the selected folder because it is presentation-owned runtime data
- `Import Single TRACER` (from Config > Data Management)
  - selects one `.zip` file
  - stages it in app cache
  - requests a passphrase
  - imports through the TRACER exchange runtime path
  - native import decrypts the package, validates packaged converter TOML,
    restores packaged markdown insights TOML,
    builds an effective canonical TXT view, runs TXT structure validation,
    then runs TXT logic validation with the imported converter config before
    replacing managed files and rebuilding the database
- `Export Complete Exchange Package` (from Config > Data Management)
  - selects a destination tree
  - collects managed TXT payloads in memory
  - packages the active converter main TOML and activity-hierarchy TOML under
    `config/user/`
  - requests a passphrase
  - exports one complete encrypted standard `.zip` package through a native fd sink
- `Export Current TXT ZIP` (from Config > Data Management)
  - selects a destination tree
  - writes one unencrypted `.zip`
  - writes TXT under `txt/`
  - exports every TOML under the mutable `config/user/` root, including
    `behavior.toml`, `charts.toml`, `heatmap.toml`, and all activity-hierarchy
    TOML files
  - preserves the canonical user-config paths under `config/user/`, for example
    `txt/2026/2026-01.txt` and `config/user/activity_hierarchy/study.toml`

## Android directory mapping

| Repository | Android private runtime | Role |
| --- | --- | --- |
| `config/program/**` | `<filesDir>/tracer_core/config/program/**` | Immutable program resources; generated into APK assets during Gradle build; not exchange data |
| `assets/tracer_core/defaults/activity_hierarchy/**` | `<filesDir>/tracer_core/config/user/activity_hierarchy/**` on first launch | Distribution seed for user hierarchy |
| `test/data/**/*.txt` | `<filesDir>/tracer_core/input/**/*.txt` through the ADB helper | Test input, not APK content |
| `test/data/activity_hierarchy/**` | `<filesDir>/tracer_core/config/user/activity_hierarchy/**` through the ADB helper | Test hierarchy, not APK content |

## Core Data Flow

- App route helpers own picker flow, SAF target resolution, status updates, and transfer skeleton behavior.
- Runtime owns exchange import/export execution, payload validation, package assembly, and native output writing.
- Candidate TXT/config validation uses the Core pipeline-only Runtime. It
  requires user converter/hierarchy TOML and TXT input, but does not load or
  copy `config/program/**`; that resource tree is used only after the complete
  application Runtime starts.
- Record-side UI state owns crypto progress presentation. Complete encrypted ZIP
  export uses one overall progress bar; it does not show a separate current-file
  progress bar.

## First Code Entry Points

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTxtImport.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTomlImport.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTracerImport.kt`

If the change is shell-flow related, then also open:

- `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenTransferCoordinator.kt`
