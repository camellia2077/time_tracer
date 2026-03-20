# Android Agent Onboarding

This document is a fast entry for coding agents and new contributors.
Goal: locate the right files in 5 minutes and avoid broad repo-wide searching.

## 1. Read Order (5-Minute Path)
1. `apps/android/agent.md`
2. `docs/time_tracer/presentation/android/README.md`
3. `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
4. `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
5. This file (`AGENT_ONBOARDING.md`)

Open only the extra docs needed for the current task:

- edit routing:
  - `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
- runtime protocol:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- config assets:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- preference storage:
  - `docs/time_tracer/presentation/android/specs/preference-storage.md`
- i18n sync:
  - `docs/time_tracer/presentation/android/specs/i18n-button-sync.md`

## 2. Module Responsibility Map
1. `apps/android/app`
   - Composition root and cross-feature orchestration.
   - Tab routing, screen-level action wiring, import/export entry flow.
2. `apps/android/feature-data`
   - Data tab UI and interaction widgets.
   - Should not own runtime bridge logic.
3. `apps/android/feature-record`
   - Record/TXT related UI state and view models.
   - Shared UI progress state for record-related actions.
4. `apps/android/feature-report`
   - Query/report UI and chart rendering pipeline.
5. `apps/android/runtime`
   - Android runtime adapter: gateway/services/translators/NativeBridge.
   - JNI call entry for Android host.
6. `apps/android/contract`
   - Cross-module contracts and result models.
   - First stop for shared model/interface changes.

## 3. First Routing Heuristic

1. UI wiring or tab-level actions:
   - start at `apps/android/app`
2. Data tab UI:
   - start at `apps/android/feature-data`
3. Record or TXT editor behavior:
   - start at `apps/android/feature-record`
4. Report or chart behavior:
   - start at `apps/android/feature-report`
5. JNI, native calls, runtime services, path/bootstrap behavior:
   - start at `apps/android/runtime`
6. Shared models or gateway signatures:
   - start at `apps/android/contract`

Detailed file-by-file routing lives in:

- `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`

## 4. Guardrails
1. Keep module boundaries stable:
   - `feature-*` should not call JNI directly.
   - JNI and native-call details stay in `runtime`.
2. Sync string resources in same change:
   - `values/strings.xml`
   - `values-zh/strings.xml`
   - `values-ja/strings.xml`
3. Do not scan/build generated folders by default:
   - `**/build/**`, `**/.gradle/**`, `**/.kotlin/**`, `**/.cxx/**`, `**/.externalNativeBuild/**`

## 5. Minimal Validation Commands
Run from repo root:

```powershell
python tools/run.py build --app tracer_android --profile android_edit
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
```

Always serialize Gradle-backed commands for `apps/android`; do not run multiple Gradle tasks in parallel.

## 6. When To Go Deeper

Open these only when needed:

1. file-level change routing:
   - `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
2. runtime/config bootstrap:
   - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
3. build/test/release execution:
   - `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
4. protocol and payload behavior:
   - `docs/time_tracer/presentation/android/runtime-protocol.md`
