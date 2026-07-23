# Developer Scripts Local Contract

## Scope

Applies to `tools/scripts/**`. This subtree contains developer helpers; it does
not own repository build, verify, tidy, or platform-config orchestration.

## Routing And Invariants

- Helper implementations and docs belong under `tools/scripts/devtools/**`.
- Requests for build, verify, validate, tidy, or platform-config behavior belong
  under `tools/toolchain/**` instead.
- Do not add a new root-level script entry unless the user explicitly requests
  one and the toolchain entry model cannot represent the workflow.
- Keep secrets, signing inputs, and machine-specific paths out of checked-in
  helper defaults and examples.

## Validation

Use the smallest safe check for the changed helper: a focused unit test when one
exists, otherwise its non-destructive `--help`, dry-run, parser, or fixture path.
If the helper mutates external or machine-local state, do not execute that path
without the authorization required by the repository contract.

## Local Completion Bar

- The helper remains under `devtools/**` and does not duplicate a toolchain
  command.
- Changed argument/error behavior is covered by a focused test or safe smoke.
- The adjacent helper README reflects changed usage, prerequisites, and side
  effects.
