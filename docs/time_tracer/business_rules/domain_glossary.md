# Domain Glossary

This file is the entry point for TimeTracer business vocabulary.

It is intentionally short. Detailed terms are split by domain area under
`glossary/` so each document stays focused.

## Naming Principles

1. Use `event` for authored input lines.
2. Use `interval` for normalized activity facts with `start_time` and
   `end_time`.
3. Use `duration` for elapsed time measured by summing intervals.
4. Use `span` for a calendar or query range boundary.
5. Use `contiguous`, not `continuous`, when two intervals touch end-to-start.
6. Use `logical day` for TimeTracer's user-domain day.
7. Use `civil day` for the calendar day.

## Glossary Sections

1. [events_and_intervals.md](glossary/events_and_intervals.md)
   - `authored event`, `point event`, `interval event`, `activity token`,
     `activity interval`.
2. [recording_modes.md](glossary/recording_modes.md)
   - `point-event recording`, `point-event stream`, `interval-event
     recording`, `mixed-event recording`, `mixed timeline`.
3. [day_terms.md](glossary/day_terms.md)
   - `civil day`, `logical day`, `wake anchor`, `continuation day`.
4. [timeline_terms.md](glossary/timeline_terms.md)
   - `last known boundary`, `contiguous`, `contiguity scope`, `unrecorded
     gap`, `overlap`, `cross-midnight activity`, `invalid backward range`.
5. [duration_terms.md](glossary/duration_terms.md)
   - `recorded duration`, `calendar span`, `civil-day span`, `recorded
     coverage ratio`.
6. [validation_terms.md](glossary/validation_terms.md)
   - `validity`, `completeness`.
7. [overnight_terms.md](glossary/overnight_terms.md)
   - `recorded sleep duration`, `overnight sleep interval`, `possible
     overnight continuation`, `missing wake anchor`.
8. [naming_conventions.md](glossary/naming_conventions.md)
   - Terms to avoid or narrow, including `stream-style recording`,
     `interval-style recording`, and `continuous`.

## Summary Table

| Term Family | Preferred Terms |
| --- | --- |
| Authored input | `authored event`, `point event`, `interval event`, `activity token` |
| Normalized facts | `activity interval` |
| Recording modes | `point-event recording`, `interval-event recording`, `mixed-event recording` |
| Day model | `civil day`, `logical day`, `wake anchor`, `continuation day` |
| Timeline relationships | `last known boundary`, `contiguous`, `unrecorded gap`, `overlap`, `cross-midnight activity` |
| Time totals | `recorded duration`, `calendar span`, `recorded coverage ratio` |
| Sleep and overnight | `recorded sleep duration`, `overnight sleep interval`, `possible overnight continuation`, `missing wake anchor` |
| Correctness | `validity`, `completeness` |
