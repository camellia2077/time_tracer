# tracer_core

## Purpose

`tracer_core` is the main engine library for `time_tracer`.
It owns the core business rules, application orchestration, DTO/type surfaces,
module declaration ownership, and the internal infrastructure families that
implement persistence, reporting, query, config, logging, crypto, and platform
support.

## Layer Position

1. This library is the business-logic owner.
2. Host adapters in `apps/tracer_core_shell` call into this library.
3. Helper libraries such as `tracer_core_bridge_common`,
   `tracer_transport`, and `tracer_adapters_io` sit around it, not above its
   business logic.

## Allowed Dependencies

1. `shared`
   - standard library and shared internal helpers only
2. `domain`
   - `shared`
3. `application`
   - `domain`
   - `shared`
   - application-owned DTOs, ports, and interfaces
4. `infrastructure`
   - `shared`
   - `domain`
   - `application`
   - implementation dependencies needed by adapters/persistence/reporting/etc.

## Forbidden Dependencies

1. `domain` and `application` must not depend on:
   - `libs/tracer_transport`
   - `libs/tracer_adapters_io`
   - `libs/tracer_core_bridge_common`
   - `apps/**`
2. `domain` and `application` must not expose `nlohmann::json`.
3. Runtime envelope and request/response wire-shape ownership does not live here.

## Public Surfaces

1. Canonical module surfaces under `src/*/modules`
2. Explicit boundary declaration headers under `src/**` where the named-module
   interface is not the contract owner
3. Core DTOs, domain types, ports, and infrastructure boundaries consumed by
   shell-facing adapters and tests

## Module-First Guardrails

1. For non-boundary consumers, prefer `import tracer.*` when a canonical module
   surface already exists for the needed project type or service.
2. Do not treat every direct project-header include as debt:
   - keep self-owned implementation headers when the `.cpp` / `.module.cpp`
     file is defining that header's declarations
   - keep explicit stable boundaries in `C ABI`, `JNI`, host bridge,
     `android_runtime`, and `tracer_transport` public-header paths
3. When replacing a direct project-header include with `import tracer.*`,
   re-add the real standard-library includes that were previously arriving by
   transitive header inclusion.
4. Do not redesign wrapper namespaces just to avoid a local `using` statement;
   prefer a narrow consumer-side `import + local using` pattern unless the
   wrapper is genuinely unusable.

## Physical Layout

1. `src/shared`
2. `src/domain`
3. `src/application`
4. `src/infrastructure`
5. `cmake/` and `cmake/sources/`
6. `tests/`

## Hotspot Families

| Family | Start Here | Also Check |
| --- | --- | --- |
| Shared helpers and common types | `src/shared` | `tests/shared/tests/*` |
| Domain rules, models, and repositories | `src/domain` | `tests/domain/tests/*` |
| Use case facade and DTO-owned boundaries | `src/application/use_cases`, `src/application/dto`, `src/application/modules` | `tests/application/tests/application_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/api/c_api` |
| Workflow orchestration | `src/application/workflow`, `src/application/pipeline`, `src/application/modules` | `tests/application/tests/application_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/host` |
| Reporting tree | `src/application/reporting/tree`, `src/application/modules` | `tests/application/tests/application_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/tests/platform/infrastructure/tests/data_query` |
| Config | `src/infrastructure/config`, `src/infrastructure/modules` | `../../../apps/tracer_core_shell/api/c_api`, `../../../apps/tracer_core_shell/host` |
| Persistence and schema | `src/infrastructure/persistence`, `src/infrastructure/schema` | `tests/infrastructure/tests/infrastructure_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime` |
| Query and stats | `src/infrastructure/query` | `tests/infrastructure/tests/infrastructure_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/tests/platform/infrastructure/tests/data_query` |
| Reports and export | `src/infrastructure/reports` | `tests/infrastructure/tests/infrastructure_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime` |
| Logging | `src/infrastructure/logging` | `tests/infrastructure/tests/infrastructure_modules_smoke_tests.cpp`, `../../../apps/tracer_core_shell/tests/platform/infrastructure/tests/validation_issue_reporter_tests.cpp` |
| Crypto | `src/infrastructure/crypto` | `../../core/contracts/crypto/` |
| Platform support | `src/infrastructure/platform` | `../../../apps/tracer_core_shell/host` |

## Change Routing

1. Change core use case entrypoints or DTO-owned business flow:
   - start in `src/application/use_cases`
   - pair with `src/application/dto`
   - if the change is shell-visible, also inspect
     `../../../apps/tracer_core_shell/api/c_api`
2. Change workflow import/convert/validate orchestration:
   - start in `src/application/workflow`
   - then inspect `src/application/pipeline`
3. Change reporting tree or query/report output semantics:
   - start in `src/application/reporting/tree`,
     `src/infrastructure/query`, or `src/infrastructure/reports`
   - read `../../core/contracts/stats/README.md` first if
     external output meaning changes
4. Change config ownership or shell config bridges:
   - start in `src/infrastructure/config`
   - then inspect:
     - `../../../apps/tracer_core_shell/api/c_api`
     - `../../../apps/tracer_core_shell/host`
5. Change persistence or SQLite behavior:
   - start in `src/infrastructure/persistence`
   - pair with `src/infrastructure/schema` if the storage contract changes
6. Change module ownership, retained declaration boundaries, or module-first regression coverage:
   - inspect `src/*/modules`
   - inspect explicit boundary declaration headers under `src/**`
   - review `cmake/sources/*.cmake`, module smoke tests, and shell/runtime contract regressions

## Tests / Validate Entry Points

1. Layer smoke coverage lives under `tests/` module smoke targets.
2. Long-lived boundary regressions live in:
   - `../../../apps/tracer_core_shell/tests/platform`
   - `tc_c_api_smoke_tests`
   - `tc_c_api_stability_tests`
3. Focused repo validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core apps/tracer_core_shell
```

4. Shell/runtime integration after core-boundary changes:

```powershell
python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise
```

5. When adding or moving sources, review:
   - `cmake/*.cmake`
   - `cmake/sources/*.cmake`

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [Core Docs](../../core/README.md)
3. [Core Agent Onboarding](../../core/specs/AGENT_ONBOARDING.md)
4. [C ABI Contract](../../core/contracts/c_abi.md)
5. [Stats Contracts](../../core/contracts/stats/README.md)
6. [Core JSON Boundary Design](../../core/architecture/core_json_boundary_design.md)
