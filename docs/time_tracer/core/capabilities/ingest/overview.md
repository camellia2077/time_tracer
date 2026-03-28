# Ingest Overview

Ingest owns conversion from validated source text into persisted runtime data.

## Responsibilities
1. collect input sources
2. coordinate parse/validate workflow through the pipeline
3. enter the persistence gate only after validation succeeds
4. write or replace SQLite data transactionally

## Read With
1. [persistence_boundary.md](persistence_boundary.md)
2. [../../ingest/ingest_data_structures.md](../../ingest/ingest_data_structures.md)
3. [../../ingest/ingest_conversion_algorithms.md](../../ingest/ingest_conversion_algorithms.md)
4. [../../ingest/day_bucket_and_wake_anchor_semantics.md](../../ingest/day_bucket_and_wake_anchor_semantics.md)
