# Query Data-Query Routing

## Scope

This page is the capability-first routing doc for the `data_query` family.

## Main Query/Data-Query Concepts
1. stats calculation belongs to `infra/query/data/stats/`
2. period/range orchestration belongs to `infra/query/data/orchestrators/`
3. text and semantic-json rendering belong to `infra/query/data/renderers/`
4. runtime-service request/dispatch belongs to `infra/query/data/repository/`

## Current Detailed Docs
1. [../../architecture/data_query/README.md](../../architecture/data_query/README.md)
2. [../../architecture/data_query/data_query_responsibility_boundaries_v1.md](../../architecture/data_query/data_query_responsibility_boundaries_v1.md)
3. [../../architecture/data_query/data_query_refactor_completion_v1.md](../../architecture/data_query/data_query_refactor_completion_v1.md)
4. [../../contracts/stats/capability/README.md](../../contracts/stats/capability/README.md)
5. [../../contracts/stats/semantic_json/README.md](../../contracts/stats/semantic_json/README.md)
6. [../../contracts/stats/adapters/README.md](../../contracts/stats/adapters/README.md)

## Code Entry References
1. `libs/tracer_core/src/infra/query/data/stats/`
2. `libs/tracer_core/src/infra/query/data/orchestrators/`
3. `libs/tracer_core/src/infra/query/data/renderers/`
4. `libs/tracer_core/src/infra/query/data/repository/`
