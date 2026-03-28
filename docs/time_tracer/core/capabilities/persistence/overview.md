# Persistence Overview

## Purpose

Persistence is split into two core capability slices:
1. `persistence_write`
2. `persistence_runtime`

The split exists to keep write-side ingest/import storage concerns separate from
runtime-side DB support and read-side infrastructure support.

## Responsibility Boundary

Persistence owns:
1. sqlite writer and write repositories
2. database runtime support such as DB health and shared sqlite management
3. project repository and other stable persistence support services

Persistence does not own:
1. ingest/query/reporting/exchange orchestration
2. host UI/CLI path UX
3. business-level validation semantics

## Read Next
1. [write_side.md](write_side.md)
2. [runtime_side.md](runtime_side.md)
3. [../../architecture/infrastructure_persistence.md](../../architecture/infrastructure_persistence.md)
