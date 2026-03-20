# Library Dependency Map

This document provides one-hop routing for the four libraries that most often
show up in core runtime, bridge, transport, and adapter work:

1. `libs/tracer_core`
2. `libs/tracer_core_bridge_common`
3. `libs/tracer_transport`
4. `libs/tracer_adapters_io`

## Ownership Summary

| Library | Owns | Depends On | Must Not Own |
| --- | --- | --- | --- |
| `tracer_core` | Core business rules, domain/application DTOs, module-first boundaries, internal infrastructure families | Standard library and infrastructure dependencies used by core internals | Runtime JSON contracts, C ABI/JNI bridge glue, file-system adapter behavior outside core-owned ports |
| `tracer_core_bridge_common` | Shared parsing and bridge helpers for C API / JNI code | `tracer_core` headers and `tracer_transport` envelope helpers | Business rules, runtime orchestration, transport contract ownership |
| `tracer_transport` | Runtime envelope helpers, field readers, runtime request/response DTOs, runtime codec implementation | Standard library and `nlohmann_json` | Core business rules, database access, host lifecycle logic |
| `tracer_adapters_io` | File-system ingest helpers, processed-data persistence helpers, canonical module surfaces for IO adapters | `tracer_core`, `tracer_transport`, filesystem/JSON implementation deps | Core domain/application ownership, C ABI/JNI behavior, runtime contract ownership |

## Dependency Direction
1. `tracer_core` is the business-logic owner.
2. `tracer_core_bridge_common` depends on:
   - `tracer_core` for enums/DTO/types
   - `tracer_transport` for envelope helpers
3. `tracer_adapters_io` depends on:
   - `tracer_core` for ports and shared serialization helpers
   - `tracer_transport` where runtime/processed-data helpers need shared wire utilities
4. `tracer_transport` is transport-only and does not depend on `tracer_core`.

## Change Routing
### Change core use cases, workflow, or pipeline behavior
Start here:
1. [tracer_core](libraries/tracer_core.md)
2. [`../core/README.md`](../core/README.md)

Read first if the change crosses a boundary:
1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)
2. [`../core/contracts/stats/README.md`](../core/contracts/stats/README.md)

### Change core config ownership or shell config bridging
Start here:
1. [tracer_core](libraries/tracer_core.md)
2. [`../core/specs/AGENT_ONBOARDING.md`](../core/specs/AGENT_ONBOARDING.md)

Then check:
1. `apps/tracer_core_shell/api/c_api`
2. `apps/tracer_core_shell/host`

Read first:
1. [`../core/architecture/core_json_boundary_design.md`](../core/architecture/core_json_boundary_design.md)
2. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)

### Change runtime JSON codec behavior
Start here:
1. [tracer_transport](libraries/tracer_transport.md)

Read first:
1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)
2. [`../clients/android_ui/runtime-protocol.md`](../clients/android_ui/runtime-protocol.md)

### Change JNI / C API shared bridge helpers
Start here:
1. [tracer_core_bridge_common](libraries/tracer_core_bridge_common.md)
2. [`../core/specs/AGENT_ONBOARDING.md`](../core/specs/AGENT_ONBOARDING.md)

Read first:
1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)
2. [`../clients/android_ui/runtime-protocol.md`](../clients/android_ui/runtime-protocol.md)

### Change file-system ingest or processed-data IO
Start here:
1. [tracer_adapters_io](libraries/tracer_adapters_io.md)
2. [tracer_core](libraries/tracer_core.md)

Read first:
1. [`../core/architecture/core_json_boundary_design.md`](../core/architecture/core_json_boundary_design.md)

## Validation Shortcuts
1. Preferred focused entry:
   - `python tools/run.py validate --plan <plan_name> --paths <touched paths>`
2. If the change touches shell-facing integration:
   - `python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise`
3. If the change touches transport or module surfaces:
   - check the module smoke tests and boundary regressions listed in the relevant detailed library doc
     under `docs/time_tracer/architecture/libraries/`

## Boundary Rules Worth Remembering
1. `tracer_core` owns business rules and module declaration ownership.
2. `tracer_transport` owns transport implementation, not business semantics.
3. `tracer_core_bridge_common` exists to reduce duplicated bridge glue, not to become a second contract layer.
4. `tracer_adapters_io` is an adapter library; it should follow core-owned ports rather than define them.
5. For `tracer_core` non-boundary consumers, prefer canonical `import tracer.*`
   surfaces over direct project-header includes, but do not treat
   self-owned implementation headers or explicit stable boundaries as guardrail
   violations.
