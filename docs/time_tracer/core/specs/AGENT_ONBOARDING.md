# tracer_core Agent Onboarding

Use this document to locate the right core-adjacent entrypoint in a few minutes
without broad repository search.

## 5-Minute Reading Order
1. [Library Dependency Map](../../architecture/library_dependency_map.md)
2. [tracer_core](../../architecture/libraries/tracer_core.md)
3. This file: `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`
4. Then read the exact contract docs for the boundary you are touching:
   - [C ABI](../contracts/c_abi.md)
   - [Stats contracts](../contracts/stats/README.md)
   - [Android runtime protocol](../../clients/android_ui/runtime-protocol.md)
   - [Core JSON boundary design](../architecture/core_json_boundary_design.md)

## Who Owns What
1. `libs/tracer_core`
   - business rules, DTOs, module ownership, and core infrastructure families
2. `apps/tracer_core_shell/api/c_api`
   - shell-facing C ABI entrypoints and runtime glue
3. `apps/tracer_core_shell/api/android_jni`
   - Android JNI entrypoints and registration
4. `libs/tracer_core_bridge_common`
   - shared helper logic used by C API / JNI bridge code
5. `libs/tracer_transport`
   - runtime envelope, field, and codec implementation
6. `libs/tracer_adapters_io`
   - file-system ingest and processed-data adapter behavior

## High-Frequency Change Routes
1. Change C ABI inputs, outputs, or runtime behavior:
   - read [C ABI](../contracts/c_abi.md)
   - start in `apps/tracer_core_shell/api/c_api`
   - pair with [tracer_transport](../../architecture/libraries/tracer_transport.md)
     if JSON payload parsing or envelope behavior changes
2. Change Android JNI bridge behavior or code/value translation:
   - read [Android runtime protocol](../../clients/android_ui/runtime-protocol.md)
   - start in `apps/tracer_core_shell/api/android_jni`
   - pair with [tracer_core_bridge_common](../../architecture/libraries/tracer_core_bridge_common.md)
3. Change core config ownership or shell config bridging:
   - start in [tracer_core](../../architecture/libraries/tracer_core.md)
   - then inspect:
     - `apps/tracer_core_shell/api/c_api`
     - `apps/tracer_core_shell/host`
4. Change use cases, workflow, or reporting-tree boundaries:
   - start in [tracer_core](../../architecture/libraries/tracer_core.md)
   - then inspect:
     - `libs/tracer_core/src/application/use_cases`
     - `libs/tracer_core/src/application/workflow`
     - `libs/tracer_core/src/application/reporting/tree`
5. Change query/report/stat output semantics:
   - read [Stats contracts](../contracts/stats/README.md)
   - start in `libs/tracer_core/src/infrastructure/query`
   - then inspect `libs/tracer_core/src/infrastructure/reports`
6. Change file-system ingest or processed-data IO:
   - start in [tracer_adapters_io](../../architecture/libraries/tracer_adapters_io.md)

## Search Scope Guidance
Prefer scanning only the smallest relevant set:
1. `docs/time_tracer/architecture`
2. `docs/time_tracer/core`
3. The one or two libraries named by the dependency map
4. `apps/tracer_core_shell` only when the change crosses the shell/runtime boundary

Default exclusions:
1. `out/**`
2. `build/**`
3. `.cache/**`

## Module-First Defaults
1. For `libs/tracer_core` non-boundary consumers, default to `import tracer.*`
   when a canonical module surface already exists.
2. Keep explicit direct includes when the file is:
   - the owner implementation for that header's declarations
   - a stable boundary path such as `C ABI`, `JNI`, host bridge,
     `android_runtime`, or `tracer_transport` public-header code
3. If an `import` conversion breaks because a standard-library type suddenly
   disappears, add the missing `<...>` include explicitly instead of reverting
   to the old project-header include.
4. Prefer a narrow local `using` or namespace alias in the consumer before
   redesigning the wrapper surface.

## Validation Shortcuts
1. Focused change:

```powershell
python tools/run.py validate --plan <plan_name> --paths <touched paths>
```

2. Shell/runtime integration:

```powershell
python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise
```

3. Before editing module or explicit boundary declaration surfaces, also inspect
   the relevant module smoke tests and contract regressions listed in the touched detailed library doc under
   `docs/time_tracer/architecture/libraries/`.

## Common Failure Checks
1. If a runtime payload change breaks consumers, re-check:
   - `c_abi.md`
   - `runtime-protocol.md`
   - `docs/time_tracer/architecture/libraries/tracer_transport.md`
2. If config or bridge changes fail under `modules_on`, re-check:
   - `docs/time_tracer/architecture/libraries/tracer_core.md`
   - `apps/tracer_core_shell/api/c_api`
   - `apps/tracer_core_shell/host`
3. If stats or report output regresses, re-check:
   - `contracts/stats/*`
   - `libs/tracer_core/src/infrastructure/query`
   - `libs/tracer_core/src/infrastructure/reports`
