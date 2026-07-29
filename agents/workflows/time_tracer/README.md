# time_tracer Workflows Index

This directory contains agent workflow policies for `time_tracer`.

## Clang-Tidy Workflow Map

Use this matrix to pick the right document:

- Scope axis:
  - `all`: full queue / end-to-end cleanup flow
  - `by_num`: one specific pending task (`task_NNN`)
- Profile axis:
  - `daily`: repo-root `.clang-tidy`
  - `strict`: repo-root `.clang-tidy.strict` (via `--strict-config`)
- Base axis:
  - `shared`: profile-agnostic common contract and process rules

## Files and Purpose

- `clang_tidy_all_shared.md`
  - Shared base for full-queue cleanup (`all`), independent of daily/strict.
  - Defines common path contract, task handling order, batch policy, completion gate, and guardrails.

- `clang_tidy_all_daily.md`
  - Daily-profile bindings for full-queue cleanup.
  - Adds `.clang-tidy` policy and expands daily commands.

- `clang_tidy_all_strict.md`
  - Strict-profile bindings for full-queue cleanup.
  - Adds `--strict-config` policy and expands strict commands.

- `clang_tidy_by_num_shared.md`
  - Shared base for single-task cleanup (`by_num`), independent of daily/strict.
  - Defines one-task contract, required command order, batch handoff, and completion checks.

- `clang_tidy_by_num_daily.md`
  - Daily-profile bindings for single-task cleanup.
  - Adds `.clang-tidy` policy and expands daily commands.

- `clang_tidy_by_num_strict.md`
  - Strict-profile bindings for single-task cleanup.
  - Adds `--strict-config` policy and expands strict commands.

## Selection Guide

- Need to process one known task id: start from `clang_tidy_by_num_*`.
- Need to run/continue full queue automation: start from `clang_tidy_all_*`.
- Unsure between daily and strict:
  - choose `daily` for routine iteration
  - choose `strict` for hard gate / final cleanup

## Source-of-Truth Rule

- Style rules are owned by clang-tidy configs (`.clang-tidy` / `.clang-tidy.strict`), not duplicated in these workflow docs.
- Workflow docs define execution policy and command contracts.
