# Ingest Capability

This directory is the capability-first entry for ingest docs.

## Current Read Order
1. [overview.md](overview.md)
2. [persistence_boundary.md](persistence_boundary.md)
3. [../../ingest/README.md](../../ingest/README.md)

## Key Business Contract

- [Interval event and mixed timeline semantics](../../ingest/interval_event_and_mixed_timeline_semantics.md)
  - Includes fixed start-time attribution dates and cross-midnight interval rules.

## Migration Note
The legacy `core/ingest/` folder still holds most long-form ingest topics during
the transition. New capability-owned ingest docs should prefer this directory.
