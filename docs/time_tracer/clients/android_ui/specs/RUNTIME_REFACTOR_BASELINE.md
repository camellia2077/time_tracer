# tracer_android Runtime Refactor Baseline

This checklist locks guardrails for Android runtime refactor phases.

## 1. Boundary Guardrails

1. Runtime `coreadapter/services/translators` layers must not depend on UI resources.
2. Feature modules must not parse native raw JSON directly.
3. New error handling should use typed domain error first, with legacy `message` as compatibility output.
4. Semantic JSON compatibility must follow:
   - `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
   - `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`

## 2. Baseline Verification Commands

Run from repository root:

```powershell
python scripts/run.py verify --app tracer_android --profile android_style --concise
python scripts/run.py verify --app tracer_android --profile android_ci --concise
```

## 3. Baseline Unit Test Set

Primary regression anchors for refactor:

1. `apps/tracer_android/app/src/test/java/com/example/tracer/TracerTabRegistryTest.kt`
2. `apps/tracer_android/app/src/test/java/com/example/tracer/DomainResultCompatibilityTest.kt`
3. `apps/tracer_android/feature-report/src/test/java/com/example/tracer/QueryPeriodArgumentResolverTest.kt`
4. `apps/tracer_android/feature-report/src/test/java/com/example/tracer/QueryReportViewModelChartTest.kt`
5. `apps/tracer_android/feature-report/src/test/java/com/example/tracer/QueryReportViewModelStatsTest.kt`
6. `apps/tracer_android/runtime/src/test/java/com/example/tracer/NativeRuntimeQueryOpsTest.kt`

## 4. Runtime Package Naming Convention (In-Module)

Before physical module split, keep logical package layout in `:runtime`:

1. `runtime/coreadapter/*`
2. `runtime/services/*`
3. `runtime/translators/*`

Current Phase 2 landing path:

1. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/translators/NativeQueryTranslator.kt`
2. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/translators/NativeReportTranslator.kt`
3. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/translators/NativeRecordTranslator.kt`
