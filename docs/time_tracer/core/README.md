# Core Docs

This directory holds the developer-facing docs for the core engine:
domain rules, application orchestration, contracts, architecture constraints,
and core-specific onboarding.

The structure is migrating from doc-type folders toward capability-first
routing. During the transition, both the new and legacy folders remain valid.

## Start Here
1. [Library Dependency Map](../architecture/library_dependency_map.md)
   - Use this first when you need to decide which library owns the change.
2. [Core Agent Onboarding](specs/AGENT_ONBOARDING.md)
   - Fastest route for common core and shell-facing changes.
3. [overview/README.md](overview/README.md)
   - Capability-first top-level routing for core docs.
4. [shared/README.md](shared/README.md)
   - Shared cross-capability contracts and semantics.
5. [errors/README.md](errors/README.md)
   - Core-wide error model, logging semantics, and machine-readable error codes.
6. [capabilities/validation/README.md](capabilities/validation/README.md)
   - Primary entry for validation docs.
7. [capabilities/ingest/README.md](capabilities/ingest/README.md)
   - Primary entry for ingest docs.
8. [capabilities/query/README.md](capabilities/query/README.md)
   - Primary entry for query docs.
9. [capabilities/reporting/README.md](capabilities/reporting/README.md)
   - Primary entry for reporting docs.
10. [capabilities/exchange/README.md](capabilities/exchange/README.md)
   - Primary entry for exchange docs.
11. [capabilities/config/README.md](capabilities/config/README.md)
   - Primary entry for config docs.
12. [capabilities/persistence/README.md](capabilities/persistence/README.md)
   - Primary entry for persistence docs.
13. [overview/module_boundaries.md](overview/module_boundaries.md)
   - Engineering contract for owner paths, forbidden edges, and validate entrypoints.

## What Lives Here
1. Domain model, business rules, and application orchestration docs.
2. Core-facing contracts, error semantics, reporting/query contracts, and schema docs.
3. Architecture rules for module boundaries, JSON handling, and dependency direction.
4. Capability-owned docs that describe SOP, responsibility boundaries, and
   source-of-truth rules.
5. Agent/collaborator routing docs for fast entry into the right library or host adapter path.

## What Does Not Live Here
1. Android Compose UI details.
2. Windows CLI interaction docs.
3. Long-form library navigation for non-core libraries.
   - Use [`../architecture/README.md`](../architecture/README.md) and
     [`../architecture/libraries/README.md`](../architecture/libraries/README.md)
     for that.

## Read-First Docs by Change Type
1. Validation flow, canonical text, or validation diagnostics changes:
   - [capabilities/validation/README.md](capabilities/validation/README.md)
   - [shared/canonical_text_contract_v1.md](shared/canonical_text_contract_v1.md)
   - [errors/error-codes.md](errors/error-codes.md)
2. Core-wide boundary, capability ownership, or module routing changes:
   - [overview/identity_and_boundary.md](overview/identity_and_boundary.md)
   - [overview/capability_map.md](overview/capability_map.md)
   - [overview/module_boundaries.md](overview/module_boundaries.md)
3. Ingest workflow or persistence gate changes:
   - [capabilities/ingest/README.md](capabilities/ingest/README.md)
   - [capabilities/ingest/persistence_boundary.md](capabilities/ingest/persistence_boundary.md)
4. Query/data-query output, stats, or semantic-json changes:
   - [capabilities/query/README.md](capabilities/query/README.md)
   - [capabilities/query/data_query.md](capabilities/query/data_query.md)
   - [contracts/stats/README.md](contracts/stats/README.md)
5. Reporting formatter/export/report snapshot changes:
   - [capabilities/reporting/README.md](capabilities/reporting/README.md)
   - [capabilities/reporting/contracts.md](capabilities/reporting/contracts.md)
6. Exchange `.tracer` package or runtime-crypto behavior:
   - [capabilities/exchange/README.md](capabilities/exchange/README.md)
   - [capabilities/exchange/contracts.md](capabilities/exchange/contracts.md)
7. Config ownership or config-validation changes:
   - [capabilities/config/README.md](capabilities/config/README.md)
   - [capabilities/validation/config_validation.md](capabilities/validation/config_validation.md)
8. Persistence write/runtime split or SQLite boundary changes:
   - [capabilities/persistence/README.md](capabilities/persistence/README.md)
   - [capabilities/persistence/write_side.md](capabilities/persistence/write_side.md)
   - [capabilities/persistence/runtime_side.md](capabilities/persistence/runtime_side.md)
9. C ABI or runtime boundary behavior:
   - [shared/c_abi.md](shared/c_abi.md)
   - [../clients/android_ui/runtime-protocol.md](../clients/android_ui/runtime-protocol.md)
10. Core use case, workflow, query tree, or config ownership changes:
   - [specs/AGENT_ONBOARDING.md](specs/AGENT_ONBOARDING.md)
   - [../architecture/libraries/tracer_core.md](../architecture/libraries/tracer_core.md)
   - [overview/capability_map.md](overview/capability_map.md)
   - [overview/module_boundaries.md](overview/module_boundaries.md)
11. JSON boundary changes:
   - [architecture/core_json_boundary_design.md](architecture/core_json_boundary_design.md)
12. Module-boundary refactors:
   - [overview/capability_map.md](overview/capability_map.md)
   - [overview/module_boundaries.md](overview/module_boundaries.md)
   - [architecture/refactor_module_boundaries.md](architecture/refactor_module_boundaries.md)
   - [overview/identity_and_boundary.md](overview/identity_and_boundary.md)

## Canonical Subdirectories
1. `overview/`
   - Capability-first top-level routing and migration-friendly entry docs.
2. `shared/`
   - Shared contracts and semantics used by more than one capability.
3. `errors/`
   - Core-wide error model, machine-readable error codes, and logging semantics.
4. `capabilities/`
   - Capability-owned docs such as validation and ingest.
5. Legacy folders still retained during migration:
   - `contracts/`
   - `architecture/`
   - `design/`
   - `ingest/`
   - `specs/`

## Core Rules
1. New core docs should prefer `shared/`, `errors/`, or `capabilities/<name>/` when the
   ownership is clear.
2. Legacy doc-type folders may remain as thin indexes or transitional homes
   until each capability is migrated.
3. `domain` and `application` must not depend on adapter implementation libraries.
4. Core should expose contracts through ports/DTOs and explicit module or thin-header boundaries.
5. If a change crosses ABI or runtime JSON boundaries, update the contract docs before the implementation docs drift.
