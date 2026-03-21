# tracer_core

## Purpose

Detailed navigation for the core business-logic library.

## When To Open

- Open this when the task changes core business rules, use cases, workflow, or core-owned infrastructure behavior.
- Open this after `library_dependency_map.md` once `tracer_core` is the confirmed owner.

## What This Doc Does Not Cover

- Full file inventories
- Historical refactor notes
- Exact implementation steps for every change

## Ownership

1. `tracer_core` is the business-logic owner.
2. It owns domain/application boundaries, core DTOs, and internal infrastructure families.
3. It does not own runtime JSON wire semantics or host bridge lifecycle glue.

## Allowed Dependencies

1. `shared`
2. `domain -> shared`
3. `application -> domain + shared`
4. `infrastructure -> shared + domain + application`

## Forbidden Dependencies

1. `domain` and `application` must not depend on:
   - `libs/tracer_transport`
   - `libs/tracer_adapters_io`
   - `libs/tracer_core_bridge_common`
   - `apps/**`
2. `domain` and `application` must not expose `nlohmann::json`.
3. Runtime envelope ownership does not live here.

## Public Surfaces

1. Canonical module surfaces under `src/*/modules`
2. Explicit boundary declaration headers under `src/**`
3. Core DTOs, domain types, ports, and infrastructure boundaries consumed by shell-facing code and tests

## Change Routing

1. Change core use cases, workflow, or pipeline behavior:
   - start in `src/application/use_cases`, `src/application/workflow`, or `src/application/pipeline`
   - if the change is shell-visible, also inspect `apps/tracer_core_shell/api/c_api`
2. Change reporting/query semantics:
   - start in `src/application/reporting/tree`, `src/infrastructure/query`, or `src/infrastructure/reports`
   - read stats contract docs first if external meaning changes
3. Change config ownership or shell config bridges:
   - start in `src/infrastructure/config`
   - then inspect `apps/tracer_core_shell/api/c_api` and `apps/tracer_core_shell/host`
4. Change persistence or schema behavior:
   - start in `src/infrastructure/persistence` and `src/infrastructure/schema`
5. Change module ownership or retained declaration boundaries:
   - inspect `src/*/modules`, explicit boundary headers, and `cmake/sources/*.cmake`

## Tests / Validate Entry Points

1. Layer smoke coverage lives under `tests/`.
2. Long-lived shell/runtime regressions live under `apps/tracer_core_shell/tests/platform`.
3. Focused validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core apps/tracer_core_shell
python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [Core Docs](../../core/README.md)
3. [Core Agent Onboarding](../../core/specs/AGENT_ONBOARDING.md)
4. [C ABI Contract](../../core/contracts/c_abi.md)
5. [Stats Contracts](../../core/contracts/stats/README.md)
