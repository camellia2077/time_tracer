# Activity Remark Update Contract v1

## Purpose

This contract defines the core operation used to edit one authored activity
remark from a structured insights. The TXT file remains the source of truth and
the database is rebuilt from the candidate TXT in the first implementation.

## Request

```text
target_date_iso      YYYY-MM-DD
logical_id           stable database activity-record identity
remark               decoded user-facing remark; empty removes the remark
preferred_txt_path   optional managed month TXT path
date_check_mode      existing ingest date-check policy
```

The combination of date and logical id identifies the activity record. The
operation changes only its remark. Activity name, start time, and end time are
immutable for this operation and must remain unchanged in TXT and the database
projection.

## Success and failure

The operation is successful only after the candidate TXT has passed structure
and logic validation and the single-month database replacement ingest has
completed. On ingest failure, the official TXT is restored. A rollback failure
returns the transaction workspace so the host can surface recovery guidance.

The first implementation intentionally uses full single-month ingest. An
incremental database fast path may be added later without changing this
contract.

## TXT representation

The user-facing remark may contain real newlines. TXT stores each physical
line with the `//` continuation marker:

```text
0900study // first line
// second line
```

Core and SQLite keep the remark as one `TEXT` value containing real LF
characters. The two characters `\\n` written by the user remain literal text;
they are not decoded into a newline.

## Ownership

Core owns record-to-TXT resolution, remark-only TXT mutation, validation
ordering, ingest, and rollback. Transport owns wire encoding. Hosts only
collect the record identity and new remark, display the result, and refresh
the insights.
