# TXT Logic Validation

## Scope

TXT logic validation runs after config validation, canonical text
normalization, and successful structure validation.

Its job is to verify that the parsed activity stream is semantically acceptable
for downstream ingest and reporting.

## What It Checks

1. date continuity across the source set
2. duration sanity and zero-duration restrictions
3. time discontinuity and impossible sequence shapes
4. wake-anchor rules
5. sleep-day and overnight continuity semantics
6. recognition of activities against config and alias mapping

## Important Clarification

This phase is not pure syntax checking and not regex checking.

It is business-rule validation over parsed activities and day semantics.

It no longer treats "a day has fewer than 2 authored events" as a blocking
logic-validation failure. That situation is considered authoring completeness,
not ingest invalidity.

## Wake Anchor And Day Semantics

The most commonly misunderstood logic rules are documented separately in:
1. [../../ingest/day_bucket_and_wake_anchor_semantics.md](../../ingest/day_bucket_and_wake_anchor_semantics.md)

That document explains:
1. why `Total Time Recorded` may exceed `24h`
2. how `wake_keywords`, `isContinuation`, and `getupTime` interact
3. why `wake_anchor` and `sleep_*` activities are related but not equivalent
4. why `<2 authored events` is surfaced only as authoring-time warning in
   `Record Input` / `TXT save+sync`

## Why Config Still Matters Here

Logic validation still depends on validated config because activity semantics
and wake-related meaning come from converter configuration and alias mapping.

For authoring-facing input checks, use the explicit config-driven boundary:
1. `alias_mapping.keys` for alias-only lookup
2. `wake_keywords` for wake semantics
3. `authorable_event_tokens = alias_mapping.keys ∪ wake_keywords` for direct
   user-entered event tokens

## Code Entry References
1. `libs/tracer_core/src/domain/logic/validator/txt/rules/txt_rules.cpp`
2. `libs/tracer_core/src/domain/logic/validator/txt/facade/text_validator.cpp`
