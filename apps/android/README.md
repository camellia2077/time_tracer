# android

## Purpose

Human-facing local entrypoint for the Android host app workspace.

## When To Open

- Open this when you need local setup, common commands, or the main Android doc index.

## What This Doc Does Not Cover

- Detailed module boundaries
- File-level routing
- Runtime protocol details
- Historical notes

## Quick Start

1. Create `apps/android/local.properties`.
2. Ensure `JAVA_HOME` points to JDK 17+.
3. Use Android NDK `29.0.14206865`.

## Common Commands

For standard build/verify flows, run from repo root:

```bash
python tools/run.py build --app tracer_android --profile android_edit
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise
python tools/run.py verify --app tracer_android --profile android_release_verify --concise
```

Gradle rule:

- Do not run Gradle commands for `apps/android` in parallel.
- Always serialize Gradle build/test/verify tasks for this app.
- `python tools/run.py` is the recommended entrypoint for standard Android workflows.
- Direct Gradle is also acceptable for targeted debugging or narrower module/task validation when it is the more precise tool.
- Choose the smallest command that safely validates the change.
- Repeating `--profile` for `tracer_android` is supported only when `tools/run.py` merges them into one Gradle invocation; it does not make this workspace safe for multiple concurrent Gradle processes.
- `android_ci` is the signing-free default CI path; release QA lives under `android_release_verify`.

## Local Facts

- Runtime config source of truth: `assets/tracer_core/config`
- Android runtime config snapshot: `apps/android/runtime/src/main/assets/tracer_core/config`
- Android app version source: `apps/android/meta/version.properties`
- Core business version source: `libs/tracer_core/src/shared/types/version.hpp`
- Release signing template: `apps/android/keystore.properties.example`

## Start Here

- Docs hub:
  - `docs/time_tracer/presentation/android/README.md`
- Agent onboarding:
  - `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
- Stable structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Change routing:
  - `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`
- Build and validation:
  - `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`

Open only when needed:

- Runtime/config lifecycle:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Runtime protocol:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- Behavior reference:
  - `docs/time_tracer/presentation/android/features.md`

## Exchange Docs

Open these when the task touches Android-side tracer exchange export/import,
TXT import/export flow, or SAF/document/fd export behavior:

- Android import/export behavior:
  - `docs/time_tracer/presentation/android/reference/data-import-export.md`
- Android runtime payload/protocol surface:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- Core tracer exchange package contract:
  - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
- Core runtime crypto JSON contract:
  - `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`

## Boundary Notes

- Android `app` wiring should inject the smallest gateway interface a route needs.
- `RuntimeGateway` stays in `contract` as the aggregate runtime surface, but it is not the preferred default dependency for UI routes or app-side tests.
