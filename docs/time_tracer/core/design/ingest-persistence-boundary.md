# Ingest Persistence Boundary

## Purpose

This document defines the non-negotiable persistence boundary for `time_tracer`
ingest flows.

The core rule is:

> Ingest may create or mutate the SQLite database only after every validation
> stage has completed successfully.

This rule applies to Windows CLI, Android runtime reuse, and any future host
that bootstraps `tracer_core`.

## Required Ingest Order

Every ingest flow must preserve this order:

1. collect input sources
2. convert / parse input
3. validate structure
4. validate logic
5. enter the persistence gate
6. create / open database for write
7. initialize schema if needed
8. import or replace data transactionally

Steps `6` to `8` are forbidden unless steps `1` to `4` have already succeeded.

## What Counts As Persistence Side Effects

The following actions are write-side persistence side effects:

- creating a new `.sqlite3` file
- creating `-wal`, `-shm`, or `-journal` companion files
- opening the write repository in a mode that can create the database
- creating tables or indexes
- starting import / replace transactions
- inserting, deleting, or updating day / record / project rows

These side effects must not happen during host bootstrap, runtime creation, or
validation-only execution.

## Runtime Bootstrap Rules

Runtime bootstrap may do the following:

- resolve paths
- prepare log / output directories
- load config
- register formatters
- assemble validation services
- assemble read services that do not create a database

Runtime bootstrap must not do the following:

- create the ingest database
- initialize ingest write schema
- treat runtime creation as implicit permission to write

## Read Path vs Write Path

Read paths include:

- query
- report
- tree

Write paths include:

- ingest
- import
- replace-month ingest

Rules:

- read paths must not create a database when it does not already exist
- validation phases must not depend on write repository construction
- previous-tail lookups may read an existing database, but must not create a new
  one if the database is absent

## Failure Semantics

If the database does not exist before an ingest run starts, and validation
fails, the run must leave behind:

- no new `.sqlite3`
- no new `.sqlite3-wal`
- no new `.sqlite3-shm`
- no new `.sqlite3-journal`

If the database already exists before the run:

- validation failure must not mutate persisted data

## Design Implications

- write repositories must be lazily opened
- schema initialization must move to the write phase
- database health checks used by ingest must validate write preconditions
  without creating the database
- read services must report a missing database as an explicit error, not hide it
  by auto-creating a new one

## Implementation Snapshot (2026-03-07)

- `apps/tracer_core_shell/host/android_runtime_factory.cpp` keeps runtime
  bootstrap side-effect free with respect to ingest persistence and no longer
  bootstraps SQLite during runtime creation.
- `libs/tracer_core/src/infrastructure/persistence/importer/repository.*`
  stores the database path and opens / initializes SQLite only when a write path
  actually executes.
- `libs/tracer_core/src/infrastructure/persistence/sqlite_database_health_checker.cpp`
  validates write preconditions by checking path readiness instead of opening
  SQLite.
- `libs/tracer_core/src/infrastructure/reports/lazy_sqlite_report_query_service.*`
  and `lazy_sqlite_report_data_query_service.*` keep report/query reads lazy and
  return explicit missing-database errors instead of silently creating a new
  database.
- `apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/`
  contains regression coverage that locks the rule “invalid ingest must not
  create db”.

## Enforcement Notes

When changing runtime bootstrap, ingest workflow, or SQLite repository code:

- do not reintroduce database creation into constructors
- do not reintroduce database creation into runtime bootstrap
- keep the persistence gate explicit in ingest code
- add or update regression tests for “invalid ingest must not create db”
