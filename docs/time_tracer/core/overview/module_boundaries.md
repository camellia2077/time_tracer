# tracer_core Module Boundaries

## Purpose

This document defines the engineering contract for `tracer_core` internal
capabilities. It is authoritative for:
1. owner paths
2. allowed dependencies
3. forbidden cross-capability imports
4. focused validation routing

This is an internal architecture/design contract, not an external ABI contract.

## Global Rules
1. New code should enter the owner paths of an existing capability before
   creating a new directory family.
2. Cross-capability reuse must flow through ports, DTOs, or composition
   surfaces, not by directly reaching into another capability internals.
3. `application/aggregate_runtime/**`, `tc_core_iface`, and `tc_infra_full_lib`
   are aggregation layers, not capability owners.
4. `ITracerCoreRuntime` is the aggregate application runtime surface.
5. `src/application/runtime_bridge/**` is a shell/runtime bridge family, not a
   capability-owned port subtree.
6. Compatibility aliases may remain, but they are not authority surfaces for
   new boundary documentation or new refactors.
7. `domain` and `application` remain free of adapter-library ownership and
   public `nlohmann::json` surfaces.
8. Focused validate scopes should cover owner implementation paths together
   with canonical port and module trees that publish the same boundary.

## Capability Owner Summary

1. `pipeline`
   - owner paths:
     - `src/application/pipeline/**`
     - `src/application/parser/**`
     - `src/application/workflow*`
     - `src/application/use_cases/pipeline_api.*`
2. `query`
   - owner paths:
     - `src/application/query/tree/**`
     - `src/application/use_cases/query_api.*`
     - `src/infra/query/**`
3. `reporting`
   - owner paths:
     - `src/application/reporting/**`
     - `src/application/use_cases/report_api*`
     - `src/infra/reporting/**`
4. `exchange`
   - owner paths:
     - `src/application/use_cases/tracer_exchange_api.*`
     - `src/infra/exchange/**`
     - `src/infra/crypto/**`
5. `config`
   - owner paths:
     - `src/infra/config/**`
6. `persistence_write`
   - owner paths:
     - `src/infra/persistence/importer/**`
     - `src/infra/persistence/sqlite_time_sheet_repository.*`
7. `persistence_runtime`
   - owner paths:
     - `src/infra/persistence/repositories/**`
     - `src/infra/persistence/sqlite_database_health_checker.*`
     - `src/infra/persistence/sqlite/db_manager.*`

## Explicit Non-Owner Families

These paths are not capability-local ownership surfaces:
1. `src/application/dto/compat/**`
2. `src/application/compat/reporting/**`
3. `src/application/aggregate_runtime/**`
4. `src/application/runtime_bridge/**`
5. shell aggregate/runtime glue under:
   - `apps/tracer_core_shell/api/c_api/runtime/**`
   - `apps/tracer_core_shell/host/bootstrap/**`
   - `apps/tracer_core_shell/host/exchange/**`
   - retained root-level `apps/tracer_core_shell/api/c_api/tracer_core_c_api.cpp`

Changes there should usually use explicit multi-capability validation or
escalate to a broader verify pass.

## Shell Boundary Rules
1. New business logic does not belong in `apps/tracer_core_shell/**`.
2. New shell entrypoints must not reintroduce flat `api/c_api/*.cpp` or
   `host/*.cpp` layouts.
3. New shell files must enter an existing shell family.

## Read Next
1. [capability_map.md](capability_map.md)
2. [identity_and_boundary.md](identity_and_boundary.md)
3. [../shared/c_abi.md](../shared/c_abi.md)
