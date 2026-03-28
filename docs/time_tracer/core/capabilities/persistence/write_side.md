# Persistence Write Side

## Scope

`persistence_write` owns ingest/import write-side repositories and the SQLite
writer chain.

## Main Owner Paths
1. `libs/tracer_core/src/infra/persistence/importer/**`
2. `libs/tracer_core/src/infra/persistence/sqlite_time_sheet_repository.*`

## Allowed Direct Dependencies
1. shared/domain/application ports
2. shared sqlite support reused from `persistence_runtime`

## Forbidden Direct Dependencies
1. `infra/query/**`
2. `infra/reporting/**`
3. `infra/exchange/**`

## Read Next
1. [../../capabilities/ingest/persistence_boundary.md](../../capabilities/ingest/persistence_boundary.md)
2. [../../architecture/infrastructure_persistence.md](../../architecture/infrastructure_persistence.md)
