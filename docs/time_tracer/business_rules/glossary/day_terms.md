# Day Terms

This glossary section defines calendar and TimeTracer day concepts.

## Civil Day

`civil day` means a calendar day from local `00:00` to the next local `00:00`.

Example:

```text
2026-03-01 00:00 -> 2026-03-02 00:00
```

A civil day normally has a 24-hour civil-day span.

## Logical Day

`logical day` means TimeTracer's user-domain day.

It is the day to which the user assigns activity while recording life and work.
It may cross civil midnight when the user continues activity after `00:00`.

`logical day` should not be treated as identical to a fixed 24-hour civil-day
span.

## Wake Anchor

`wake anchor` means a day-level authored point event that marks wake semantics
for a logical day.

It is a day-level semantic marker. It is not itself a sleep interval.

## Continuation Day

`continuation day` means a logical day whose first authored point event is not a
wake anchor and whose leading interval may require previous context.

Do not use `continuation day` for a day whose first authored event is already a
self-contained interval event.
