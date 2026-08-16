# Recording Modes

This document defines how users can author time records.

The key distinction is between authored events and generated activity
intervals:

```text
authored event -> activity interval
```

## Point-Event Recording

`point-event recording` means the user records activity changes as point
events.

Format:

```text
`HHMMtoken` or `HHMMSS` + token
```

Example:

```text
0809breakfast
1200game
1809internet
```

Business meaning:

1. Each line provides one time boundary.
2. The boundary acts as the `end_time` of the generated activity interval.
3. The `start_time` is inferred from the last known boundary.
4. The first point event in a logical day may need previous context.
5. Cross-day or cross-month context can matter because the first interval may
   not be self-contained.

This is the preferred term for the old stream-style recording mode.

Point-event recording is the mode where continuity checks and previous-context
repair are most meaningful. The recording style implies a sequence of
boundaries, not isolated self-contained intervals.

## Point-Event Stream

`point-event stream` means a sequence of point events.

Use this term when the sequence itself matters. Prefer `point-event recording`
when discussing the user-facing recording mode.

## Interval-Event Recording

`interval-event recording` means the user records activity intervals
explicitly.

Format:

```text
`HHMM-HHMMtoken` or `HHMMSS-HHMMSS` + token
```

Example:

```text
0901-1200math
1220-1409lunch
1608-1900english
```

Business meaning:

1. Each line provides both `start_time` and `end_time`.
2. The event can produce an activity interval without previous-day context.
3. Missing time between interval events remains unrecorded.
4. Sparse days and sparse months can still contain valid recorded activity.
5. Date continuity is not required just to interpret interval events.
6. A single interval event can be valid and statistically useful.

Example of sparse interval-event recording:

```text
0301
0901-1200math
1220-1409lunch
1608-1900english

0305
0803-0907game
1320-1409lunch
1608-1900english
```

The missing days between `0301` and `0305` do not make these authored intervals
invalid. Each interval event is self-contained.

## Mixed-Event Recording

`mixed-event recording` means point events and interval events appear in the
same recording scope.

Example:

```text
0809breakfast
1200game
1230-1304sleep
1404-1623study
1809internet
```

Business meaning:

1. Point events infer `start_time` from the last known boundary.
2. Interval events use explicit `start_time` and `end_time`.
3. A generated activity interval advances the same timeline regardless of which
   authored event kind produced it.
4. An interval event's `end_time` becomes the next boundary for following point
   events.

Detailed interpretation:

```text
0809breakfast
1200game
1200-1304sleep
1304-1623study
1809internet
```

This produces:

```text
08:09 -> 12:00 game
12:00 -> 13:04 sleep
13:04 -> 16:23 study
16:23 -> 18:09 internet
```

The interval event `1200-1304sleep` advances the last known boundary to
`13:04`. The later point event `1809internet` starts from the previous interval
event's `end_time`, not from an earlier point event.

Mixed-event recording must be interpreted as one timeline. It is not two
independent modes running side by side.

If the shared timeline has already crossed midnight, later point events are
expanded relative to the last known boundary. The program does not move them
back to the earlier civil day to make the duration shorter.

Example:

```text
2132-0135study
2350game
```

The generated point-event activity is:

```text
01:35 -> 23:50 game
```

on the next civil day within the same logical-day bucket. If that duration is
longer than the normal single-activity limit, validation insights a duration
problem unless the event has the long-duration override token.

## Selective Recording

`selective recording` means the user intentionally records only some activity
intervals.

Example:

```text
0900-1030study
1400-1500game
```

The time from `10:30` to `14:00` is an unrecorded gap.

Selective recording is allowed when the recording mode provides explicit
intervals or otherwise leaves gaps unambiguous.

## Wake Tokens In Recording Modes

Wake tokens are wake anchors, not ordinary sleep intervals.

Current business rule:

1. A wake anchor is authored as a point event.
2. A wake token should not be authored as an interval event.
3. Generated overnight sleep is derived from previous context and a wake
   anchor.

Avoid this form unless a future business rule explicitly defines it:

```text
2305-0809wake
```

If the user intends to record sleep as an explicit activity interval, use a
sleep activity token rather than a wake token.
