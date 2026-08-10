# Android Architecture Overview

## Purpose

Provide a thin overview of the Android documentation and code layering.

## When To Open

- Open this when you need a quick architectural picture before drilling into `STRUCTURE.md` or the runtime protocol docs.

## What This Doc Does Not Cover

- File-level edit routing
- Full feature behavior
- Historical refactor detail

## Module Layers

- `app`
  - composition root and cross-feature wiring
- `contract`
  - stable gateway interfaces and shared models
- `feature-data`
  - Data management presentation rendered from the Config route
- `feature-record`
  - Record and TXT presentation
- `feature-insights`
  - insights and chart presentation
- `runtime`
  - runtime implementation, JNI integration, services, coreadapter, translators

## Dependency Direction

- `app -> contract + feature-* + runtime`
- `feature-* -> contract`
- `runtime -> contract`

## Runtime Boundary Shape

- Kotlin/Compose routes depend on contract interfaces.
- Android app routes should prefer the narrowest gateway they need, not the aggregate `RuntimeGateway`.
- Runtime execution and JNI details stay in `runtime`.
- `NativeRuntimeController` is the runtime facade/composition root.
- `RuntimeCoreAdapter` owns native init/query execution flow.
- Runtime services own capability-oriented behavior such as diagnostics, ingest, record, and crypto.
- Android candidate TXT/config imports use the Core pipeline-only Runtime. This
  path consumes `config/user/behavior.toml` and
  `config/user/activity_hierarchy/**`; it must not require or copy
  `config/program/**`. The complete Runtime still loads program resources for
  insights, charts, and other presentation capabilities.

## Open Next

- Stable structure:
  - `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Runtime protocol:
  - `docs/time_tracer/presentation/android/runtime-protocol.md`
- Core Runtime/pipeline boundary:
  - `docs/time_tracer/core/architecture/runtime_bootstrap_and_pipeline_scope.md`
- Behavior reference:
  - `docs/time_tracer/presentation/android/features.md`
