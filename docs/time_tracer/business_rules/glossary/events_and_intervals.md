# Events And Intervals

This glossary section defines authored input entities and normalized activity
facts.

## Authored Event

`authored event` means one event line written by the user in TXT or inserted by
an authoring client.

An authored event is input data. It is not yet the normalized activity fact used
for querying or reporting.

Examples:

```text
1200game
0900-1030study
```

## Point Event

`point event` means an authored event that records one time boundary and an
activity token.

Format:

```text
HHMMtoken
```

Example:

```text
1200game
```

Meaning:

1. `HHMM` is the event boundary.
2. For activity conversion, it acts as the `end_time` of the generated activity
   interval.
3. Its `start_time` must be inferred from context.

## Interval Event

`interval event` means an authored event that records explicit start and end
boundaries.

Format:

```text
HHMM-HHMMtoken
```

Example:

```text
0900-1030study
```

Meaning:

1. The first `HHMM` is the explicit `start_time`.
2. The second `HHMM` is the explicit `end_time`.
3. The event can produce an activity interval without previous-day context.

## Activity Token

`activity token` means the authored activity text after the time prefix.

Examples:

```text
game
study
wake
```

The token is not itself a time interval. It becomes part of an activity interval
only after conversion.

## Activity Interval

`activity interval` means the normalized activity fact used by core logic,
persistence, query, and reporting.

An activity interval has at least:

1. `start_time`
2. `end_time`
3. `duration`
4. activity or project identity

All user-facing statistics should be based on activity intervals, not directly
on authored events.
