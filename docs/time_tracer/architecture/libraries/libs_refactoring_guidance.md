# Library Refactoring Guidance

## Purpose

This is the short, cross-library entrypoint for refactoring the reusable
libraries in `time_tracer`:

- `libs/tracer_core`
- `libs/tracer_adapters_io`
- `libs/tracer_core_bridge_common`
- `libs/tracer_transport`

Read this document first, then open the guidance for the library that owns the
behavior. It is a routing document, not a replacement for capability,
protocol, ABI, or business-rule contracts.

## Ownership and Dependency Direction

| Library | Primary owner | Must not become owner of |
|---|---|---|
| `tracer_core` | Domain rules, application capabilities, parsing, validation, query/insights semantics, persistence and exchange implementations | Runtime JSON, filesystem orchestration, C ABI/JNI glue, host lifecycle |
| `tracer_adapters_io` | Filesystem ingest input and processed-data IO | Business validation, TXT semantics, use-case orchestration, runtime protocol |
| `tracer_core_bridge_common` | Shared C API/JNI boundary mechanics | Business rules, filesystem behavior, use-case orchestration |
| `tracer_transport` | Runtime envelopes, fields, DTOs, codecs, wire defaults | Core business rules, database access, filesystem access, host lifecycle |

The normal direction is:

```text
apps / presentation -> core or bridge -> transport
apps / presentation -> adapters -> core
adapters -> transport

transport -/-> core
core -/-> adapters, bridge, transport
```

A composition target may link several libraries without owning their behavior.
Keep composition surfaces thin and keep capability ownership in the library
that defines the behavior.

## Common Rules

1. Identify the behavior owner before moving code. If ownership is unclear,
   introduce a narrow port or DTO rather than another catch-all shared helper.
2. Preserve public headers, module surfaces, DTO meaning, wire formats, C ABI,
   JNI behavior, and diagnostics unless the change explicitly updates the
   relevant contract.
3. Keep core domain/application code independent of transport, adapters,
   bridge code, apps, and public `nlohmann::json` representation.
4. Add characterization, golden, round-trip, or semantic tests before
   extracting behavior that is not already protected.
5. Keep the existing facade usable while extracting internal helpers. A lower
   LOC count is not success if it creates a second model or hides ownership.
6. Do not add broad diagnostics or speculative static analysis during the first
   refactor. Scan, form one concrete hypothesis, add the smallest useful check,
   then scan and validate again.

## How to Use the LOC Scan

The scan is a review queue, not an automated diagnosis. Large files indicate
where to inspect; they do not prove that a split is needed. Within a priority,
line count can order review, but semantic risk and boundary impact take
precedence. Tests remain an independent category and should not be mixed into
production-code priority.

For every candidate, state a concrete hypothesis, for example:

- “This core file mixes query orchestration and row mapping.”
- “This bridge duplicates transport envelope serialization.”
- “This adapter mixes file discovery with processed-data persistence.”

Do not start from “the file is too long.”

## Common Refactoring Workflow

1. Run the library-oriented LOC scan and identify the library and capability.
2. Read this document and the owning library's guidance below.
3. Read public headers/modules, callers, relevant contracts, and focused tests.
4. Add or identify a test that captures the behavior to preserve.
5. Extract behind the existing public/module surface using a narrow seam.
6. Run focused validation; use cross-boundary validation when a public surface
   or dependency edge changes.
7. Re-run the scan and record the architectural reason and remaining risk.

## Read Next

- [tracer_core refactoring guidance](tracer_core.md#refactoring-guidance)
- [tracer_adapters_io refactoring guidance](tracer_adapters_io.md#refactoring-guidance)
- [tracer_core_bridge_common refactoring guidance](tracer_core_bridge_common.md#refactoring-guidance)
- [tracer_transport refactoring guidance](tracer_transport.md#refactoring-guidance)
- [Library dependency map](../library_dependency_map.md)
- [Core architecture](../../core/architecture/README.md)
