# Query Overview

## Purpose

Query owns read-side data access and query-oriented output assembly in
`tracer_core`.

It is the capability that answers runtime read requests such as:
1. tree queries
2. list/stat queries
3. semantic-json and text query payloads

## Responsibility Boundary

Query owns:
1. query request handling and orchestration
2. date-range / action orchestration for data-query flows
3. query-side stats calculation
4. query renderers for query output modes
5. query-owned repository/runtime service entry points

Query does not own:
1. report formatter/export flows
2. write-side persistence/import
3. exchange packaging/import
4. config loading as a standalone capability

## Main Owner Paths
1. `libs/tracer_core/src/application/query/tree/**`
2. `libs/tracer_core/src/application/use_cases/query_api.*`
3. `libs/tracer_core/src/infra/query/**`
4. `libs/tracer_core/src/infra/query/data/repository/query_runtime_service*`

## Allowed Direct Dependencies
1. `config`
2. `persistence_runtime`

## Forbidden Direct Dependencies
1. `reporting`

## Read Next
1. [data_query.md](data_query.md)
2. [../../overview/capability_map.md](../../overview/capability_map.md)
3. [../../contracts/stats/README.md](../../contracts/stats/README.md)
