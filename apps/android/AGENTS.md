# android

## Purpose

Local navigation entry for coding agents working in `apps/android`.

## When To Open

- Open this first when the task touches Android code.
- Use it to find the next 2 to 4 documents to read.

## What This Doc Does Not Cover

- Detailed module boundaries
- File-by-file edit routing
- Runtime payload details
- Product behavior reference

## 5-Minute Path

1. `docs/time_tracer/presentation/android/README.md`
2. `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
3. `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
4. `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`

Open additional docs only when needed:

- Stable boundaries:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Runtime/config bootstrap:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- JNI/runtime payloads:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- Behavior reference:
  - `docs/time_tracer/presentation/android/features.md`
- Activity doc rules:
  - `docs/time_tracer/presentation/android/specs/DOC_RULES.md`

## Exchange Docs

Open these when the task is specifically about Android tracer exchange export /
import, TXT import-export behavior, or SAF/document/fd export behavior:

1. `docs/time_tracer/presentation/android/reference/data-import-export.md`
2. `docs/time_tracer/presentation/android/runtime-protocol.md`
3. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
4. `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`

## Hard Rules

- Shared config source of truth is `assets/tracer_core/config`.
- Android runtime config snapshot under `apps/android/runtime/src/main/assets/tracer_core/config` is generated, not canonical.
- Android app version source is `apps/android/meta/version.properties`.
- Core version source is `libs/tracer_core/src/shared/types/version.hpp`.
- Do not run Gradle commands for `apps/android` in parallel.
- For standard Android build/verify flows, `python tools/run.py` is the recommended entrypoint.
- For targeted debugging or narrower module/task validation, direct Gradle is also allowed when it is the more precise tool.
- Choose the smallest command that safely validates the change.
- Multi-profile `tools/run.py` merge is allowed for Android only when it still results in one Gradle invocation.
  - Example: `python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise`

## Code Areas

- Composition root and app-local wiring:
  - `apps/android/app`
- Data UI:
  - `apps/android/feature-data`
- Record/TXT UI:
  - `apps/android/feature-record`
- Report/chart UI:
  - `apps/android/feature-report`
- Runtime/JNI implementation:
  - `apps/android/runtime`
- Shared contracts:
  - `apps/android/contract`

## Validation

Run from repo root for standard validation flows:

```powershell
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise
```

## Boundary Notes

- `RuntimeGateway` remains a contract-layer aggregate, but Android `app` routes and app-side tests should prefer the smallest gateway interfaces they actually need.
- `NativeRuntimeController` may still implement `RuntimeGateway`, but treat that aggregate as a runtime/composition-root boundary, not the default UI entrypoint.
