# tracer_android

Android host app for `apps/tracer_core` (`Jetpack Compose + JNI`).

## Scope Policy (Read First)

- The directories/docs listed in this file are recommended entry points, not a mandatory reading checklist.
- If a task can be solved by direct symbol/path search (`rg`) or by editing a clearly scoped file, you may skip unrelated sections.
- Prioritize relevance: open only files needed for the current change, then expand scope only when blocked.

## Architecture Docs

- Agent onboarding (first-stop index for coding agents):
  - `docs/time_tracer/clients/android_ui/specs/AGENT_ONBOARDING.md`
- Structure:
  - `docs/time_tracer/clients/android_ui/specs/STRUCTURE.md`
- Runtime refactor baseline:
  - `docs/time_tracer/clients/android_ui/specs/RUNTIME_REFACTOR_BASELINE.md`
- Preference storage:
  - `docs/time_tracer/clients/android_ui/specs/preference-storage.md`
- i18n button text sync:
  - `docs/time_tracer/clients/android_ui/specs/i18n-button-sync.md`
- Config/asset lifecycle:
  - `docs/time_tracer/clients/android_ui/specs/CONFIG_ASSET_LIFECYCLE.md`
- Core stats semantic JSON contract:
  - `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`
  - `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
  - `docs/time_tracer/core/contracts/stats/README.md`

## Kotlin Split Landmarks

- Config import/export split:
  - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigViewModel.kt`
  - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigBundleTransferUseCase.kt`
  - `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigBundleTransferModels.kt`
- Report result split:
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartParameterSection.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSection.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualMode.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationControls.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationHeatmapControls.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSummary.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownRenderer.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownParser.kt`
- Report chart pipeline split:
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartPipeline.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartParamResolver.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartMappers.kt`
  - `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartModels.kt`
- Runtime record store split:
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawTxtFileStore.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordNormalization.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordParsing.kt`
  - `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordPersistence.kt`

## Setup

1. Create `apps/tracer_android/local.properties`:
   ```properties
   sdk.dir=C\:\\Application\\Android\\SDK
   # cmake.dir=... (optional, if not in PATH or SDK)
   ```
2. Ensure `JAVA_HOME` points to JDK 17+.
3. NDK requirement:
   - `29.0.14206865`

## Agent Scan Scope (Ignore Generated Artifacts)

When scanning or grepping this module, prioritize source directories:

- `apps/tracer_android/app/src`
- `apps/tracer_android/feature-data/src`
- `apps/tracer_android/feature-report/src`
- `apps/tracer_android/feature-record/src`
- `apps/tracer_android/runtime/src`
- `apps/tracer_android/contract/src`

Exclude generated/build outputs by default:

- `apps/tracer_android/**/build/**`
- `apps/tracer_android/**/.gradle/**`
- `apps/tracer_android/**/.kotlin/**`
- `apps/tracer_android/**/.cxx/**`
- `apps/tracer_android/**/.externalNativeBuild/**`
- `apps/tracer_android/runtime/runtime/**`
- `test/output/**`

Reference search command:

```powershell
rg -n --glob '!**/build/**' --glob '!**/.gradle/**' --glob '!**/.kotlin/**' --glob '!**/.cxx/**' --glob '!**/.externalNativeBuild/**' --glob '!runtime/runtime/**' "PATTERN" apps/tracer_android
```

## Build (Python Entry, Recommended)

Run from repo root:

```powershell
# Daily post-change command (only)
python scripts/run.py build --app tracer_android --profile fast
```

Compile/test command policy (fixed entrypoints):

- Build: `python scripts/run.py build --app tracer_android --profile fast`
- Style verify: `python scripts/run.py verify --app tracer_android --profile android_style --concise`
- CI-like verify: `python scripts/run.py verify --app tracer_android --profile android_ci --concise`
- Device verify: `python scripts/run.py verify --app tracer_android --profile android_device --concise`

## Non-daily Optional Commands

Use only when explicitly needed:

```powershell
# Style gate
python scripts/run.py verify --app tracer_android --profile android_style --concise

# CI-like gate
python scripts/run.py verify --app tracer_android --profile android_ci --concise

# Device gate
python scripts/run.py verify --app tracer_android --profile android_device --concise

# Release APK with native optimization enabled
python scripts/run.py build --app tracer_android --profile android_release

# Release APK without native optimization
python scripts/run.py build --app tracer_android --profile android_release_no_opt
```

Note:
- Daily workflow after local edits is fixed to:
  - `python scripts/run.py build --app tracer_android --profile fast`
- Python build flow now defaults to fast-compile-first on Android:
  - If `-PtimeTracerDisableNativeOptimization` is not explicitly passed,
    scripts auto-inject `-PtimeTracerDisableNativeOptimization=true`.
  - To force optimized native build, pass
    `-PtimeTracerDisableNativeOptimization=false` or use `android_release` profile.
- `android_device` profile is an explicit non-default entry; default verify flow does not require a device.
- `tracer_android` uses Gradle backend; build artifacts are under `apps/tracer_android/build`.
- For `tracer_android`, do not rely on `build_fast` semantics used by CMake apps.
- `post-change` state is written to:
  - `apps/tracer_android/build/post_change_last.json`
- Machine-readable results:
  - `test/output/tracer_android/result.json`
  - `test/output/tracer_android/result_cases.json`
  - `test/output/tracer_android/logs/output.log`
- Canonical integration input source is `test/data`; Android runtime assets `input/full` must be synced from it.
- Unit/component tests should use small fixtures and avoid relying on large integration datasets.

## Build (Direct Gradle, Optional)

Powershell:

```powershell
cd apps/tracer_android
$env:JAVA_HOME='C:\Application\Android\as\jbr'
.\gradlew :app:assembleDebug
```

Native CMake entry:

- `runtime/src/main/cpp/CMakeLists.txt`

## Regression

Use:

- `apps/tracer_android/REGRESSION_CHECKLIST.md`

## Domain Rules (Reference)

- Preference persistence rules:
  - `apps/tracer_android/docs/PREFERENCE_STORAGE.md`
- i18n button text sync rules:
  - `apps/tracer_android/docs/I18N_BUTTON_TEXT_SYNC.md`
- XML i18n sync rule:
  - When adding/removing/updating string keys in Android XML resources,
    keep these files in sync in the same change:
    - `apps/tracer_android/**/res/values/strings.xml` (English baseline)
    - `apps/tracer_android/**/res/values-zh/strings.xml` (Chinese)
    - `apps/tracer_android/**/res/values-ja/strings.xml` (Japanese)
  - Do not leave new/changed keys only in one locale unless explicitly intended.
- Keep these two documents as the single source of truth for domain-specific UI rules.

