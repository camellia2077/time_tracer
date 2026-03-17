# Library Architecture Docs

This directory is the library-level navigation entry for `time_tracer`.
Start here when the change crosses library boundaries or when you need to
answer "which library owns this behavior?" before opening code.

## Start Here
1. [Library Dependency Map](library_dependency_map.md)
   - Cross-library ownership, dependency direction, and one-hop change routing.
2. [Library Detail Docs](libraries/README.md)
   - Detailed library-level ownership, hotspot routing, and validation notes.
3. [tracer_core](libraries/tracer_core.md)
   - Core business logic, module boundaries, config/query/report families.
4. [tracer_core_bridge_common](libraries/tracer_core_bridge_common.md)
   - Shared C API / JNI bridge helpers.
5. [tracer_transport](libraries/tracer_transport.md)
   - Runtime envelope, field readers, and request/response codec logic.
6. [tracer_adapters_io](libraries/tracer_adapters_io.md)
   - File-system ingest and processed-data IO adapters.

## Use This Directory When
1. A change touches more than one of the following:
   - core business logic
   - runtime JSON payloads
   - C API / JNI bridge helpers
   - file-system or processed-data adapters
2. You need to find the right validation entrypoint before editing code.
3. You need to know which contract doc must be updated first.

## Read-First Boundary Docs
1. [`../core/contracts/c_abi.md`](../core/contracts/c_abi.md)
   - Read before changing C ABI payloads or runtime request/response shape.
2. [`../clients/android_ui/runtime-protocol.md`](../clients/android_ui/runtime-protocol.md)
   - Read before changing JNI/runtime bridge payload behavior.
3. [`../core/contracts/stats/README.md`](../core/contracts/stats/README.md)
   - Read before changing query/report output semantics.
4. [`../core/architecture/core_json_boundary_design.md`](../core/architecture/core_json_boundary_design.md)
   - Read before moving JSON handling across core boundaries.

## Out of Scope
This directory is not the place for long-form design history, platform UI
details, or deep algorithm walkthroughs. Those remain in the existing
`core/`, `clients/`, `guides/`, and `design/` docs trees.
