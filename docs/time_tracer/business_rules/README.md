# TimeTracer Business Rules

This directory is the entry point for TimeTracer business rules.

Read these documents before changing parsing, conversion, validation, query,
reporting, CLI authoring, or Android authoring behavior. They define what the
project means by time, events, intervals, logical days, recorded duration, and
validity.

These documents describe domain semantics first. They are not implementation
notes and should not be reverse-engineered from current code.

## Reading Order

1. [domain_glossary.md](domain_glossary.md)
   - Shared vocabulary. Start here when terminology is unclear.
2. [time_model.md](time_model.md)
   - Defines time points, intervals, durations, spans, civil days, and logical
     days.
3. [recording_modes.md](recording_modes.md)
   - Defines point-event recording, interval-event recording, and mixed-event
     recording.
4. [logical_day.md](logical_day.md)
   - Explains why TimeTracer's day model is not just a fixed 24-hour calendar
     day.
5. [timeline_rules.md](timeline_rules.md)
   - Defines last known boundary, contiguous intervals, unrecorded gaps,
     overlaps, and invalid backward ranges.
6. [duration_and_coverage.md](duration_and_coverage.md)
   - Defines recorded duration, calendar span, civil-day span, and recorded
     coverage ratio.
7. [overnight_and_sleep.md](overnight_and_sleep.md)
   - Defines recorded sleep duration, overnight sleep intervals, possible
     overnight continuation, and missing wake anchors.
8. [validation_vs_completeness.md](validation_vs_completeness.md)
   - Defines hard validity errors versus authoring completeness warnings.

## Core Business Premises

1. Users write authored events.
2. The system converts authored events into activity intervals.
3. Query and reporting operate on activity intervals.
4. A logical day is the user's domain day and can cross civil midnight.
5. A civil day is the calendar day from local `00:00` to the next local
   `00:00`.
6. Recorded duration is the sum of recorded activity interval durations.
7. Calendar span is the elapsed span of a civil day or date range.
8. Unrecorded gaps are allowed when the recording mode supports selective
   recording.
9. Overlap is a validity error unless a future business rule explicitly
   introduces parallel recording.
10. Completeness is not the same as validity.
11. Missing sleep activity does not imply overnight.
12. Missing wake anchor is metadata/completeness, not recorded sleep.

## Naming Rule

Prefer terms from [domain_glossary.md](domain_glossary.md) in new docs, tests,
code comments, UI copy, CLI output, and Android text.
