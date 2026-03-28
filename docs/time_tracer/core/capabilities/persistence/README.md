# Persistence Capability

This directory is the capability-first entry for `tracer_core` persistence
docs.

Use it when you need to understand:
1. write-side vs runtime-side persistence boundaries
2. what persistence owns and what it must not absorb from business capabilities
3. where SQLite support ends and capability orchestration begins

## Read First
1. [overview.md](overview.md)
   - Persistence write/runtime split and owner boundaries.
2. [write_side.md](write_side.md)
   - Write-side persistence responsibilities and ingest relationship.
3. [runtime_side.md](runtime_side.md)
   - Runtime-side persistence support and read-side boundaries.

## Legacy Pointers
Persistence topics were previously spread across:
1. `core/architecture/infrastructure_persistence.md`
2. `core/overview/capability_map.md`
3. `core/overview/module_boundaries.md`
4. `core/capabilities/ingest/persistence_boundary.md`
