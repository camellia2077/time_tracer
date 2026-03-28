# Persistence Runtime Side

## Scope

`persistence_runtime` owns stable DB runtime support and shared sqlite support
used by read-side capabilities.

## Main Owner Paths
1. `libs/tracer_core/src/infra/persistence/repositories/**`
2. `libs/tracer_core/src/infra/persistence/sqlite_database_health_checker.*`
3. `libs/tracer_core/src/infra/persistence/sqlite/db_manager.*`

## Allowed Direct Dependencies
1. shared/domain/application ports
2. sqlite support

## Forbidden Direct Dependencies
1. pipeline/query/reporting/exchange orchestration

## Important Boundary

`persistence_runtime` is stable persistence support. Query-owned read
orchestration and request entry points should stay in query owner paths rather
than drifting back into persistence.

## Read Next
1. [../../architecture/infrastructure_persistence.md](../../architecture/infrastructure_persistence.md)
2. [../../overview/module_boundaries.md](../../overview/module_boundaries.md)
