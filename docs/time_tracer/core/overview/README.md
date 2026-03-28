# Core Overview

This directory is the capability-first entry layer for `tracer_core` docs.

Use it before diving into legacy `architecture/`, `contracts/`, `design/`, or
`ingest/` folders.

## Read First
1. [../README.md](../README.md)
   - Top-level routing for core docs.
2. [identity_and_boundary.md](identity_and_boundary.md)
   - Core identity, top-level routing, and search-first path families.
3. [capability_map.md](capability_map.md)
   - Capability ownership and validate-first map.
4. [module_boundaries.md](module_boundaries.md)
   - Capability boundary rules, owner paths, and non-owner families.
5. [../architecture/README.md](../architecture/README.md)
   - Legacy architecture index that still owns other long-form subsystem docs.

## Current Role
1. Provide a stable top-level entry for capability-first documentation.
2. Gather global architecture and boundary references before readers branch into
   capability-specific folders.
3. Reduce the need to guess whether a topic is filed under `architecture/`,
   `design/`, or `contracts/`.

## Current Docs
1. [identity_and_boundary.md](identity_and_boundary.md)
2. [capability_map.md](capability_map.md)
3. [module_boundaries.md](module_boundaries.md)

## Migration Note
The old folder split remains valid during migration. New docs should prefer:
1. `shared/` for cross-capability contracts and semantics.
2. `capabilities/<name>/` for capability-owned docs.
