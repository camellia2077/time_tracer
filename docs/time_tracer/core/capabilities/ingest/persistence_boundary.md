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

1. creating a new `.sqlite3` file
2. creating `-wal`, `-shm`, or `-journal` companion files
3. opening the write repository in a mode that can create the database
4. creating tables or indexes
5. starting import / replace transactions
6. inserting, deleting, or updating day / record / project rows

These side effects must not happen during host bootstrap, runtime creation, or
validation-only execution.

## Runtime Bootstrap Rules

Runtime bootstrap may:
1. resolve paths
2. prepare log/output directories
3. load config
4. register formatters
5. assemble validation services
6. assemble read services that do not create a database

Runtime bootstrap must not:
1. create the ingest database
2. initialize ingest write schema
3. treat runtime creation as implicit permission to write

## Read Path vs Write Path

Read paths include:
1. query
2. report
3. tree

Write paths include:
1. ingest
2. import
3. replace-month ingest

Rules:
1. read paths must not create a database when it does not already exist
2. validation phases must not depend on write repository construction
3. previous-tail lookups may read an existing database, but must not create a
   new one if the database is absent

## Failure Semantics

If the database does not exist before an ingest run starts, and validation
fails, the run must leave behind:

1. no new `.sqlite3`
2. no new `.sqlite3-wal`
3. no new `.sqlite3-shm`
4. no new `.sqlite3-journal`

If the database already exists before the run:
1. validation failure must not mutate persisted data

## Design Implications

1. write repositories must be lazily opened
2. schema initialization must move to the write phase
3. database health checks used by ingest must validate write preconditions
   without creating the database
4. read services must report a missing database as an explicit error, not hide
   it by auto-creating a new one

## Implementation Snapshot (2026-03-07)

1. `apps/tracer_core_shell/host/bootstrap/android_runtime_factory.cpp` keeps
   runtime bootstrap side-effect free with respect to ingest persistence and no
   longer bootstraps SQLite during runtime creation.
2. `libs/tracer_core/src/infra/persistence/importer/repository.*` stores the
   database path and opens/initializes SQLite only when a write path actually
   executes.
3. `libs/tracer_core/src/infra/persistence/sqlite_database_health_checker.module.cpp`
   validates write preconditions by checking path readiness instead of opening
   SQLite.
4. `libs/tracer_core/src/infra/reporting/lazy_sqlite_report_query_service.module.cpp`
   and `lazy_sqlite_report_data_query_service.module.cpp` keep report/query
   reads lazy and return explicit missing-database errors instead of silently
   creating a new database.
5. `apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/`
   contains regression coverage that locks the rule "invalid ingest must not
   create db".

## Enforcement Notes

When changing runtime bootstrap, ingest workflow, or SQLite repository code:
1. do not reintroduce database creation into constructors
2. do not reintroduce database creation into runtime bootstrap
3. keep the persistence gate explicit in ingest code
4. add or update regression tests for "invalid ingest must not create db"

## Read With
1. [../validation/sop.md](../validation/sop.md)
2. [overview.md](overview.md)
