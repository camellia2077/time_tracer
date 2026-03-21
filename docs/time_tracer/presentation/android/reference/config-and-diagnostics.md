# Android Reference: Config and Diagnostics

## Purpose

Describe the user-visible config editing and diagnostics support behavior.

## When To Open

- Open this when the task changes local TOML browsing/editing, diagnostics payload copy, or app appearance/language settings.

## What This Doc Does Not Cover

- Full runtime config bundle validation
- Build snapshot sync commands
- Historical config import/export behavior

## Behavior Summary

- Config UI edits the local runtime config snapshot in app-private storage.
- Config UI is for local browsing/editing and diagnostics support, not for package-style config exchange.
- Diagnostics payload copy is a support action, not a runtime behavior authoring surface.
- Appearance and language settings are persisted as UI preferences in the app layer.

## Core Flow

- App-layer view models and repositories own preference and diagnostics UI wiring.
- Runtime owns config storage access and diagnostics payload assembly.

## First Code Entry Points

- `apps/android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigViewModel.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigScreen.kt`
- `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeDiagnosticsService.kt`
