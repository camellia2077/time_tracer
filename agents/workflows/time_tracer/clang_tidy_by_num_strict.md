---
description: Agent policy for one numbered time_tracer clang-tidy task (strict profile)
---

## Shared Base (MUST)
- Read and obey: `.agent/workflows/time_tracer/clang_tidy_by_num_shared.md`
- This file only defines strict-profile bindings for shared placeholders.

## Fixed Contract (MUST)
- Official config profile: strict

## Config Policy (MUST)
- Always use `--strict-config`.
- Keep `--strict-config` on every `tidy-step` / `tidy-batch` command in this run.
- Do not silently downgrade to the daily profile mid-run.

## Profile Bindings (MUST)
- `<PROFILE_STEP_FLAGS>` = `--strict-config`
- `<PROFILE_BATCH_FLAGS>` = `--strict-config`

## Expanded Commands (MUST)
- Step 4 command:
  - `tidy-step --task-log <resolved_task_json> --dry-run --strict-config`
- Step 5 command:
  - `tidy-step --task-log <resolved_task_json> --strict-config`
- Batch close command:
  - `tidy-batch --preset sop --strict-config`
