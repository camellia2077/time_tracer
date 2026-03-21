# Library Detail Docs

## Purpose

Navigation hub for detailed library-level guidance under
`docs/time_tracer/architecture/libraries/`.

## When To Open

- Open this after `library_dependency_map.md` once you know which library owns the change.
- Use it to jump into one library's stable boundaries and first routing hints.

## What This Doc Does Not Cover

- Cross-library ownership decisions
- Full implementation walkthroughs
- Historical refactor notes

## Read Order

1. Start with `../library_dependency_map.md` if ownership is still unclear.
2. Open one detailed library doc only after you know the owning library.
3. Use local `libs/*/README.md` only as a thin workspace entrypoint.

## Libraries

1. [tracer_core](tracer_core.md)
   - core business-logic owner
2. [tracer_core_bridge_common](tracer_core_bridge_common.md)
   - shared bridge-helper owner
3. [tracer_transport](tracer_transport.md)
   - transport implementation owner
4. [tracer_adapters_io](tracer_adapters_io.md)
   - IO adapter owner
