# Android Host App

Human-facing entrypoint for the Android workspace.

## Quick Start

1. Create `apps/android/local.properties`.
2. Use JDK 17+.
3. Use Android NDK `29.0.14206865`.

## Common Commands

Run standard flows from the repository root:

```bash
python tools/run.py build --app tracer_android --profile android_edit
python tools/run.py verify --app tracer_android --profile android_ci --concise
python tools/run.py verify --app tracer_android --profile android_release_verify --concise
python tools/run.py verify --app tracer_android --profile android_release_device --concise
```

Notes:

- Do not run Gradle commands for `apps/android` in parallel.
- Prefer `python tools/run.py` for standard workflows.
- Direct Gradle is fine for targeted debugging or narrower module validation.
- `android_style` is an explicit opt-in style check, not part of the default
  Android change validation.
- `android_ci` is signing-free; release QA is `android_release_verify`.
- `android_release_device` is the connected-device release smoke path: it installs the signed release APK and verifies `MainActivity` can launch without an immediate crash.

## Local Facts

- Runtime program-resource source of truth:
  `config/program`
- User-config seed:
  `config/user_config`
- Test activity-hierarchy source, not packaged into APK:
  `test/data/activity_hierarchy`
- Android runtime config assets are generated during Gradle builds under:
  `apps/android/runtime/build/generated/tracer/assets/config/program`
- Android APK builds use the root `config/program` for immutable program
  resources. Test TXT and hierarchy data are injected separately through the
  ADB helper; they are not selected at build time.
- Android app version source: `apps/android/meta/version.properties`
- Core business version source: `libs/tracer_core/src/shared/types/version.hpp`
- Release signing template: `apps/android/keystore.properties.example`

## Documentation

- Android docs hub: `docs/time_tracer/presentation/android/README.md`
- Structure: `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Change routing: `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
- Build workflow: `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
- Runtime/config lifecycle:
  `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Runtime protocol: `docs/time_tracer/presentation/android/runtime-protocol.md`
- Behavior reference: `docs/time_tracer/presentation/android/features.md`

For tracer exchange, TXT import/export, SAF/document, or fd export behavior:

- `docs/time_tracer/presentation/android/reference/data-import-export.md`
- `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v6.md`
- `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`

## Boundary Notes

- Android `app` wiring should inject the smallest gateway interface a route
  needs.
- `RuntimeGateway` is the aggregate runtime surface in `contract`, not the
  preferred default dependency for UI routes or app-side tests.
