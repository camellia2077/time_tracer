# tracer_core Capability Boundary Contract

## Purpose

Define the engineering contract for `tracer_core` internal capabilities. This
document is authoritative for owner paths, allowed dependencies, forbidden
cross-capability imports, and required focused validation.

This is an architecture/design contract, not an external runtime contract.
`core/contracts/` remains the source of truth for ABI / JSON / stats surfaces.

## Global Rules

1. New code enters the owner paths of an existing capability before it creates
   a new directory family.
2. Cross-capability reuse must flow through ports, DTOs, or composition
   surfaces; it must not reach into another capability's internal directory by
   convenience import.
3. `application/aggregate_runtime/**`, `tc_core_iface`, and `tc_infra_full_lib`
   are aggregation layers, not capability owners.
4. `ITracerCoreRuntime` is the only aggregate application runtime surface.
   New consumers must enter capability APIs through
   `runtime.pipeline()/query()/report()/tracer_exchange()`. Capability-specific
   wiring must stay outside `TracerCoreRuntime`; the runtime surface aggregates
   prebuilt capability APIs instead of constructing them itself.
5. `src/application/runtime_bridge/**` is a shell/runtime bridge family, not a
   capability-owned port subtree.
6. Compatibility aliases may remain, but they are not authority surfaces for
   new boundary documentation or new refactors.
7. `domain` and `application` remain free of adapter-library ownership and
   `nlohmann::json` public surfaces.
8. Focused validate scopes should cover owner implementation paths together
   with the canonical capability-owned port subtrees and canonical module
   trees that publish the same capability boundary.

## pipeline

- Owner paths:
  - `src/application/pipeline/**`
  - `src/application/parser/**`
  - `src/application/workflow*`
  - `src/application/use_cases/pipeline_api.*`
- Public surfaces:
  - `src/application/dto/pipeline_requests.hpp`
  - `src/application/ports/pipeline/**`
  - retained workflow declaration boundary under
    `src/application/interfaces/i_workflow_handler.hpp`
  - canonical application module files under
    `src/application/modules/tracer.core.application.pipeline*.cppm`
  - canonical application module files under
    `src/application/modules/tracer.core.application.workflow*.cppm`
  - retained pipeline bootstrap module files
    `src/application/modules/tracer.core.application.importer.service.cppm`
    and
    `src/application/modules/tracer.core.application.service.converter.cppm`
- Allowed direct deps:
  - shared/domain types
  - config through providers / DTO-backed options
  - persistence write through application ports
- Forbidden direct deps:
  - `application/query/**`
  - `application/reporting/**`
  - `infra/query/**`
  - `infra/reporting/**`
  - `infra/exchange/**`
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/pipeline.toml`
- Main regression surfaces:
  - `tt_pipeline_api_tests`

## query

- Owner paths:
  - `src/application/query/tree/**`
  - `src/application/use_cases/query_api.*`
  - `src/infra/query/**`
  - `src/infra/query/data/repository/query_runtime_service*`
- Public surfaces:
  - canonical DTO headers
    `src/application/dto/query_requests.hpp` and
    `src/application/dto/query_responses.hpp`
  - `src/application/ports/query/**`
  - canonical application module files under
    `src/application/modules/tracer.core.application.query.tree*.cppm`
  - canonical infrastructure module files under `src/infra/modules/query/**`
    (`tracer.core.infrastructure.query.data.*`)
- Allowed direct deps:
  - shared/domain types
  - config
  - persistence runtime
  - query-owned tree projection helper under `src/infra/query/data/internal/**`
  - query-owned text render helper under `src/infra/query/data/renderers/detail/**`
- Forbidden direct deps:
  - `application/reporting/**`
  - `infra/reporting/**`
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/query.toml`
- Main regression surfaces:
  - `tt_query_api_tests`
  - `tc_query_infra_smoke_tests`

## reporting

- Owner paths:
  - `src/application/reporting/**`
  - `src/application/use_cases/report_api*`
  - `src/infra/reporting/**`
- Public surfaces:
  - canonical DTO headers
    `src/application/dto/reporting_requests.hpp` and
    `src/application/dto/reporting_responses.hpp`
  - `src/application/ports/reporting/**`
  - canonical infrastructure module files under
    `src/infra/modules/reporting/**`
    (`tracer.core.infrastructure.reporting.*`)
- Allowed direct deps:
  - shared/domain types
  - config
  - persistence runtime
- Forbidden direct deps:
  - `application/query/**`
  - `infra/query/**`
- Required interpretation:
  - `reporting` stays an independent capability; it does not re-enter `query`
    as its primary implementation path.
  - `reporting` no longer owns query-side tree projection or query text
    duration formatting helpers.
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/reporting.toml`
- Main regression surfaces:
  - `tt_reporting_api_tests`
  - `tc_reporting_infra_smoke_tests`
  - `tt_fmt_parity_tests`

## exchange

- Owner paths:
  - `src/application/use_cases/tracer_exchange_api.*`
  - `src/infra/exchange/**`
  - `src/infra/crypto/**`
- Public surfaces:
  - canonical DTO headers
    `src/application/dto/exchange_requests.hpp` and
    `src/application/dto/exchange_responses.hpp`
  - `src/application/ports/exchange/**`
  - canonical infrastructure module files under
    `src/infra/modules/exchange/**`
    (`tracer.core.infrastructure.exchange`)
- Allowed direct deps:
  - shared/domain types
  - config
  - file-crypto support under `src/infra/crypto/**`
- Forbidden direct deps:
  - `application/query/**`
  - `application/reporting/**`
  - `infra/query/**`
  - `infra/reporting/**`
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/exchange.toml`
- Main regression surfaces:
  - `tt_exchange_api_tests`
  - `tc_exchange_infra_smoke_tests`
  - `tt_file_crypto_tests`

## config

- Owner paths:
  - `src/infra/config/**`
- Public surfaces:
  - canonical infrastructure module files under `src/infra/modules/config/**`
    (`tracer.core.infrastructure.config*`)
- Allowed direct deps:
  - shared/domain/application DTOs and option types
  - internal parse helpers owned by config
- Forbidden direct deps:
  - pipeline/query/reporting/exchange orchestration
  - capability-specific business flows
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/config.toml`
- Main regression surfaces:
  - `tc_config_infra_smoke_tests`
  - escalate to full `verify` when the change crosses shell/runtime config bridges

## persistence_write

- Owner paths:
  - `src/infra/persistence/importer/**`
  - `src/infra/persistence/sqlite_time_sheet_repository.*`
- Public surfaces:
  - `src/application/ports/pipeline/i_processed_data_storage.hpp`
  - `src/application/ports/pipeline/i_time_sheet_repository.hpp`
  - canonical infrastructure module files under
    `src/infra/modules/persistence/write/**`
    (`tracer.core.infrastructure.persistence.write*`)
- Allowed direct deps:
  - shared/domain/application ports
  - shared sqlite support reused from persistence runtime
- Forbidden direct deps:
  - `infra/query/**`
  - `infra/reporting/**`
  - `infra/exchange/**`
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/persistence_write.toml`
- Main regression surfaces:
  - `tc_persistence_write_infra_smoke_tests`

## persistence_runtime

- Owner paths:
  - `src/infra/persistence/repositories/**`
  - `src/infra/persistence/sqlite_database_health_checker.*`
  - `src/infra/persistence/sqlite/db_manager.*`
- Public surfaces:
  - `src/application/ports/pipeline/i_database_health_checker.hpp`
  - canonical infrastructure module files under
    `src/infra/modules/persistence/runtime/**`
    (`tracer.core.infrastructure.persistence.runtime*`)
- Allowed direct deps:
  - shared/domain/application ports
  - sqlite support
- Forbidden direct deps:
  - pipeline/query/reporting/exchange orchestration
- Required interpretation:
  - `persistence.runtime` owns stable DB runtime support only.
  - query-owned read orchestration and adapter entry no longer live here.
- Validate entry:
  - `python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/persistence_runtime.toml`
- Main regression surfaces:
  - `tc_persistence_runtime_infra_smoke_tests`

## Aggregate DTO Interpretation

`src/application/dto/compat/**` is the retained compatibility DTO family for
shell-facing runtime/C ABI consumers. Authoritative DTO ownership is declared
by capability-owned headers first:

- pipeline DTOs:
  `src/application/dto/pipeline_requests.hpp`
- query DTOs:
  `src/application/dto/query_requests.hpp` and
  `src/application/dto/query_responses.hpp`
- reporting DTOs:
  `src/application/dto/reporting_requests.hpp` and
  `src/application/dto/reporting_responses.hpp`
- exchange DTOs:
  `src/application/dto/exchange_requests.hpp` and
  `src/application/dto/exchange_responses.hpp`
- retained shared aggregate envelopes:
  `src/application/dto/shared_envelopes.hpp`
  (`OperationAck`, `TextOutput`)

New DTOs should enter a capability-owned header first unless they are truly
shared aggregate envelopes used across multiple capability APIs or shell-facing
runtime bridges.

## Explicit Non-Owner Families

- compatibility DTO family:
  `src/application/dto/compat/**`
- retained reporting declaration boundaries:
  `src/application/compat/reporting/**`
- aggregate runtime bridge:
  `src/application/aggregate_runtime/**`
- shell/runtime bridge:
  `src/application/runtime_bridge/**`

Legacy forwarding shims under `src/application/dto/core_*`,
`src/application/interfaces/i_report_*`, and
`src/application/use_cases/tracer_core_runtime*` are retired; they are not
canonical owner or reviewer-routing surfaces.

## Non-Owner Paths Requiring Escalation

Changes under these paths are not capability-local and should use explicit
multi-capability paths or escalate to full `verify`:

- `src/infra/internal/**`
- `src/application/aggregate_runtime/**`
- `src/application/dto/compat/**`
- `src/application/compat/reporting/**`
- `src/application/dto/shared_envelopes.hpp`
- `src/application/runtime_bridge/**`
- any touched port subtree that is consumed by more than one capability

## Done Criteria For Phase 1

Phase 1 boundary hardening is considered complete when all of the following
stay true together:

1. Capability docs, routing docs, and validate commands use the same names.
2. `CoreTargets.cmake` rejects the known forbidden edges:
   - `pipeline -> query/reporting/exchange`
   - `query -> reporting`
   - `reporting -> query`
3. Each first-class capability has a checked-in `paths-file`.
4. Reviewers can classify a touched file into a capability without reading the
   full implementation.
