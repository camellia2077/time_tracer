# Timeline Rules

This document defines timeline relationships between activity intervals.

## Last Known Boundary

`last known boundary` is the most recent reliable time boundary available while
converting authored events into activity intervals.

It can come from:

1. A wake anchor.
2. A previous point event.
3. A previous interval event's `end_time`.
4. Previous context when a point-event recording crosses a logical-day
   boundary.

For mixed-event recording, both point events and interval events update the
same last known boundary after they produce an activity interval.

An interval event advances the boundary to its explicit `end_time`.

A point event advances the boundary to its authored `HHMM`.

When an authored boundary appears after a previous cross-midnight boundary, the
timeline stays monotonic. The program must not reinterpret a later clock value
as an earlier same-logical-day time just to shorten the duration.

Example:

```text
2132-0135study
2350game
```

The point event `2350game` uses the previous interval's end boundary as its
`start_time`:

```text
next-day 01:35 -> next-day 23:50 game
```

It is not interpreted as same-logical-day `23:50`, because that would move the
timeline backward and create an ambiguous overlap. The resulting duration is
handled by normal activity duration validation.

## Contiguous

`contiguous` means two neighboring activity intervals touch exactly.

Formula:

```text
previous.end_time == current.start_time
```

or:

```text
current.end_time == next.start_time
```

Use `contiguous`, not `continuous`.

This project does not use the mathematical meaning of continuous functions.

## Contiguity Scope

Contiguity is always evaluated within a scope.

Valid scopes include:

1. A logical day.
2. A query range.
3. Neighboring authored records.
4. Generated activity intervals from one conversion run.

The first interval in a scope has no previous interval in that scope.

The last interval in a scope has no next interval in that scope.

Do not search indefinitely across all stored history just to make every
interval comparable with a previous or next interval.

This prevents the first interval in a scope from being incorrectly judged
against unrelated historical data, and prevents the last interval in a scope
from requiring a future interval that does not exist yet.

## Unrecorded Gap

`unrecorded gap` means a span between two activity intervals where no activity
interval exists.

Example:

```text
0900-1030study
1400-1500game
```

`10:30 -> 14:00` is an unrecorded gap.

An unrecorded gap:

1. Is allowed in interval-event recording and mixed-event recording.
2. Is not automatically filled.
3. Does not contribute to recorded duration.
4. Is not a validity error by itself.
5. Does not belong to the previous activity interval.
6. Does not belong to the next activity interval.

Example from mixed-event recording:

```text
0809breakfast
1200game
1230-1304sleep
1404-1623study
1809internet
```

The spans `12:00 -> 12:30` and `13:04 -> 14:04` are unrecorded gaps.

## Overlap

`overlap` means two activity intervals cover the same time span in whole or in
part.

Example:

```text
0954-1007game
0954-1409lunch
```

The second interval begins before the first interval ends.

Unless a future business rule explicitly supports parallel recording, overlap
is a validity error.

Overlap is different from a gap:

```text
0954-1007game
0954-1409lunch
```

The second interval starts before the first interval ends, so it is not
selective recording. It is conflicting recorded time.

## Cross-Midnight Interval

`cross-midnight interval` means an authored interval event whose `end_time`
clock value is earlier than its `start_time` clock value, and is therefore
interpreted as ending on the next civil day.

Example:

```text
2132-0135study
```

This means:

```text
21:32 -> next-day 01:35
```

The written `end_time` remains `01:35`, but duration calculation treats it as
the next day's `01:35`.

Cross-midnight interval authoring is allowed, but it is still subject to normal
activity duration validation. In particular, a single activity longer than 16
hours is a validity error unless the event remark contains the long-duration
override token.

Example:

```text
1030-0900study
```

This is interpreted as a cross-midnight interval with a duration of 22h30m. It
is invalid under the normal 16-hour activity duration limit unless the authored
event explicitly opts into the long-duration override.

Cross-midnight activity intervals remain assigned to the logical day bucket in
which their `start_time` is authored. Query and report date filters use that
logical-day bucket, not the civil date of the interval's `end_time`.

## Cross-Midnight Activity Query Fact

`cross_midnight_activity` is the query/API field name for filtering logical
days that contain at least one activity interval crossing local civil `00:00`.

It is defined by timeline boundaries only. It must not be implemented by any of
these proxy conditions:

1. Missing sleep activity.
2. Missing wake anchor.
3. `getup_time` being empty.
4. `getup_time` being `00:00`.
5. A normal sleep interval that does not cross `00:00`.

`cross_midnight_activity` is also not equivalent to `overnight sleep interval`.
An overnight sleep interval is sleep-specific; a cross-midnight activity can be
any activity category.

## Invalid Backward Range

`invalid backward range` means an interval event whose end boundary is earlier
than its start boundary and still cannot produce a valid cross-midnight
activity interval under the duration rules.

Example:

```text
1007-1007game
```

This is invalid because it has zero duration.

Backward-looking clock text by itself is not enough to decide validity. First
interpret `start_time > end_time` as a cross-midnight interval. Then apply
duration validation.

## Gap, Contiguous, Overlap

For neighboring intervals:

```text
previous.end_time < current.start_time   => unrecorded gap
previous.end_time == current.start_time  => contiguous
previous.end_time > current.start_time   => overlap
```

For cross-midnight neighbors, compare the expanded timeline values, not only
the visible clock text.

Examples:

```text
2300-0100study
0030-0200game
```

The second interval expands to `next-day 00:30 -> next-day 02:00`, which starts
before the previous interval ends at `next-day 01:00`. This is an overlap.

```text
2300-0100study
0130-0200game
```

The second interval expands to `next-day 01:30 -> next-day 02:00`. This leaves
an unrecorded gap from `01:00 -> 01:30` and is valid.

## Sparse Dates

Sparse dates are allowed when authored intervals are self-contained.

Example:

```text
0301
0900-1030study

0305
1400-1500game
```

The missing dates between `0301` and `0305` do not prevent interpreting the
recorded intervals.

Date continuity rules may still be useful for point-event recording workflows
that depend on previous context. They should not be applied blindly to reject
self-contained interval-event recording.
