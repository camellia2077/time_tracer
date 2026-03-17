# TimeTracer Docs Index

This file is the top-level docs router. It points to the right contract,
library, client, and workflow documents before you search the repository.

## Start Here
1. [Library Architecture Docs](architecture/README.md)
   - Cross-library ownership and change-routing for `tracer_core`,
     `tracer_core_bridge_common`, `tracer_transport`, and `tracer_adapters_io`.
2. [Core Docs](core/README.md)
   - Core contracts, architecture rules, and onboarding guidance.
3. [Android Client Docs](clients/android_ui/README.md)
   - Android UI/runtime behavior and platform-specific client docs.
4. [Windows CLI Docs](clients/windows_cli/README.md)
   - Windows CLI behavior and command-surface guidance.
5. [Guides](guides/)
   - Implementation guides, data flow details, and supporting technical notes.

## Common Change Routes
1. Change core business logic, use cases, workflow, config, query, or reports:
   - start with [architecture/libraries/tracer_core.md](architecture/libraries/tracer_core.md)
   - then use [Core Docs](core/README.md)
2. Change runtime envelope, field readers, or request/response codecs:
   - start with [architecture/libraries/tracer_transport.md](architecture/libraries/tracer_transport.md)
3. Change C API / JNI shared bridge helpers:
   - start with [architecture/libraries/tracer_core_bridge_common.md](architecture/libraries/tracer_core_bridge_common.md)
4. Change file-system ingest or processed-data IO:
   - start with [architecture/libraries/tracer_adapters_io.md](architecture/libraries/tracer_adapters_io.md)
5. Change shell-facing runtime behavior:
   - start with [Core Agent Onboarding](core/specs/AGENT_ONBOARDING.md)

## Read-First Contract Docs
1. [C ABI Contract](core/contracts/c_abi.md)
2. [Android Runtime Protocol](clients/android_ui/runtime-protocol.md)
3. [Stats Contracts](core/contracts/stats/README.md)
4. [Core JSON Boundary Design](core/architecture/core_json_boundary_design.md)
5. [Reporting Data Consistency Spec](core/contracts/reporting/report_data_consistency_spec_v1.md)

## Other Useful Entry Points
1. [Cross-layer workflow](workflows/workflow.md)
2. [Core data query architecture](core/architecture/data_query/README.md)
3. [Native modules guide](guides/native/native_modules.md)
4. [Project history](history/)
