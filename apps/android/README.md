# android

Android host app (`Jetpack Compose + JNI`) for `apps/tracer_core_shell`.

This file is the short local entrypoint. Detailed design and workflow docs live under `docs/time_tracer/presentation/android/**`.

## Quick Start

1. Create `apps/android/local.properties`
2. Ensure `JAVA_HOME` points to JDK 17+
3. Use Android NDK `29.0.14206865`

## Common Commands

Run from repo root:

```bash
python tools/run.py build --app tracer_android --profile android_edit
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
```

Gradle rule:

- Do not run Gradle commands for `apps/android` in parallel.
- Always serialize Gradle build/test/verify tasks for this app.

## Local Facts

- Runtime config source of truth: `assets/tracer_core/config`
- Android runtime config copy: `apps/android/runtime/src/main/assets/tracer_core/config`
- Android app version source: `apps/android/meta/version.properties`
- Release signing template: `apps/android/keystore.properties.example`

## Docs Index

- Android docs home:
  - `docs/time_tracer/presentation/android/README.md`
- Agent onboarding:
  - `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
- Structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Build workflow:
  - `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
- Edit routing:
  - `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
- Config asset lifecycle:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Runtime protocol:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- APK/release notes:
  - `docs/time_tracer/presentation/android/apk-compilation-guide.md`
