# Android Runtime Protocol

## Purpose

Describe the stable Android runtime boundary contracts between Kotlin, JNI, and core C ABI calls.

## When To Open

- Open this when the task changes JNI method contracts, runtime payload shapes, or C ABI alignment.

## What This Doc Does Not Cover

- UI routing
- Feature behavior
- File-level ownership

## Boundary Model

Runtime call chain:

1. Kotlin/Compose calls JNI-facing native methods.
2. JNI calls core C ABI entrypoints (`tracer_core_*`).
3. C ABI forwards into core/runtime implementation.

Important rules:

- Kotlin does not call `tracer_core_*` directly.
- Kotlin-visible runtime APIs remain JNI methods.
- Business payloads between JNI and core remain UTF-8 JSON strings.

## C ABI Scope

Current Android JNI integration uses C ABI entrypoints in these categories:

- runtime create/destroy
- ingest/query/tree/report
- structure/logic validation
- last-error access

Canonical global rules live in:

- `docs/time_tracer/core/contracts/c_abi.md`

## Stable Response Envelope

Kotlin-visible JNI responses are normalized to:

- `ok`
- `error_message`
- `content`

Some operations may add operation-specific fields, but the envelope shape stays stable.

## Transport Status

Current status:

- JNI request encoding for main ingest/query/tree/report paths is unified through shared transport helpers.
- Validation requests still have JNI-local request assembly.
- Tree responses are normalized before returning to Kotlin.
- Kotlin-visible response shape remains `{ok,error_message,content}`.
- JNI native method signatures remain stable.

## Crypto Progress Note

- Android crypto progress uses the same snapshot-to-JSON callback path as the core C ABI.
- Android host intentionally exposes only the Android-supported security levels.

## Open Next

- Stable structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Config/runtime bootstrap:
  - `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
