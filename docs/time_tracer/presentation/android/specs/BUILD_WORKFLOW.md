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
- Style verify:
  - `python tools/run.py verify --app tracer_android --profile android_style --concise`
- CI-like verify:
  - `python tools/run.py verify --app tracer_android --profile android_ci --concise`
- Release verify:
  - `python tools/run.py verify --app tracer_android --profile android_release_verify --concise`
- Combined closeout in one Gradle invocation:
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
$env:JAVA_HOME='C:\Application\Android\as\jbr'
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

- If a change touches Android UI only:
  - `android_style` is the minimum closeout.
- If a change touches runtime, contracts, or build behavior:
  - run both `android_style` and `android_ci`.
  - Prefer one merged invocation when practical:
    `python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise`
- If you need release-specific QA or signing validation:
  - run `python tools/run.py verify --app tracer_android --profile android_release_verify --concise`
  - this path requires the existing release signing inputs and is intentionally separate from default CI.
- If a change touches core-side code that affects the Android host/runtime path:
  - include `python tools/run.py build --app tracer_android --profile android_edit`.

## Output Locations

- `out/test/artifact_android/result.json`
- `out/test/artifact_android/result_cases.json`
- `out/test/artifact_android/logs/output.log`
- `out/validate/<run_name>/summary.json`
- `apps/android/app/build/outputs/final-apk/release/Tracer.apk`

## Related Docs

- Onboarding:
  - `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
- Structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Config lifecycle:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
