# android

Android host app for `apps/tracer_core_shell` (`Jetpack Compose + JNI`).

This file is a short navigation page. Keep detailed rules in `docs/time_tracer/presentation/android/**`.

## Read Order

1. `docs/time_tracer/presentation/android/README.md`
2. `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
3. `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
4. `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
5. `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
6. `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`

Open additional docs only when the task needs them:

- Runtime protocol:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- Preference storage:
  - `docs/time_tracer/presentation/android/specs/preference-storage.md`
- i18n button sync:
  - `docs/time_tracer/presentation/android/specs/i18n-button-sync.md`
- Runtime refactor baseline:
  - `docs/time_tracer/presentation/android/specs/RUNTIME_REFACTOR_BASELINE.md`

## Hard Rules

- Shared config source of truth is `assets/tracer_core/config`.
- Android runtime config copy under `apps/android/runtime/src/main/assets/tracer_core/config` is generated, not canonical.
- Android app version source is `apps/android/meta/version.properties`.
- Core version source is `libs/tracer_core/src/shared/types/version.hpp`.
- Do not run Gradle commands for `apps/android` in parallel.
- Prefer `python tools/run.py build --app tracer_android --profile android_edit` for the default edit loop.

## Code Areas

- Composition root:
  - `apps/android/app`
- Data feature:
  - `apps/android/feature-data`
- Record/TXT feature:
  - `apps/android/feature-record`
- Report feature:
  - `apps/android/feature-report`
- Runtime/JNI bridge:
  - `apps/android/runtime`
- Shared contracts:
  - `apps/android/contract`

Detailed edit routing lives in:

- `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
- `docs/time_tracer/presentation/android/specs/STRUCTURE.md`

## Scan Scope

- Prefer source dirs under `apps/android/**/src`.
- Ignore generated outputs by default:
  - `apps/android/**/build/**`
  - `apps/android/**/.gradle/**`
  - `apps/android/**/.kotlin/**`
  - `apps/android/**/.cxx/**`
  - `apps/android/**/.externalNativeBuild/**`
