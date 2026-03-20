# Android Build Workflow

This document holds Android build, test, and validation execution rules.

## Recommended Entry Points

Run from repo root unless a section explicitly says otherwise.

- Edit loop:
  - `python tools/run.py build --app tracer_android --profile android_edit`
- IDE-first minimal path:
  - `cd apps/android`
  - `.\gradlew.bat :app:assembleDebug`
  - `.\gradlew.bat :app:installDebug`
- Style verify:
  - `python tools/run.py verify --app tracer_android --profile android_style --concise`
- CI-like verify:
  - `python tools/run.py verify --app tracer_android --profile android_ci --concise`
- Full validation after local edits:
  - `python tools/run.py verify --app tracer_android --profile android_ci --scope batch --concise`
- Device verify:
  - `python tools/run.py verify --app tracer_android --profile android_device --concise`

## Gradle Rule

- Do not run Gradle commands for `apps/android` in parallel.
- Always serialize Gradle build/test/verify tasks for this app.
- Do not launch multiple `gradlew` or Gradle-backed `tools/run.py` commands at the same time against the same workspace.

## Direct Gradle

Run only when Python entrypoints are not enough:

```powershell
cd apps/android
$env:JAVA_HOME='C:\Application\Android\as\jbr'
.\gradlew.bat :app:assembleDebug
```

Useful targeted commands:

- `.\gradlew.bat :runtime:syncTracerCoreConfigSnapshot`
- `.\gradlew.bat :runtime:verifyTracerCoreConfigSnapshot`
- `.\gradlew.bat :runtime:testDebugUnitTest`
- `.\gradlew.bat :app:check`
- `.\gradlew.bat :app:qaRelease`
- `.\gradlew.bat :app:testDebugUnitTest`
- `.\gradlew.bat :app:assembleRelease -PtimeTracerDisableNativeOptimization=false`

## Validation Rule For Core-Side Changes

- If a change touches `libs/tracer_core/**`, `libs/tracer_core_bridge_common/**`, or `apps/tracer_core_shell/host/**` and can affect the Android host/runtime path, do not stop at CLI-only or core-only validation.
- In those cases, include `python tools/run.py build --app tracer_android --profile android_edit` before closing the task.

## Build Notes

- `tracer_android` uses the Gradle backend.
- Build artifacts are under `apps/android/build`.
- Do not pass `--build-dir` for `tracer_android`; fixed-dir backends reject that override.
- `assembleDebug` / `installDebug` are the smallest IDE path and must not depend on Python-backed sync or QA finalizers.
- `check` owns debug/local QA only: `ktlintCheck`, `lintDebug`, `testDebugUnitTest`, and debug packaging policy.
- `qaRelease` owns explicit release QA: `assembleRelease`, `lintRelease`, and release packaging policy.
- Python-backed config sync is explicit:
  - refresh snapshot: `:runtime:syncTracerCoreConfigSnapshot`
  - verify snapshot drift: `:runtime:verifyTracerCoreConfigSnapshot`
- Android Gradle build paths are fixed to the repository layout; external config-root/debug-asset override properties are no longer part of the supported build interface.
- `android_ci` maps to: `:runtime:verifyTracerCoreConfigSnapshot`, `:app:check`, `:app:qaRelease`
- To force optimized native build, pass `-PtimeTracerDisableNativeOptimization=false` or use `android_release`.

## Outputs

- `out/test/artifact_android/result.json`
- `out/test/artifact_android/result_cases.json`
- `out/test/artifact_android/logs/output.log`
- `out/validate/<run_name>/summary.json`

## Related Docs

- Onboarding: `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
- Structure: `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Config assets: `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- APK/release notes: `docs/time_tracer/presentation/android/apk-compilation-guide.md`
