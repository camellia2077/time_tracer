# Validity Versus Completeness

This document defines the difference between hard correctness and authored
record completeness.

## Validity

`validity` means whether authored input or generated data satisfies hard
business rules.

Validity errors may block validation, conversion, ingest, or persistence.

Examples of validity errors:

1. Invalid event-line format.
2. Unknown activity token where the input contract requires known tokens.
3. Wake anchor in an invalid position.
4. Zero duration.
5. Overlap.
6. Activity duration longer than the configured hard limit without an explicit
   override token.
7. Invalid backward range.
8. Wake token authored as an interval event, unless a future business rule
   explicitly allows it.

## Completeness

`completeness` means whether the authored record contains enough information for
the expected recording style or authoring workflow.

Completeness is not the same as validity.

Completeness issues should usually be warnings in authoring workflows, not hard
ingest errors.

## Examples

### Single Point Event

```text
0301
1200game
```

This may be valid authored input, but incomplete if no previous boundary exists
to infer `start_time`.

### Single Interval Event

```text
0301
0900-1030study
```

This can be valid and statistically useful because it already contains
`start_time` and `end_time`.

Do not treat one authored event as incomplete without considering event kind.

The event count alone is not enough to decide completeness.

### Sparse Interval Recording

```text
0301
0900-1030study

0305
1400-1500game
```

This can be valid selective recording.

Missing days do not automatically imply invalid input when authored intervals
are self-contained.

Sparse interval-event recording can still have low recorded coverage. That is a
coverage fact, not a validity error.

### Overlap

```text
0954-1007game
0954-1409lunch
```

This is not a completeness issue. It is a validity error because the intervals
overlap.

### Mixed Recording Gap

```text
0809breakfast
1200game
1230-1304sleep
```

`12:00 -> 12:30` is an unrecorded gap. It is not a validity error and should not
be silently filled.

### Wake Interval

```text
0600-0700wake
```

Under the current business rule, wake is a point-event anchor. Authoring wake as
an interval event is a validity error.

### Cross-Midnight Interval Too Long

```text
1030-0900study
```

This is interpreted as `10:30 -> next-day 09:00`, not as an unrecorded gap.
Because the resulting duration is 22h30m, it violates the normal single-activity
duration limit unless the event remark contains the long-duration override
token.

The long-duration override only bypasses the single-activity duration limit. It
does not bypass overlap, zero duration, invalid event syntax, or wake-anchor
interval rules.

## Authoring Warnings

Authoring clients may warn when data appears incomplete.

Examples:

1. A point-event day has too little context to infer a leading interval.
2. A day has sparse recorded coverage.
3. A wake anchor lacks previous context to generate overnight sleep.
4. A selective interval-event day records less than the civil-day span.
5. A day has no sleep interval in selective recording.

Warnings should not be promoted into hard validity errors unless a specific
business rule says so.

## Sleep And Overnight Warning Boundary

Missing sleep activity is not a validity error.

Missing wake anchor can support a `possible overnight continuation` warning in
point-event recording, but that warning is not recorded sleep duration.

Do not infer:

```text
no sleep interval => overnight
```

For the full sleep and overnight rule set, see
[overnight_and_sleep.md](overnight_and_sleep.md).
