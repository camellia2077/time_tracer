# Android Build Workflow

## Purpose

Define the supported Android build, verify, and validation entrypoints.

## When To Open

- Open this before running Android build or verification commands.
- Use it to choose the smallest safe validation path.

## What This Doc Does Not Cover

- Feature behavior
- File routing
- Runtime payload details

## Recommended Entry Points

Run from repo root unless a section explicitly says otherwise.

- Edit loop:
  - `python tools/run.py build --app tracer_android --profile android_edit`
- Style verify (only when explicitly requested):
  - `python tools/run.py verify --app tracer_android --profile android_style --concise`
- CI/full verify (explicit request or release/merge readiness only):
  - `python tools/run.py verify --app tracer_android --profile android_ci --concise`
- Release verify:
  - `python tools/run.py verify --app tracer_android --profile android_release_verify --concise`
- Release device smoke:
  - `python tools/run.py verify --app tracer_android --profile android_release_device --concise`
- Combined style + CI/full closeout in one Gradle invocation (only when both are explicitly requested):
  - `python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise`
- Device verify:
  - `python tools/run.py verify --app tracer_android --profile android_device --concise`

## Gradle Rule

- Do not run Gradle commands for `apps/android` in parallel.
- Do not launch multiple `gradlew` or Gradle-backed `tools/run.py` commands at the same time against the same workspace.
- Repeating `--profile` for `tracer_android` is allowed only when `tools/run.py` merges them into one Gradle invocation.
- Multi-profile merge does not make this workspace safe for concurrent Gradle processes.

## Direct Gradle

Use direct Gradle only when the Python entrypoints are not enough:

```powershell
cd apps/android
$env:JAVA_HOME='C:\Program Files\Android\Android Studio\jbr'
.\gradlew.bat :app:assembleDebug
```

Common targeted commands:

- `.\gradlew.bat :runtime:syncTracerCoreConfigSnapshot`
- `.\gradlew.bat :runtime:verifyTracerCoreConfigSnapshot`
- `.\gradlew.bat :runtime:testDebugUnitTest`
- `.\gradlew.bat :app:check`
- `.\gradlew.bat :app:qaRelease`
- `.\gradlew.bat :app:testDebugUnitTest`

## Validation Rule

- For normal Android changes, do not run `android_style` by default.
  - For Android UI changes, run the smallest relevant targeted compile or unit
    test, plus focused state/render coverage when applicable.
  - For runtime, contracts, or build behavior changes, run the smallest
    relevant targeted build or unit check and any affected snapshot/config
    verification.
  - Run `android_style` only when the user explicitly requests style validation.
- If the Android host/runtime path is affected, include:
  - `python tools/run.py build --app tracer_android --profile android_edit`
- Do not run `android_ci` by default.
  - Run it only when the user explicitly requests CI/full validation or when
    release/merge readiness is being checked.
  - When it is required, prefer one merged invocation:
    `python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise`.
- If you need release-specific QA or signing validation:
  - run `python tools/run.py verify --app tracer_android --profile android_release_verify --concise`
  - this path requires the existing release signing inputs and is intentionally separate from default CI.
- If you need a real signed-APK startup smoke on a connected device:
  - run `python tools/run.py verify --app tracer_android --profile android_release_device --concise`
  - this path requires release signing and an attached device or emulator with `adb`.
- If a change touches core-side code that affects the Android host/runtime path,
  rebuild the affected core/runtime artifacts before treating Android validation
  as representative.

## Output Locations

- `out/test/artifact_android/result.json`
- `out/test/artifact_android/result_cases.json`
- `out/test/artifact_android/logs/output.log`
- `apps/android/app/build/outputs/final-apk/release/TimeTracer-release.apk`

## Related Docs

- Onboarding:
  - `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
- Structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Config lifecycle:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
