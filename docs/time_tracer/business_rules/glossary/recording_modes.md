# Recording Mode Terms

This glossary section defines the terms used for user recording styles.

## Point-Event Recording

`point-event recording` means the user records activity changes as point
events.

This is the preferred term for the old stream-style recording mode.

## Point-Event Stream

`point-event stream` means a sequence of point events.

Use this term only when emphasizing the sequence itself. Prefer
`point-event recording` when describing the recording mode.

## Interval-Event Recording

`interval-event recording` means the user records activity intervals explicitly
with interval events.

This is the preferred term for interval-style recording.

Interval-event recording is self-contained at the event level: each interval
event carries its own start and end boundaries.

## Mixed-Event Recording

`mixed-event recording` means point events and interval events appear in the
same recording scope.

Both event kinds advance one shared timeline after producing activity
intervals.

## Mixed Timeline

`mixed timeline` means the normalized timeline produced from mixed-event
recording.

Use `mixed timeline` when discussing validation, gaps, overlaps, ordering, or
query behavior. Use `mixed-event recording` when discussing authored input.
