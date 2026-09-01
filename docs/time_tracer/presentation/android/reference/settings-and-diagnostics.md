# Android Reference: Settings and Diagnostics

## Purpose

Describe the user-visible Settings and diagnostics support behavior.

## When To Open

- Open this when the task changes local TOML browsing/editing, diagnostics payload copy, or app appearance/language settings.

## What This Doc Does Not Cover

- Full runtime config bundle validation
- Build snapshot sync commands
- Historical config import/export behavior

## Behavior Summary

- The Settings UI edits the local runtime config snapshot in app-private storage.
- The Settings UI is for local browsing/editing and diagnostics support, not for package-style config exchange.
- Alias raw TOML editing uses the shared Android native `EditText` multiline editor rather than Compose `OutlinedTextField`.
- Unsaved config drafts stay in memory per file for the current app session and are written only after explicit `Save changes`.
- Diagnostics payload copy is a support action, not a runtime behavior authoring surface.
- Appearance, language, and Android-owned insights/chart settings are persisted
  as UI preferences in the app layer. The shared CLI user TOMLs for charts,
  heatmap thresholds, and insights are not part of Android configuration.
- Tab content extends behind the transparent gesture navigation area without a
  fixed bottom band exposing the page background. Clearance for the floating
  navigation belongs inside scrolling content so the final item remains reachable.
  This applies when switching Light/Dark modes and to every fixed appearance palette.

## Core Flow

- App-layer view models and repositories own preference and diagnostics UI wiring.
- Runtime owns config storage access and diagnostics payload assembly.

## First Code Entry Points

- `apps/android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigViewModel.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/SettingsScreen.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigEditorCard.kt`
- `apps/android/app/src/main/java/com/example/tracer/ui/screen/ConfigAliasEditorCard.kt`
- `apps/android/feature-ui-common/src/main/java/com/example/tracer/ui/components/NativeMultilineTextEditor.kt`
- `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeDiagnosticsService.kt`
