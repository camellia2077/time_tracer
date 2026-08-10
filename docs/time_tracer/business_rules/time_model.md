# Time Model

This document defines TimeTracer's core time model.

The project is based on user-authored events that are converted into activity
intervals. Statistics and insights should be based on those activity intervals.

## Time Point

A `time point` is a local clock boundary such as:

```text
08:09
12:00
23:30
```

In TXT authoring, time points are written as `HHMM`.

Time points are boundaries. A time point alone is not a duration.

## Activity Interval

An `activity interval` is the normalized fact used for storage, query, and
insights.

It has:

1. `start_time`
2. `end_time`
3. `duration`
4. activity or project identity

Example:

```text
08:09 -> 12:00 game
```

## Duration

`duration` is elapsed time between an interval's `start_time` and `end_time`.

Example:

```text
08:09 -> 12:00 = 3h51m
```

Use `duration` when summing activity time.

## Span

`span` is the elapsed boundary of a calendar day, date range, or query range.

Example:

```text
2026-03-01 00:00 -> 2026-03-02 00:00 = 24h
```

Use `span` for date/range boundaries, not for summed recorded activity.

## Civil Day

A `civil day` is a calendar day from local `00:00` to the next local `00:00`.

Example:

```text
2026-03-01 00:00 -> 2026-03-02 00:00
```

The normal civil-day span is 24 hours.

## Logical Day

A `logical day` is TimeTracer's user-domain day.

It represents the day to which the user assigns activity. It may cross civil
midnight when the user stays up and continues the previous day's activity.

Do not assume:

```text
logical day == civil day
```

## Recorded Duration Versus Calendar Span

`recorded duration` is the sum of activity interval durations.

`calendar span` is the elapsed date/range span.

They are different concepts:

```text
recorded_duration = sum(activity_interval.duration)
calendar_span = date_range.end - date_range.start
```

In a fully recorded civil day, recorded duration may approach the civil-day
span. In selective recording, recorded duration can be much smaller.

This is expected behavior, not an error.
