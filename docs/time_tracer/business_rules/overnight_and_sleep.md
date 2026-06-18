# Overnight And Sleep

This document defines sleep and overnight-related business terms.

The central rule is:

```text
missing sleep record != overnight
```

After interval-event recording and selective recording are supported, the user
may intentionally omit sleep intervals. Missing sleep data must not be
interpreted as proof that the user stayed awake overnight.

## Recorded Sleep Duration

`recorded sleep duration` means the sum of activity intervals that are
classified as sleep.

It is part of recorded duration:

```text
recorded_sleep_duration = sum(sleep activity interval durations)
```

It may include:

1. Explicit interval-event sleep records.
2. Generated overnight sleep intervals.
3. Other sleep activity intervals defined by business rules.

It must not include unrecorded gaps.

## Overnight Sleep Interval

`overnight sleep interval` means a recorded or generated sleep activity interval
that crosses a logical-day or civil-midnight boundary according to business
rules.

Generated `sleep_night` is one form of overnight sleep interval:

```text
previous known boundary -> current wake anchor
```

Because it is an activity interval, generated `sleep_night` contributes to
recorded duration and recorded sleep duration.

## Wake Anchor

A `wake anchor` is a point-event day marker. It is not an activity interval by
itself.

Wake anchors can support generated overnight sleep when previous context exists.

Wake anchors should not be treated as proof that an explicit sleep record
exists.

## Possible Overnight Continuation

`possible overnight continuation` means the authored data may be continuing
from previous context, usually because a leading point event has no wake anchor
or no local start boundary.

This is a completeness or metadata condition.

It is not the same as recorded sleep.

## Missing Wake Anchor

`missing wake anchor` means day metadata does not contain a valid wake anchor.

This can be useful for authoring warnings or query filters, but it must not be
reported as sleep duration or overnight sleep by itself.

## Business Rules

1. Missing sleep activity does not imply overnight.
2. Missing wake anchor does not imply recorded sleep.
3. Generated `sleep_night` is an activity interval and contributes to recorded
   duration.
4. Explicit sleep intervals contribute to recorded sleep duration.
5. A selective interval-event day can omit sleep entirely and still be valid.
6. "Possible overnight continuation" belongs to completeness/warning semantics,
   not activity statistics.

## Examples

### Explicit Sleep Interval

```text
2305-0809sleep
```

If this range is accepted by the cross-day interval rules, it is a recorded
sleep activity interval.

### Missing Sleep In Selective Recording

```text
0900-1030study
1400-1500game
```

There is no sleep record. This does not mean the user stayed awake overnight.

### Wake Anchor With Previous Context

```text
previous boundary: 23:30
current day:
0700wake
```

The system may generate:

```text
23:30 -> 07:00 sleep_night
```

That generated interval is recorded sleep duration.

### Missing Wake Anchor

```text
0301
1200game
```

This may indicate possible overnight continuation in point-event recording. It
does not prove sleep happened, and it does not prove overnight wakefulness.
