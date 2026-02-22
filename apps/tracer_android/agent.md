# tracer_android

Android host app for `apps/time_tracer` (`Jetpack Compose + JNI`).

## Architecture Docs

- Overview:
  - `docs/time_tracer/android_ui/architecture.md`
- Full structure:
  - `apps/tracer_android/STRUCTURE.md`
- Preference storage:
  - `apps/tracer_android/docs/PREFERENCE_STORAGE.md`
- i18n button text sync:
  - `apps/tracer_android/docs/I18N_BUTTON_TEXT_SYNC.md`
- Core stats semantic JSON contract:
  - `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
  - `docs/time_tracer/core/contracts/stats/README.md`

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
- Style verify: `python scripts/verify.py --app tracer_android --profile android_style --concise`
- CI-like verify: `python scripts/verify.py --app tracer_android --profile android_ci --concise`
- Device verify: `python scripts/verify.py --app tracer_android --profile android_device --concise`

## Non-daily Optional Commands

Use only when explicitly needed:

```powershell
# Style gate
python scripts/verify.py --app tracer_android --profile android_style --concise

# CI-like gate
python scripts/verify.py --app tracer_android --profile android_ci --concise

# Device gate
python scripts/verify.py --app tracer_android --profile android_device --concise

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
- `android_device` profile is an explicit non-default入口; default verify flow does not require a device.
- `tracer_android` uses Gradle backend; build artifacts are under `apps/tracer_android/build`.
- For `tracer_android`, do not rely on `build_fast` semantics used by CMake apps.
- `post-change` state is written to:
  - `apps/tracer_android/build/post_change_last.json`
- Machine-readable results:
  - `test/output/tracer_android/result.json`
  - `test/output/tracer_android/result_cases.json`
  - `test/output/tracer_android/logs/output.log`

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
- Keep these two documents as the single source of truth for domain-specific UI rules.

