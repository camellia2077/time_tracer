# Timeline Terms

This glossary section defines relationships between activity intervals.

## Last Known Boundary

`last known boundary` means the most recent reliable time boundary used to
infer a point event's start boundary.

It can come from:

1. A wake anchor.
2. A previous point event.
3. A previous interval event's `end_time`.
4. Previous context when point-event recording crosses a logical-day boundary.

In mixed-event recording, an interval event's `end_time` can become the last
known boundary for a following point event.

## Contiguous

`contiguous` means two neighboring intervals touch exactly:

```text
previous.end_time == current.start_time
```

Use `contiguous`, not `continuous`.

## Contiguity Scope

`contiguity scope` means the boundary within which contiguity is evaluated.

The first interval in a scope has no previous interval in that scope. The last
interval in a scope has no next interval in that scope.

## Unrecorded Gap

`unrecorded gap` means a span between activity intervals where no activity
interval exists.

An unrecorded gap is not automatically filled and does not contribute to
recorded duration.

It is allowed by selective interval-event recording and mixed-event recording.

## Overlap

`overlap` means two activity intervals cover the same time span in whole or in
part.

Unless a future feature explicitly introduces parallel recording, overlap is a
validity error.

## Cross-Midnight Activity

`cross-midnight activity` means an activity interval whose expanded timeline
crosses local civil `00:00`.

In persisted query data, the current compact representation detects this when
the visible `start_time` clock value is later than the visible `end_time` clock
value:

```text
start_time > end_time
```

This is a timeline fact. It is not a sleep fact, a wake-anchor fact, or proof
that the user stayed awake overnight.

Use `cross_midnight_activity` for query/API fields that filter this fact. Do
not use `overnight` as a shorthand for this query meaning.

## Invalid Backward Range

`invalid backward range` means an interval whose end boundary is earlier than
its start boundary without satisfying an accepted cross-day rule.

This should not be treated as an unrecorded gap.
