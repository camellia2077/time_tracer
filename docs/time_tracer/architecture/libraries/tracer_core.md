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
2. It owns domain/application boundaries, capability-owned DTO families, and internal infrastructure families.
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
3. `ITracerCoreRuntime` as the only aggregate runtime surface for shell-facing consumers
   and as a composition bridge over prebuilt capability APIs rather than a
   capability-specific wiring entrypoint
4. Capability APIs, capability-owned `application/ports/<capability>/**` subtrees,
   and capability-owned DTO headers under `src/application/dto/{pipeline,query,reporting,exchange}_*.hpp`
   consumed by shell-facing code and tests
5. Explicit non-owner families under `src/application/dto/compat/**`,
   `src/application/compat/reporting/**`, `src/application/aggregate_runtime/**`,
   and `src/application/runtime_bridge/**`
6. Retained workflow declaration boundary under `src/application/interfaces/i_workflow_handler.hpp`
7. Legacy forwarding shim paths under `src/application/dto/core_*`,
   `src/application/interfaces/i_report_*`, and
   `src/application/use_cases/tracer_core_runtime*` are retired

## Reviewer Shortcut

1. Classify owner by capability-owned port subtree first:
   `src/application/ports/pipeline|query|reporting|exchange/**`
2. If the touched path is a capability-owned DTO header, route by DTO family:
   `src/application/dto/pipeline_*`, `query_*`, `reporting_*`, `exchange_*`
3. If the touched path is `src/application/compat/reporting/**`, treat it as
   retained `reporting` compatibility surface, not as canonical owner path
4. If the touched path is `src/application/interfaces/i_workflow_handler.hpp`,
   treat it as retained `pipeline` declaration boundary
5. Treat canonical module locations as the authority surface for reviewers:
   `src/application/modules/tracer.core.application.*` for application-owned capability modules and
   `src/infra/modules/<capability>/**` for infrastructure-owned capability modules
6. If the touched path is a non-owner family, route by family before capability:
   `src/application/dto/compat/**`, `src/application/aggregate_runtime/**`,
   `src/application/dto/shared_envelopes.hpp`, and `src/application/runtime_bridge/**`
7. Treat `src/application/runtime_bridge/**` as shell/runtime bridge surface,
   not as a capability-owned `application/ports/<capability>` subtree

## Change Routing

1. Change core use cases, workflow, or pipeline behavior:
   - read `docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md`
     and `docs/time_tracer/core/design/tracer_core_capability_boundary_contract.md`
   - start in `src/application/use_cases`, `src/application/workflow`, or `src/application/pipeline`
   - if the change is shell-visible, also inspect `apps/tracer_core_shell/api/c_api`
2. Change reporting/query semantics:
   - read `docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md`
     first to confirm whether the work belongs to `query` or `reporting`
   - start in `src/application/query/tree`, `src/application/reporting`, `src/infra/query`,
     or `src/infra/reporting`
   - read stats contract docs first if external meaning changes
3. Change config ownership or shell config bridges:
   - start in `src/infra/config`
   - then inspect `apps/tracer_core_shell/api/c_api` and `apps/tracer_core_shell/host`
4. Change persistence or schema behavior:
   - start in `src/infra/persistence` and `src/infra/schema`
5. Change module ownership or retained declaration boundaries:
   - inspect `src/*/modules`, explicit boundary headers, and `cmake/sources/*.cmake`
6. Change shell/runtime bridge, aggregate runtime, or compatibility envelope:
   - start in `src/application/runtime_bridge`, `src/application/aggregate_runtime`,
     `src/application/dto/shared_envelopes.hpp`, `src/application/dto/compat`, or
     `src/application/compat/reporting`
   - then inspect `apps/tracer_core_shell/host` and `apps/tracer_core_shell/api/c_api`

## Tests / Validate Entry Points

1. Layer smoke coverage lives under `tests/`.
2. Long-lived shell/runtime regressions live under `apps/tracer_core_shell/tests/platform`.
3. Focused validation:

```powershell
python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/query.toml
python tools/run.py verify --app tracer_core_shell --profile cap_query --concise
```

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [Core Docs](../../core/README.md)
3. [Core Agent Onboarding](../../core/specs/AGENT_ONBOARDING.md)
4. [tracer_core Capability Dependency Map](../../core/architecture/tracer_core_capability_dependency_map.md)
5. [tracer_core Capability Boundary Contract](../../core/design/tracer_core_capability_boundary_contract.md)
6. [C ABI Contract](../../core/contracts/c_abi.md)
7. [Stats Contracts](../../core/contracts/stats/README.md)
