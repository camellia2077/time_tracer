# Logical Day

This document defines `logical day`, `civil day`, wake semantics, and overnight
continuation.

## Civil Day

A `civil day` is the calendar day:

```text
local 00:00 -> next local 00:00
```

Example:

```text
2026-03-01 00:00 -> 2026-03-02 00:00
```

## Logical Day

A `logical day` is the user-domain day used by TimeTracer.

It represents how the user experiences and assigns activity. A logical day can
cross civil midnight.

Example:

If the user keeps working after `00:00`, that activity can still belong to the
previous logical day because the user has not finished that day's activity.

## Why Logical Day Exists

TimeTracer is not only measuring calendar occupancy. It is recording the user's
activity narrative.

This matters for:

1. Staying up past midnight.
2. Overnight sleep.
3. Wake anchors.
4. Continuation days.
5. Cross-day activity intervals.

## Date Assignment For Activity Intervals

Activity intervals are assigned to the logical day bucket where their
`start_time` is authored or generated.

For a cross-midnight interval written under a logical day:

```text
0305
2300-0100study
```

the activity is stored and queried as part of logical day `0305`, even though
its `end_time` occurs on the next civil day.

Date filters and report day selection use this logical-day assignment. They do
not split one activity interval across civil dates, and they do not reassign the
activity to the civil date of `end_time`.

## Wake Anchor

A `wake anchor` is a day-level semantic marker.

It marks the beginning semantics of a logical day when the first authored point
event belongs to the wake semantic set.

It is not itself a sleep interval.

## Generated Sleep

Generated overnight sleep is an activity interval produced from previous
context and a wake anchor:

```text
previous boundary -> current wake boundary
```

This generated interval is an activity fact. It is not authored directly as the
wake event.

Generated sleep is recorded sleep duration because it is an activity interval.
Missing sleep activity must not be interpreted as generated sleep.

## Continuation Day

A `continuation day` is a logical day whose first authored point event is not a
wake anchor and therefore may continue from previous context.

Continuation applies to point-event recording because the first point event
does not carry its own `start_time`.

Do not classify a day as a continuation day only because its first authored
event is not wake. If the first authored event is a self-contained interval
event, it does not need previous context to produce its first activity interval.

## Logical Day Is Not A Fixed Span

Do not define logical day as exactly:

```text
00:00 -> 24:00
```

That is civil-day span.

Logical day boundaries are domain boundaries influenced by recording semantics,
wake semantics, and previous context.

## Related Rules

For sleep and overnight terminology, see
[overnight_and_sleep.md](overnight_and_sleep.md).
