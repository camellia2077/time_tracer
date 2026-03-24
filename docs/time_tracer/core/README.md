# Core Docs

This directory holds the developer-facing docs for the core engine:
domain rules, application orchestration, contracts, architecture constraints,
and core-specific onboarding.

## Start Here
1. [Library Dependency Map](../architecture/library_dependency_map.md)
   - Use this first when you need to decide which library owns the change.
2. [Core Agent Onboarding](specs/AGENT_ONBOARDING.md)
   - Fastest route for common core and shell-facing changes.
3. [Core Architecture Index](architecture/README.md)
   - Architecture constraints and subsystem design docs.
4. [Core Contracts Index](contracts/README.md)
   - External behavior contracts and stable payload definitions.
5. [tracer_core Capability Dependency Map](architecture/tracer_core_capability_dependency_map.md)
   - Authority map for `tracer_core` internal capability ownership and direct deps.
6. [tracer_core Capability Boundary Contract](design/tracer_core_capability_boundary_contract.md)
   - Engineering contract for owner paths, forbidden edges, and validate entrypoints.

## What Lives Here
1. Domain model, business rules, and application orchestration docs.
2. Core-facing contracts, error semantics, reporting/query contracts, and schema docs.
3. Architecture rules for module boundaries, JSON handling, and dependency direction.
4. Agent/collaborator routing docs for fast entry into the right library or host adapter path.

## What Does Not Live Here
1. Android Compose UI details.
2. Windows CLI interaction docs.
3. Long-form library navigation for non-core libraries.
   - Use [`../architecture/README.md`](../architecture/README.md) and
     [`../architecture/libraries/README.md`](../architecture/libraries/README.md)
     for that.

## Read-First Docs by Change Type
1. C ABI or runtime boundary behavior:
   - [contracts/c_abi.md](contracts/c_abi.md)
   - [../clients/android_ui/runtime-protocol.md](../clients/android_ui/runtime-protocol.md)
2. Core use case, workflow, query tree, or config ownership changes:
   - [specs/AGENT_ONBOARDING.md](specs/AGENT_ONBOARDING.md)
   - [../architecture/libraries/tracer_core.md](../architecture/libraries/tracer_core.md)
   - [architecture/tracer_core_capability_dependency_map.md](architecture/tracer_core_capability_dependency_map.md)
   - [design/tracer_core_capability_boundary_contract.md](design/tracer_core_capability_boundary_contract.md)
3. Query/report payload or chart output changes:
   - [contracts/stats/README.md](contracts/stats/README.md)
   - [contracts/reporting/report_data_consistency_spec_v1.md](contracts/reporting/report_data_consistency_spec_v1.md)
4. JSON boundary changes:
   - [architecture/core_json_boundary_design.md](architecture/core_json_boundary_design.md)
5. Module-boundary refactors:
   - [architecture/tracer_core_capability_dependency_map.md](architecture/tracer_core_capability_dependency_map.md)
   - [design/tracer_core_capability_boundary_contract.md](design/tracer_core_capability_boundary_contract.md)
   - [architecture/refactor_module_boundaries.md](architecture/refactor_module_boundaries.md)
   - [architecture/tracer_core_identity_and_boundary.md](architecture/tracer_core_identity_and_boundary.md)

## Canonical Subdirectories
1. `contracts/`
   - Public behavior, ABI, schema, and output semantics.
2. `architecture/`
   - Boundary, dependency, and layering design docs.
3. `ingest/`
   - Import data structures and conversion algorithm docs.
4. `specs/`
   - Fast routing and contributor/agent entry docs.

## Core Rules
1. New core docs should be added under this tree by responsibility.
2. `domain` and `application` must not depend on adapter implementation libraries.
3. Core should expose contracts through ports/DTOs and explicit module or thin-header boundaries.
4. If a change crosses ABI or runtime JSON boundaries, update the contract docs before the implementation docs drift.
