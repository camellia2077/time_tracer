# Duration Terms

This glossary section defines duration and span terms.

Use `duration` for summed activity time. Use `span` for calendar or range
boundaries.

## Recorded Duration

`recorded duration` means the sum of durations of recorded activity intervals.

Formula:

```text
recorded_duration = sum(activity_interval.duration)
```

It should not include unrecorded gaps.

It should be calculated from activity intervals, not from the first and last
recorded boundary.

Prefer implementation names like:

```text
recorded_duration_seconds
```

Avoid `recorded_time` when the value is a duration, because `time` can mean a
timestamp or clock value.

## Calendar Span

`calendar span` means the elapsed span covered by a civil-day or date-range
boundary.

Examples:

```text
one civil day = 24h
seven civil days = 168h
```

Prefer implementation names like:

```text
calendar_span_seconds
```

Calendar span is a denominator or range measurement. It is not the same as
recorded activity time.

## Civil-Day Span

`civil-day span` means the calendar span of one civil day.

In normal local-time cases, this is 24 hours.

Civil-day span belongs to the calendar day, not necessarily to a TimeTracer
logical day.

## Recorded Coverage Ratio

`recorded coverage ratio` means recorded duration divided by calendar span.

Formula:

```text
recorded_coverage_ratio = recorded_duration / calendar_span
```

This ratio describes recording coverage only. It is not automatically a
validity score or quality score.

## Normal Partial Recording

With interval-event recording or selective recording, this is normal:

```text
recorded_duration < calendar_span
```

It means some calendar time was not recorded as an activity interval.
