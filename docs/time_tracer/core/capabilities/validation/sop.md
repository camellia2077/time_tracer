# Validation SOP

## Core Rule

No ingest flow may create or mutate the SQLite database until every required
validation stage has succeeded.

## Required Order

For ingest-like flows, preserve this order:

1. collect input sources
2. load and validate required config
3. canonicalize input text
4. parse and validate TXT structure
5. validate TXT logic
6. enter the persistence gate
7. create/open database for write
8. initialize schema if needed
9. import or replace data transactionally

Steps `7` to `9` are forbidden unless steps `1` to `5` already succeeded.

## What Must Happen Before Logic Validation

1. Converter config must already be available and valid.
2. TXT bytes must already be normalized to canonical text.
3. Structure validation must already have produced a valid parsed view.

## Exchange Import Note

`tracer exchange import` follows the same high-level rule, but its effective
input set is assembled first:

1. decrypt package
2. validate package converter `TOML`
3. build effective TXT view
4. validate TXT structure
5. validate TXT logic
6. replace managed TXT and rebuild DB only after validation passes

This is why imported converter config must be validated and applied before the
effective TXT logic check runs.

## Failure Semantics

If validation fails before the persistence gate:
1. no new `.sqlite3` should be created when the DB did not exist before
2. no `-wal`, `-shm`, or `-journal` companions should be left behind
3. existing persisted data must remain unchanged

## Enforcement Checklist

When changing pipeline, import, runtime bootstrap, or repository code:
1. do not move DB creation into constructors or runtime bootstrap
2. do not make validation depend on write repository construction
3. keep the persistence gate explicit in code and tests
4. add regression coverage for invalid-input-no-write behavior

## Read With
1. [overview.md](overview.md)
2. [../ingest/persistence_boundary.md](../ingest/persistence_boundary.md)
3. [../ingest/overview.md](../ingest/overview.md)
