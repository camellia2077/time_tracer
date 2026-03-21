# Android Agent Onboarding

## Purpose

Get a coding agent to the correct Android code area in a few minutes.

## When To Open

- Open this right after the local Android entry docs.
- Use it before broad searching.

## What This Doc Does Not Cover

- Full file inventories
- Runtime payload schemas
- Historical design decisions

## 5-Minute Path

1. `apps/android/agent.md`
2. `docs/time_tracer/presentation/android/README.md`
3. `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
4. `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
5. `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`

Open only when needed:

- Runtime and config bootstrap:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Runtime protocol:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- User-visible behavior:
  - `docs/time_tracer/presentation/android/features.md`
- Preference storage:
  - `docs/time_tracer/presentation/android/specs/preference-storage.md`
- i18n sync:
  - `docs/time_tracer/presentation/android/specs/i18n-button-sync.md`

## Module Responsibility Map

1. `apps/android/app`
   - composition root, screen-level coordination, cross-feature wiring
2. `apps/android/feature-data`
   - Data tab UI and actions
3. `apps/android/feature-record`
   - Record and TXT editor UI state
4. `apps/android/feature-report`
   - report, query, and chart presentation
5. `apps/android/runtime`
   - runtime implementation, JNI bridge integration, services, coreadapter
6. `apps/android/contract`
   - shared gateway interfaces and result models

## First Routing Heuristic

- Cross-feature UI wiring:
  - start at `apps/android/app`
- Data import/export flow:
  - start at `apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer`
- Record or TXT behavior:
  - start at `apps/android/feature-record`
- Report or chart behavior:
  - start at `apps/android/feature-report`
- Runtime init, query, diagnostics, JNI, or path/bootstrap behavior:
  - start at `apps/android/runtime`
- Shared models or gateway signatures:
  - start at `apps/android/contract`

## Guardrails

- `feature-*` modules should not call JNI directly.
- Runtime/JNI details stay in `runtime`.
- Update string resources in the same change when visible text changes.
- Do not search or edit generated folders by default:
  - `**/build/**`
  - `**/.gradle/**`
  - `**/.kotlin/**`
  - `**/.cxx/**`
  - `**/.externalNativeBuild/**`

## Minimal Validation

Run from repo root:

```powershell
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
```

Always serialize Gradle-backed commands for `apps/android`.
