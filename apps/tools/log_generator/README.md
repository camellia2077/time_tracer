# log_generator

CLI app for generating test log datasets used by validation pipelines.

## Current Event Styles

- `point` is the default output.
- `interval` is selected with `--event-style interval`.
- `mixed` is selected with `--event-style mixed`; wake remains a point event,
  while non-wake events choose point or interval with a fixed 50/50 probability.

For CLI usage and dataset behavior, see
`docs/tools/log_generator/usage.md`. Agent ownership, validation, result
evidence, and completion requirements live in
`apps/tools/log_generator/AGENTS.md`.
