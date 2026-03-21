# Android Runtime Refactor Baseline

## Purpose

Capture the guardrails for Android runtime refactors.

## When To Open

- Open this before changing runtime layering, translators, services, or execution paths.

## What This Doc Does Not Cover

- UI routing
- Feature behavior
- Historical refactor notes

## Boundary Guardrails

- Runtime `coreadapter`, `services`, and `translators` must not depend on UI resources.
- Feature modules must not parse raw native JSON directly.
- Prefer typed domain errors first, while keeping legacy `message` compatibility where needed.
- Keep semantic JSON compatibility aligned with the core contract docs.

## Baseline Validation

Run from repo root:

```powershell
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
```

## Baseline Regression Anchors

- `apps/android/app/src/test/java/com/example/tracer/TracerTabRegistryTest.kt`
- `apps/android/app/src/test/java/com/example/tracer/DomainResultCompatibilityTest.kt`
- `apps/android/feature-report/src/test/java/com/example/tracer/QueryPeriodArgumentResolverTest.kt`
- `apps/android/feature-report/src/test/java/com/example/tracer/QueryReportViewModelChartTest.kt`
- `apps/android/feature-report/src/test/java/com/example/tracer/QueryReportViewModelStatsTest.kt`
- `apps/android/runtime/src/test/java/com/example/tracer/NativeRuntimeQueryOpsTest.kt`

## Runtime Layout Rule

Before any future physical module split, keep logical runtime layering inside `:runtime`:

- `runtime/coreadapter/*`
- `runtime/services/*`
- `runtime/translators/*`
