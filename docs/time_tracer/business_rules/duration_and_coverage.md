# Duration And Coverage

This document defines the difference between recorded activity time and date or
calendar span.

The central rule is:

```text
recorded duration belongs to activity intervals
calendar span belongs to civil-day or date-range boundaries
```

They are related, but they are not the same measurement.

## Recorded Duration

`recorded duration` means the sum of durations of recorded activity intervals.

Formula:

```text
recorded_duration = sum(activity_interval.duration)
```

Example:

```text
0900-1030study
1400-1500game
```

The recorded duration is:

```text
1h30m + 1h = 2h30m
```

The unrecorded gap from `10:30` to `14:00` is not included.

Recorded duration is the right total for:

1. Activity statistics.
2. Project/category aggregation.
3. Query totals.
4. Insights totals that claim to show recorded activity time.

Prefer implementation names like:

```text
recorded_duration_seconds
```

Avoid `recorded_time` when the value is a duration, because `time` can mean a
timestamp or clock value.

Do not calculate recorded duration as:

```text
last_recorded_end - first_recorded_start
```

That expression measures a span, and it includes unrecorded gaps.

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

Calendar span is the right total for:

1. Civil-day or date-range denominator values.
2. Coverage ratios.
3. Calendar range metadata.
4. Explaining how much calendar time a query window covers.

Calendar span should not be used as the activity total unless every gap has
explicitly become a recorded activity interval.

## Civil-Day Span

`civil-day span` means the calendar span of one civil day.

Normally:

```text
civil_day_span = 24h
```

If the project later models time zones and daylight-saving transitions,
civil-day span may need explicit timezone rules.

Civil-day span is about the calendar. It is not the same as a logical day
boundary.

## Recorded Coverage Ratio

`recorded coverage ratio` means recorded duration divided by calendar span.

Formula:

```text
recorded_coverage_ratio = recorded_duration / calendar_span
```

Example:

```text
recorded_duration = 2h30m
calendar_span = 24h
recorded_coverage_ratio = 2.5 / 24
```

This ratio describes recording coverage only.

It is not automatically:

1. A validity score.
2. A quality score.
3. A reason to reject ingest.

Use coverage language carefully:

1. High coverage means more of the calendar span was recorded.
2. Low coverage means less of the calendar span was recorded.
3. Neither value proves whether the user's data is valid.
4. Neither value proves whether the user intended full-day recording.

## Business Rule

With interval-event recording and selective recording, it is normal for:

```text
recorded_duration < calendar_span
```

This means the user recorded only part of the date or range. It is not an
error.

In a fully recorded point-event workflow, recorded duration may be close to the
calendar span. That is a recording style outcome, not a universal invariant.

## Examples

### Full-Looking Point-Event Recording

```text
0600wake
1200work
1800dinner
2300sleep
```

If the leading boundary is known, this style can produce activity intervals
that cover most of a logical day. That does not make calendar span and recorded
duration the same concept.

### Selective Interval-Event Recording

```text
0900-1030study
1400-1500game
```

For one civil day:

```text
recorded_duration = 2h30m
calendar_span = 24h
recorded_duration < calendar_span
```

This is valid selective recording.

### Mixed-Event Recording With Gaps

```text
0809breakfast
1200game
1230-1304sleep
1404-1623study
1809internet
```

The gaps `12:00 -> 12:30` and `13:04 -> 14:04` do not contribute to recorded
duration. They still belong to the calendar span of the day or query range.
