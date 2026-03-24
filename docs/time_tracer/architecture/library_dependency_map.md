# Library Dependency Map

## Purpose

Provide first-pass routing across the four active architecture library docs.

## When To Open

- Open this when you do not yet know which library owns the change.
- Open this before any detailed library doc when a task crosses library boundaries.

## What This Doc Does Not Cover

- In-library hotspot routing
- Full file inventories
- Historical refactor context

## Ownership Summary

| Library | Owns | Must Not Own |
| --- | --- | --- |
| `tracer_core` | Core business rules, application/domain boundaries, internal infrastructure families | Runtime JSON contract ownership, C ABI/JNI bridge glue, adapter-only filesystem behavior |
| `tracer_core_bridge_common` | Shared bridge-helper logic for remaining JNI/C API paths | Business rules, runtime orchestration, transport contract ownership |
| `tracer_transport` | Transport implementation: envelopes, field readers, runtime DTO helpers, codec logic | Core business rules, database behavior, host lifecycle logic |
| `tracer_adapters_io` | IO adapter implementation: ingest input collection, file IO, processed-data persistence helpers | Core-owned ports, C ABI/JNI behavior, runtime contract ownership |

## Dependency Direction

1. `tracer_core` is the business-logic owner.
2. `tracer_core_bridge_common` may depend on `tracer_core` types and `tracer_transport` envelope helpers.
3. `tracer_adapters_io` may depend on `tracer_core` ports/helpers and `tracer_transport` where runtime wire helpers are needed.
4. `tracer_transport` is transport-only and does not depend on `tracer_core`.

## First Routing

### Change core use cases, workflow, config ownership, or shell-facing business behavior

Open:

1. [tracer_core](libraries/tracer_core.md)
2. [`../core/architecture/tracer_core_capability_dependency_map.md`](../core/architecture/tracer_core_capability_dependency_map.md)
3. [`../core/design/tracer_core_capability_boundary_contract.md`](../core/design/tracer_core_capability_boundary_contract.md)

Read first if the change is shell-visible:

1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)

### Change runtime JSON codec or envelope behavior

Open:

1. [tracer_transport](libraries/tracer_transport.md)

Read first:

1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)
2. [`../presentation/android/runtime-protocol.md`](../presentation/android/runtime-protocol.md)

### Change shared JNI / C API bridge helpers

Open:

1. [tracer_core_bridge_common](libraries/tracer_core_bridge_common.md)

Read first:

1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)
2. [`../presentation/android/runtime-protocol.md`](../presentation/android/runtime-protocol.md)

### Change file-system ingest or processed-data IO

Open:

1. [tracer_adapters_io](libraries/tracer_adapters_io.md)
2. [tracer_core](libraries/tracer_core.md)

## Validation Shortcut

1. Preferred focused entry:
   - `python tools/run.py validate --plan <plan_name> --paths <touched paths>`
2. If the change touches shell-facing integration:
   - `python tools/run.py verify --app tracer_core_shell --profile fast --concise`
