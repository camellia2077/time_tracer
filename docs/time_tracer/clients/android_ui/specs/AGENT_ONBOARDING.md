# tracer_android Agent Onboarding

This document is a fast entry for coding agents and new contributors.
Goal: locate the right files in 5 minutes and avoid broad repo-wide searching.

## 1. Read Order (5-Minute Path)
1. `docs/time_tracer/clients/android_ui/specs/STRUCTURE.md`
2. `apps/tracer_android/agent.md`
3. This file (`AGENT_ONBOARDING.md`)
4. Feature-specific docs as needed:
   - `docs/time_tracer/clients/android_ui/runtime-protocol.md`
   - `docs/time_tracer/clients/android_ui/specs/CONFIG_ASSET_LIFECYCLE.md`
   - `docs/time_tracer/clients/android_ui/specs/preference-storage.md`
   - `docs/time_tracer/clients/android_ui/specs/i18n-button-sync.md`

## 2. Module Responsibility Map
1. `apps/tracer_android/app`
   - Composition root and cross-feature orchestration.
   - Tab routing, screen-level action wiring, import/export entry flow.
2. `apps/tracer_android/feature-data`
   - Data tab UI and interaction widgets.
   - Should not own runtime bridge logic.
3. `apps/tracer_android/feature-record`
   - Record/TXT related UI state and view models.
   - Shared UI progress state for record-related actions.
4. `apps/tracer_android/feature-report`
   - Query/report UI and chart rendering pipeline.
5. `apps/tracer_android/runtime`
   - Android runtime adapter: gateway/services/translators/NativeBridge.
   - JNI call entry for Android host.
6. `apps/tracer_android/contract`
   - Cross-module contracts and result models.
   - First stop for shared model/interface changes.

## 3. Common Change Routing (Where To Edit)
1. Add/modify a Data tab button or progress area:
   - `apps/tracer_android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`
   - Wiring in `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt`
2. Change import/export behavior:
   - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
   - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenImports.kt`
3. Change report chart visualization behavior/UI:
   - orchestrator: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSection.kt`
   - mode contracts: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualMode.kt`
   - mode/hint controls: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationControls.kt`
   - heatmap controls: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationHeatmapControls.kt`
   - summary/footer: `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSummary.kt`
4. Change record-side shared UI state:
   - `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
5. Add/extend runtime service behavior:
   - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/services/*.kt`
   - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
6. Change runtime record write / TXT storage behavior:
   - facade: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
   - txt IO: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawTxtFileStore.kt`
   - normalization: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordNormalization.kt`
   - parsing: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordParsing.kt`
   - persistence: `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordPersistence.kt`
7. Add/extend JNI callback/event bridge:
   - Android side: `apps/tracer_android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt`
   - Core side: `apps/tracer_core/src/api/android/native_bridge_calls.cpp`
8. Add/extend shared contract fields:
   - `apps/tracer_android/contract/src/main/java/com/example/tracer/*.kt`
   - Then sync runtime parsing + UI usage.

## 4. Example Chain: File Crypto Progress
1. Core emits progress snapshot:
   - `apps/tracer_core/src/api/android/native_bridge_calls.cpp`
2. JNI bridge forwards JSON:
   - `apps/tracer_android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt`
3. Runtime parses and dispatches:
   - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeCryptoService.kt`
4. Contract receives typed event:
   - `apps/tracer_android/contract/src/main/java/com/example/tracer/FileCryptoProgress.kt`
5. App flow passes callback:
   - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenExports.kt`
   - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenImports.kt`
6. ViewModel updates UI state:
   - `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
7. Data tab renders progress:
   - `apps/tracer_android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`

## 5. Guardrails
1. Keep module boundaries stable:
   - `feature-*` should not call JNI directly.
   - JNI and native-call details stay in `runtime`.
2. Sync string resources in same change:
   - `values/strings.xml`
   - `values-zh/strings.xml`
   - `values-ja/strings.xml`
3. Do not scan/build generated folders by default:
   - `**/build/**`, `**/.gradle/**`, `**/.kotlin/**`, `**/.cxx/**`, `**/.externalNativeBuild/**`

## 6. Minimal Validation Commands
Run from repo root:

```powershell
python scripts/run.py build --app tracer_android --profile fast
python scripts/run.py verify --app tracer_android --profile android_style --concise
```

Use CI-like verify when needed:

```powershell
python scripts/run.py verify --app tracer_android --profile android_ci --concise
```

## 7. Troubleshooting First Checks
1. Build fails after contract change:
   - Check `contract -> runtime parser -> UI` sync completeness.
2. Progress shown but no updates:
   - Check callback registration/cleanup in `RuntimeCryptoService`.
   - Check `NativeBridge.onCryptoProgressJson(...)` path.
3. UI updated but text missing:
   - Check string resources across 3 locales.
