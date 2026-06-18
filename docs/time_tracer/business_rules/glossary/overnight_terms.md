# Overnight Terms

This glossary section defines sleep and overnight terms.

## Recorded Sleep Duration

`recorded sleep duration` means the sum of durations of recorded sleep activity
intervals.

It is calculated from activity intervals, not from missing data.

## Overnight Sleep Interval

`overnight sleep interval` means a recorded or generated sleep activity interval
that crosses a logical-day or civil-midnight boundary according to business
rules.

Generated `sleep_night` is an overnight sleep interval when it is created from
previous context and a wake anchor.

## Possible Overnight Continuation

`possible overnight continuation` means the authored data may continue from
previous context because the leading point-event data lacks a wake anchor or
local start boundary.

It is a metadata or completeness concept, not a sleep activity interval.

## Missing Wake Anchor

`missing wake anchor` means day metadata does not contain a valid wake anchor.

It may support warnings or filters. It must not be interpreted as recorded
sleep duration.
