# Android Agent Entry

Agent-only navigation for work under `apps/android`. Use this before broad
searching, then follow the closest referenced doc.

## Read Next

1. `docs/time_tracer/presentation/android/README.md`
2. `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
3. `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
4. `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`

Open only when relevant:

- Structure: `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Runtime/config lifecycle:
  `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Runtime protocol: `docs/time_tracer/presentation/android/runtime-protocol.md`
- User behavior reference: `docs/time_tracer/presentation/android/features.md`
- Activity doc rules: `docs/time_tracer/presentation/android/specs/DOC_RULES.md`

For exchange, TXT import/export, SAF/document, or fd export behavior, also read:

1. `docs/time_tracer/presentation/android/reference/data-import-export.md`
2. `docs/time_tracer/presentation/android/runtime-protocol.md`
3. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
4. `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`

## Code Map

- `apps/android/app`: composition root and app-local wiring
- `apps/android/feature-data`: Data tab UI
- `apps/android/feature-record`: Record and TXT UI
- `apps/android/feature-report`: report, query, and chart UI
- `apps/android/runtime`: runtime/JNI implementation
- `apps/android/contract`: gateway interfaces and shared models

## Rules

- Shared config source of truth: `assets/tracer_core/config`.
- Generated Android config snapshot:
  `apps/android/runtime/src/main/assets/tracer_core/config`.
- Android app version source: `apps/android/meta/version.properties`.
- Core version source: `libs/tracer_core/src/shared/types/version.hpp`.
- Do not run Gradle commands for `apps/android` in parallel.
- Prefer `python tools/run.py ...` for standard Android build/verify flows.
- Direct Gradle is allowed for targeted module/debug validation when it is the
  smaller, more precise command.
- Use the smallest command that safely validates the change.
- Android multi-profile `tools/run.py` is allowed only when it produces one
  Gradle invocation; otherwise run profiles serially.
- Keep `android_device` focused on real app-shell/device flows. Hostless
  composable checks that only call `setContent { ... }` are flaky on connected
  devices here and should live in local/unit coverage instead of the device
  profile.
- `RuntimeGateway` remains an aggregate contract surface. UI routes and
  app-side tests should prefer the smallest gateway interface they need.

## Validation

Use `python tools/run.py verify -h` for current flags. Common repo-root flows:

```powershell
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise
python tools/run.py verify --app tracer_android --profile android_release_verify --concise
python tools/run.py verify --app tracer_android --profile android_release_device --concise
```

## Test Assets

- Android and Windows CLI share `test/data/**` as canonical TXT input.
- Prefer `test/fixtures/config/**` and `test/fixtures/text/**` for Android
  compat/runtime error paths.
- `test/golden/**` stores stable final baselines only.
- Do not write Android runtime results, temp databases, or exports back to
  `test/**`; use `out/test/**`.
- `apps/tools/log_generator` generates canonical TXT data; do not move its
  outputs into `test/**` unless a test fixture explicitly requires it.
