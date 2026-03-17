# Library Detail Docs

Use this directory for detailed library-level guidance.
These docs hold the long-form ownership, hotspot-family routing, validation,
and boundary notes that are too large to keep in `libs/*/README.md`.

## When To Use These Docs
1. You already know which library you need to change.
2. You need hotspot-family routing inside one library.
3. You need the detailed validation and boundary notes for that library.

## Use `library_dependency_map.md` Instead When
1. You do not yet know which library owns the change.
2. The change crosses multiple libraries.
3. You need cross-library dependency direction before editing code.

## Libraries
1. [tracer_core](tracer_core.md)
2. [tracer_core_bridge_common](tracer_core_bridge_common.md)
3. [tracer_transport](tracer_transport.md)
4. [tracer_adapters_io](tracer_adapters_io.md)
